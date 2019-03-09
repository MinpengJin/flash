#include<iostream>
#include<fstream>
#include<ctime>
#include<string>
#include<map>
#include<stdlib.h>
using namespace std;

struct preContainerData                                         // 前一个时间的容器监控数据
{
    time_t preTime;                                             // 上一次容器状态更新时间戳
    int containerTimeSlice, totalTimeSlice;                     // 容器使用时间片和机器总的时间片
    int diskRead, diskWrite;                                    // 磁盘读速率和写速率
    int netReceive, netSend;                                    // 网络接收字节数和发送字节数
};


class DataCollection{
private:
    fstream fs;
    string ContainerID;
    map<string, preContainerData> ContainerDataList;            // 前一个时间的容器监控数据列表

public:
    DataCollection(string ContainerID);
    ~DataCollection();

    void openFile(string fileName);
    void closeFile();

    void updateContainerStatus();                               // 添加或者更新指定的容器状态到容器状态列表中
    void eraseContainerStatus();                                // 在容器状态列表中删除指定容器状态

    unsigned int readConTimeSlice();                            // 读取容器使用CPU时间片
    unsigned long long readTotalTimeSlice();                    // 读取机器总的CPU时间片
    unsigned int readMemUsed();                                 // 读取容器已经使用的内存
    unsigned int readMemLimit();                                // 读取机器分配给容器的内存
    unsigned int readDiskRead();                                // 读取磁盘读速率
    unsigned int readDiskWrite();                               // 读取磁盘写速率
    unsigned int readNetReceive();                              // 读取网络接收字节数
    unsigned int readNetSend();                                 // 读取网络发送字节数

    float cpuUsage();                                           // 计算CPU使用率
    float memUsage();                                           // 计算内存使用率
    float diskReadRate();                                       // 计算磁盘读速率
    float diskWriteRate();                                      // 计算磁盘写速率
    float netReceivingRate();                                   // 计算网络接收速率
    float netSendingRate();                                     // 计算网络发送速率

    void processData();                                         // 处理指定容器的监控数据
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
    }else{
        cout << "There is already having an open file!" << endl;
    }
}


void DataCollection::closeFile(){
    if(fs.is_open()){
        fs.close();
    }
}


void DataCollection::updateContainerStatus(){
    preContainerData tempStatus;
    tempStatus.preTime = time(NULL);
    tempStatus.containerTimeSlice = readConTimeSlice();
    tempStatus.totalTimeSlice = readTotalTimeSlice();
    tempStatus.diskRead = readDiskRead();
    tempStatus.diskWrite = readDiskWrite();
    tempStatus.netReceive = readNetReceive();
    tempStatus.netSend = readNetSend();
    ContainerDataList[ContainerID] = tempStatus;
}


void DataCollection::eraseContainerStatus(){
    auto it = ContainerDataList.find(ContainerID);
    ContainerDataList.erase(it);
}


unsigned int DataCollection::readConTimeSlice(){
    string fileName = "/sys/fs/cgroup/cpuacct/docker/" + ContainerID + "/cpuacct.usage";
    openFile(fileName);
    int timeSlice = -1;
    if(!fs.is_open()){
        cout << "Errore opening file!" << endl;
    }else{
        char buffer[256];
        fs.getline(buffer, 100);
        timeSlice = atoll(buffer);
    }
    closeFile();
    return timeSlice;
}


unsigned long long DataCollection::readTotalTimeSlice(){

}


unsigned int DataCollection::readMemUsed(){

}


unsigned int DataCollection::readMemLimit(){

}


unsigned int DataCollection::readDiskRead(){

}


unsigned int DataCollection::readDiskWrite(){

}


unsigned int DataCollection::readNetReceive(){

}


unsigned int DataCollection::readNetSend(){

}


float DataCollection::cpuUsage(){

}


float DataCollection::memUsage(){

}


float DataCollection::diskReadRate(){

}


float DataCollection::diskWriteRate(){

}


float DataCollection::netReceivingRate(){

}


float DataCollection::netSendingRate(){

}


void DataCollection::processData(){

}