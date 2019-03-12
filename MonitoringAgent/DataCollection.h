#include<iostream>
#include<fstream>
#include<ctime>
#include<string>
#include<map>
#include<sstream>
#include<vector>
#include<stdlib.h>
using namespace std;

const int CORE_NUM = 1;                                         // 宿主机CPU核数

struct preContainerData                                         // 前一个时间的容器监控数据
{
    time_t preTime;                                             // 上一次容器状态更新时间戳
    unsigned long long containerTimeSlice, totalTimeSlice;      // 容器使用时间片和机器总的时间片
    unsigned long long diskRead, diskWrite;                     // 磁盘读速率和写速率
    unsigned long long netReceive, netTransmit;                 // 网络接收和发送字节数
};


class DataCollection{
private:
    fstream fs;
    string ContainerID;
    map<string, preContainerData> ContainerDataList;            // 前一个时间的容器监控数据列表

    void openFile(string fileName);                             // 打开指定文件
    void closeFile();                                           // 关闭当前打开的文件

    vector<string> split(const char* buffer);                   // 以空格为分隔符来分割字符串
    string getConPID();                                         // 获取指定容器id的容器进程号

    unsigned long long readSimpleData(string fileName);         // 读取不需要进行分割处理的文件数据流
    unsigned long long readConTimeSlice();                      // 读取容器使用CPU时间片
    unsigned long long readTotalTimeSlice();                    // 读取机器总的CPU时间片
    unsigned long long readMemUsed();                           // 读取容器已经使用的内存
    unsigned long long readMemLimit();                          // 读取机器分配给容器的内存
    unsigned long long *readDiskData();                         // 读取磁盘的读写数据,第一个值为读字节数，第二个值为写字节数
    unsigned long long *readNetData();                          // 读取网络接收和发送字节数，第一个值为接收字节数，第二个值为发送字节数

    float cpuUsage();                                           // 计算CPU使用率
    float memUsage();                                           // 计算内存使用率
    float* diskRate();                                          // 计算磁盘读速率和写速率，第一个值为读速率，第二个值为写速率
    float* netRate();                                           // 计算网络接收速率和发送速率，第一个值为接受速率，第二个值为发送速率

public:
    DataCollection(string ContainerID);                         // 构造函数
    ~DataCollection();                                          // 析构函数，关闭当前打开的文件

    // interface for the ContainerSelection module
    void updateContainerStatus();                               // 添加或者更新指定的容器状态到容器状态列表中
    void eraseContainerStatus();                                // 在容器状态列表中删除指定容器状态

    void processData();                                         // 将指定容器的监控数据转换成json格式
};


DataCollection::DataCollection(string ContainerID){
    this->ContainerID = ContainerID;
}


DataCollection::~DataCollection(){
    if(fs.is_open()){
        fs.close();
    }
}


void DataCollection::openFile(string fileName){
    if(!fs.is_open()){
        fs.open(fileName);
        if(!fs.is_open()){
            cerr << "Errore opening file! The file is " + fileName << endl;
            exit(1);
        }
    }else{
        cout << "There is already having an open file!" << endl;
    }
}


void DataCollection::closeFile(){
    if(fs.is_open()){
        fs.close();
    }
}


vector<string> DataCollection::split(const char *buffer){
    stringstream ss;
    ss.str(buffer);
    string item;
    vector<string> elems;
    while(ss >> item){
        elems.push_back(item);
    }
    return elems;
}

unsigned long long DataCollection::readSimpleData(string fileName){
    openFile(fileName);
    unsigned long long data;
    char buffer[256];
    fs.getline(buffer, 200);
    data = atoll(buffer);
    closeFile();
    return data;
}


unsigned long long DataCollection::readConTimeSlice(){
    string fileName = "/sys/fs/cgroup/cpuacct/docker/" + ContainerID + "/cpuacct.usage";
    return readSimpleData(fileName);
}


unsigned long long DataCollection::readTotalTimeSlice(){
    string fileName = "/proc/stat";
    openFile(fileName);
    unsigned long long totalTimeSlice;
    totalTimeSlice = 0;
    char buffer[256];
    fs.getline(buffer, 200);
    vector<string> elems = split(buffer);
    for(int i = 1; i < 10; i++){
        totalTimeSlice += atoll(elems[i].c_str());
    }
    closeFile();
    return totalTimeSlice;
}


unsigned long long DataCollection::readMemUsed(){
    string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.usage_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long DataCollection::readMemLimit(){
    string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.limit_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long *DataCollection::readDiskData(){
    string fileName = "/sys/fs/cgroup/blkio/docker/" + ContainerID + "/blkio.throttle.io_service_bytes";
    openFile(fileName);
    unsigned long long *diskData = new unsigned long long[2];
    char buffer[256];
    // 读取文件第一行，磁盘读速率数据
    fs.getline(buffer, 200);
    vector<string> elems1 = split(buffer);
    diskData[0] = atoll(elems1[2].c_str());
    // 读取文件第二行，磁盘写速率数据
    fs.getline(buffer, 200);
    vector<string> elems2 = split(buffer);
    diskData[1] = atoll(elems2[2].c_str());
    closeFile();
    return diskData;
}


// todo: 获取容器进程的pid
string DataCollection::getConPID(){

}


unsigned long long *DataCollection::readNetData(){
    string pid = getConPID();
    string fileName = "/proc/" + pid + "/net/dev";
    openFile(fileName);
    unsigned long long *netData = new unsigned long long[2];
    char buffer[256];
    vector<string> elems;
    while(fs.getline(buffer, 200)){
        vector<string> temp = split(buffer);
        // todo: 需要确定容器对应网卡，暂时选择物理网卡1
        if(temp[0] == "eth0:"){
            elems = temp;
        }
    }
    netData[0] = atoll(elems[1].c_str());
    netData[1] = atoll(elems[9].c_str());
    closeFile();
    return netData;
}


float DataCollection::cpuUsage(){
    float cpuUsage;
    unsigned long long preConTimeSlice = ContainerDataList[ContainerID].containerTimeSlice;
    unsigned long long preTotalTimeSlice = ContainerDataList[ContainerID].totalTimeSlice;
    unsigned long long nowConTimeSlice = readConTimeSlice();
    unsigned long long nowTotalTimeSlice = readTotalTimeSlice();
    cpuUsage = ((nowConTimeSlice - preConTimeSlice)/(nowTotalTimeSlice - preTotalTimeSlice))*CORE_NUM*100;
    return cpuUsage;
}


float DataCollection::memUsage(){
    float memUsage;
    unsigned long long memUsed = readMemUsed();
    unsigned long long memLimit = readMemLimit();
    memUsage = (memUsage / memLimit) * 100;
    return memUsage;
}


float *DataCollection::diskRate(){
    float *diskRate = new float[2];
    unsigned long long *diskData = readDiskData();
    time_t nowTime = time(NULL);
    time_t preTime = ContainerDataList[ContainerID].preTime;
    // 计算磁盘读速率
    unsigned long long nowReadBytes = diskData[0];
    unsigned long long preReadBytes = ContainerDataList[ContainerID].diskRead;
    diskRate[0] = ((nowReadBytes - preReadBytes)/(nowTime - preTime))*100;
    // 计算磁盘写速率
    unsigned long long nowWriteBytes = diskData[1];
    unsigned long long preWriteBytes = ContainerDataList[ContainerID].diskWrite;
    diskRate[1] = ((nowWriteBytes - preWriteBytes)/(nowTime - preTime))*100;
    delete[] diskData;
    return diskRate;
}


float *DataCollection::netRate(){
    float *netRate = new float[2];
    unsigned long long *netData = readNetData();
    time_t nowTime = time(NULL);
    time_t preTime = ContainerDataList[ContainerID].preTime;
    // 计算网络接受速率
    unsigned long long nowReceiveBytes = netData[0];
    unsigned long long preReceiveBytes = ContainerDataList[ContainerID].netReceive;
    netRate[0] = ((nowReceiveBytes - preReceiveBytes)/(nowTime - preTime))*100;
    // 计算网络输出速率
    unsigned long long nowTransmitBytes = netData[1];
    unsigned long long preTransmitBytes = ContainerDataList[ContainerID].netTransmit;
    netRate[1] = ((nowTransmitBytes - preTransmitBytes)/(nowTime - preTime))*100;
    delete[] netData;
    return netRate;
}


void DataCollection::updateContainerStatus(){
    preContainerData tempStatus;
    tempStatus.preTime = time(NULL);
    tempStatus.containerTimeSlice = readConTimeSlice();
    tempStatus.totalTimeSlice = readTotalTimeSlice();
    unsigned long long *diskData = readDiskData();
    tempStatus.diskRead = diskData[0];
    tempStatus.diskWrite = diskData[1];
    delete[] diskData;
    unsigned long long *netData = readNetData();
    tempStatus.netReceive = netData[0];
    tempStatus.netTransmit = netData[1];
    delete[] netData;
    ContainerDataList[ContainerID] = tempStatus;
}


void DataCollection::eraseContainerStatus(){
    auto it = ContainerDataList.find(ContainerID);
    ContainerDataList.erase(it);
}


void DataCollection::processData(){

}