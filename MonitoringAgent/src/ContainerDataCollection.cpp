#include"ContainerDataCollection.h"


ContainerDataCollection::ContainerDataCollection(){}


ContainerDataCollection::~ContainerDataCollection(){
    if(fs.is_open()){
        fs.close();
    }
}


void ContainerDataCollection::openFile(std::string fileName){
    if(!fs.is_open()){
        fs.open(fileName);
        if(!fs.is_open()){
            std::cerr << "Errore opening file! The file is " + fileName << std::endl;
            exit(1);
        }
    }else{
        std::cout << "There is already having an open file!" << std::endl;
    }
}


void ContainerDataCollection::closeFile(){
    if(fs.is_open()){
        fs.close();
    }
}


std::vector<string> ContainerDataCollection::split(const char *buffer){
    std::stringstream ss;
    ss.str(buffer);
    std::string item;
    std::vector<string> elems;
    while(ss >> item){
        elems.push_back(item);
    }
    return elems;
}

unsigned long long ContainerDataCollection::readSimpleData(std::string fileName){
    openFile(fileName);
    unsigned long long data;
    char buffer[256];
    fs.getline(buffer, 200);
    data = atoll(buffer);
    closeFile();
    return data;
}


unsigned long long ContainerDataCollection::readConTimeSlice(std::string ContainerID){
    std::string fileName = "/sys/fs/cgroup/cpuacct/docker/" + ContainerID + "/cpuacct.usage";
    return readSimpleData(fileName);
}


unsigned long long ContainerDataCollection::readTotalTimeSlice(){
    std::string fileName = "/proc/stat";
    openFile(fileName);
    unsigned long long totalTimeSlice;
    totalTimeSlice = 0;
    char buffer[256];
    fs.getline(buffer, 200);
    std::vector<std::string> elems = split(buffer);
    for(int i = 1; i < 10; i++){
        totalTimeSlice += atoll(elems[i].c_str());
    }
    closeFile();
    return totalTimeSlice;
}


unsigned long long ContainerDataCollection::readMemUsed(std::string ContainerID){
    std::string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.usage_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long ContainerDataCollection::readMemLimit(std::string ContainerID){
    std::string fileName = "/sys/fs/cgroup/memory/docker/" + ContainerID + "/memory.limit_in_bytes";
    return readSimpleData(fileName);
}


unsigned long long *ContainerDataCollection::readDiskData(std::string ContainerID){
    std::string fileName = "/sys/fs/cgroup/blkio/docker/" + ContainerID + "/blkio.throttle.io_service_bytes";
    openFile(fileName);
    unsigned long long *diskData = new unsigned long long[2];
    char buffer[256];
    // 读取文件第一行，磁盘读速率数据
    fs.getline(buffer, 200);
    std::vector<std::string> elems1 = split(buffer);
    diskData[0] = atoll(elems1[2].c_str());
    // 读取文件第二行，磁盘写速率数据
    fs.getline(buffer, 200);
    std::vector<std::string> elems2 = split(buffer);
    diskData[1] = atoll(elems2[2].c_str());
    closeFile();
    return diskData;
}


std::string ContainerDataCollection::getConPID(std::string ContainerID){
    std::string pid;
    PyObject *pModule,*pFunc;
    PyObject *pArgs, *pValue;
    // 对python初始化 
    Py_Initialize(); 
    if(!Py_IsInitialized()){
        std::cerr << "init faild/n" << std::endl;
        exit(1);
    }else{
        // 设置python文件路径 
        PyRun_SimpleString("import sys");
        PyRun_SimpleString("sys.path.append('../src/python')");
        // 加载模块 
        pModule = PyImport_ImportModule("containerPID");
        // 加载函数 
        pFunc = PyObject_GetAttrString(pModule, "getContainerPID");
        if(!pFunc || !PyCallable_Check(pFunc)){
            std::cerr << "can not find function!" << std::endl;
            exit(1);
        }else{
            // 设置参数 
            pArgs = PyTuple_New(1);
            const char *id = ContainerID.c_str();
            PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", id));
            // 调用函数 
            pValue = PyEval_CallObject(pFunc, pArgs);
            int temp;
            PyArg_Parse(pValue, "i", &temp);
            std::stringstream ss;
            ss << temp;
            pid = temp.str();
        }
        Py_Finalize(); 
    }
    
    return pid;
}


unsigned long long *ContainerDataCollection::readNetData(std::string ContainerID){
    std::string pid = getConPID(ContainerID);
    std::string fileName = "/proc/" + pid + "/net/dev";
    openFile(fileName);
    unsigned long long *netData = new unsigned long long[2];
    char buffer[256];
    std::vector<std::string> elems;
    while(fs.getline(buffer, 200)){
        std::vector<std::string> temp = split(buffer);
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


float ContainerDataCollection::getCpuLoadAvg(std::string ContainerID){
    float cpuUsage;
    unsigned long long preConTimeSlice = ContainerDataList[ContainerID].containerTimeSlice;
    unsigned long long preTotalTimeSlice = ContainerDataList[ContainerID].totalTimeSlice;
    unsigned long long nowConTimeSlice = readConTimeSlice(ContainerID);
    unsigned long long nowTotalTimeSlice = readTotalTimeSlice(ContainerID);
    cpuUsage = ((nowConTimeSlice - preConTimeSlice)/(nowTotalTimeSlice - preTotalTimeSlice))*getCoreNum()*100;
    ContainerDataList[ContainerID].containerTimeSlice = nowConTimeSlice;
    ContainerDataList[ContainerID].totalTimeSlice = nowTotalTimeSlice;
    return cpuUsage;
}


float ContainerDataCollection::getMemLoadAvg(std::string ContainerID){
    float memUsage;
    unsigned long long memUsed = readMemUsed(ContainerID);
    unsigned long long memLimit = readMemLimit(ContainerID);
    memUsage = (memUsage / memLimit) * 100;
    return memUsage;
}


float *ContainerDataCollection::getDiskRateAvg(std::string ContainerID){
    float *diskRate = new float[2];
    unsigned long long *diskData = readDiskData(ContainerID);
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


float *ContainerDataCollection::getNetRateAvg(std::string ContainerID){
    float *netRate = new float[2];
    unsigned long long *netData = readNetData(ContainerID);
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


void ContainerDataCollection::setCoreNum(int num){
    core_num = num;
}


int ContainerDataCollection::getCoreNum(){
    return core_num;
}


void ContainerDataCollection::updateContainerStatus(std::string ContainerID){
    preContainerData tempStatus;
    tempStatus.preTime = time(NULL);
    tempStatus.containerTimeSlice = readConTimeSlice(ContainerID);
    tempStatus.totalTimeSlice = readTotalTimeSlice(ContainerID);
    unsigned long long *diskData = readDiskData(ContainerID);
    tempStatus.diskRead = diskData[0];
    tempStatus.diskWrite = diskData[1];
    delete[] diskData;
    unsigned long long *netData = readNetData(ContainerID);
    tempStatus.netReceive = netData[0];
    tempStatus.netTransmit = netData[1];
    delete[] netData;

    std::lock_guard<std::mutex> lock(ContainerDataList_lock);
    ContainerDataList[ContainerID] = tempStatus;
}


void ContainerDataCollection::eraseContainerStatus(std::string ContainerID){
    std::lock_guard<std::mutex> lock(ContainerDataList_lock);
    auto it = ContainerDataList.find(ContainerID);
    ContainerDataList.erase(it);
}


void ContainerDataCollection::processData(std::string ContainerID){
    ContainerDataList_lock.lock();
    Json::Value root, item;
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    root["cmd"] = "ContainerData";
    
    item["agentID"] = c_transmission.getAgentID();
    item["ContainerID"] = ContainerID;
    item["Timestamp"] = time(NULL);
    item["CpuLoadAvg"] = getCpuLoadAvg(ContainerID);
    item["MemLoadAvg"] = getMemLoadAvg(ContainerID);
    float *diskRate = getDiskRateAvg(ContainerID);
    item["DiskReadAvg"] = diskRate[0];
    item["DiskWriteAvg"] = diskRate[1];
    delete[] diskRate;
    float *netRate = getNetRateAvg();
    item["NetReceiveAvg"] = netRate[0];
    item["NetTransmitAvg"] = netRate[1];
    delete[] netRate;
    ContainerDataList_lock.unlock();
    root["data"] = item;

    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string processedData = os.str();
    // 调用传输模块，将json数据传输给监控服务器
    c_transmission->send(processedData);
}