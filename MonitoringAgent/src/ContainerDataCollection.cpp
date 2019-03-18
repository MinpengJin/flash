#include<iostream>
#include<sstream>
#include<json/json.h>
#include<Python.h>
#include"ContainerDataCollection.h"
#include"json/json.h"
using namespace std;

ContainerDataCollection::ContainerDataCollection(string ContainerID){
    this->ContainerID = ContainerID;
}


ContainerDataCollection::~ContainerDataCollection(){
    if(fs.is_open()){
        fs.close();
    }
}


void ContainerDataCollection::openFile(string fileName){
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


void ContainerDataCollection::closeFile(){
    if(fs.is_open()){
        fs.close();
    }
}


vector<string> ContainerDataCollection::split(const char *buffer){
    stringstream ss;
    ss.str(buffer);
    string item;
    vector<string> elems;
    while(ss >> item){
        elems.push_back(item);
    }
    return elems;
}

unsigned long long ContainerDataCollection::readSimpleData(string fileName){
    openFile(fileName);
    unsigned long long data;
    char buffer[256];
    fs.getline(buffer, 200);
    data = atoll(buffer);
    closeFile();
    return data;
}


unsigned long long ContainerDataCollection::readConTimeSlice(){
    string fileName = "/sys/fs/cgroup/cpuacct/docker/" + ContainerID + "/cpuacct.usage";
    return readSimpleData(fileName);
}


unsigned long long ContainerDataCollection::readTotalTimeSlice(){
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


unsigned long long ContainerDataCollection::readMemUsed(){
    string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.usage_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long ContainerDataCollection::readMemLimit(){
    string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.limit_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long *ContainerDataCollection::readDiskData(){
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


string ContainerDataCollection::getConPID(){
    string pid;
    PyObject *pModule,*pFunc;
    PyObject *pArgs, *pValue;
    // 对python初始化 
    Py_Initialize(); 
    if(!Py_IsInitialized()){
        cout << "init faild/n" << endl;
        exit(1);
    }else{
        // 设置python文件路径 
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('../src/python')");
        // 加载模块 
        pModule = PyImport_ImportModule("containerPID");
        if(!pModule){
            cout << "module failed!" << endl;
        }else
        {
            // 加载函数 
            pFunc = PyObject_GetAttrString(pModule, "getContainerPID");
            if(!pFunc || !PyCallable_Check(pFunc)){
                cout << "can not find function!" << endl;
            }else{
                // 设置参数 
                pArgs = PyTuple_New(1);
                const char *id = this->ContainerID.c_str();
                PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", id));
                // 调用函数 
                pValue = PyEval_CallObject(pFunc, pArgs);
                int temp;
                PyArg_Parse(pValue, "i", &temp);
                stringstream ss;
                ss << temp;
                pid = temp.str();
            }
        }
        Py_Finalize(); 
    }
    
    return pid;
}


unsigned long long *ContainerDataCollection::readNetData(){
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


float ContainerDataCollection::getCpuLoadAvg(){
    float cpuUsage;
    unsigned long long preConTimeSlice = ContainerDataList[ContainerID].containerTimeSlice;
    unsigned long long preTotalTimeSlice = ContainerDataList[ContainerID].totalTimeSlice;
    unsigned long long nowConTimeSlice = readConTimeSlice();
    unsigned long long nowTotalTimeSlice = readTotalTimeSlice();
    cpuUsage = ((nowConTimeSlice - preConTimeSlice)/(nowTotalTimeSlice - preTotalTimeSlice))*CORE_NUM*100;
    ContainerDataList[ContainerID].containerTimeSlice = nowConTimeSlice;
    ContainerDataList[ContainerID].totalTimeSlice = nowTotalTimeSlice;
    return cpuUsage;
}


float ContainerDataCollection::getMemLoadAvg(){
    float memUsage;
    unsigned long long memUsed = readMemUsed();
    unsigned long long memLimit = readMemLimit();
    memUsage = (memUsage / memLimit) * 100;
    return memUsage;
}


float *ContainerDataCollection::getDiskRateAvg(){
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
    ContainerDataList[ContainerID].diskRead = nowReadBytes;
    ContainerDataList[ContainerID].diskWrite = nowWriteBytes;
    return diskRate;
}


float *ContainerDataCollection::getNetRateAvg(){
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
    ContainerDataList[ContainerID].preTime = nowTime;
    ContainerDataList[ContainerID].netReceive = nowReceiveBytes;
    ContainerDataList[ContainerID].netTransmit = nowTransmitBytes;
    return netRate;
}


void ContainerDataCollection::updateContainerStatus(){
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


void ContainerDataCollection::eraseContainerStatus(){
    auto it = ContainerDataList.find(ContainerID);
    ContainerDataList.erase(it);
}


void ContainerDataCollection::processData(){
    string processedData;
    Json::Value root;
    Json::StreamWriterBuilder writerBuilder;
    ostringstream os;
    root["ContainerID"] = this->ContainerID;
    root["Timestamp"] = time(NULL);
    root["CpuLoadAvg"] = getCpuLoadAvg();
    root["MemLoadAvg"] = getMemLoadAvg();
    float *diskRate = getDiskRateAvg();
    root["DiskReadAvg"] = diskRate[0];
    root["DiskWriteAvg"] = diskRate[1];
    delete[] diskRate;
    float *netRate = getNetRateAvg();
    root["NetReceiveAvg"] = netRate[0];
    root["NetTransmitAvg"] = netRate[1];
    delete[] netRate;
    unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    processedData = os.str();
    // todo:调用传输模块接口，将json数据传输给监控服务器
    
}