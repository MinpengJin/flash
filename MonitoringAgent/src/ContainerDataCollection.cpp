#include"ContainerDataCollection.h"

std::map<std::string, preContainerData> ContainerDataCollection::ContainerDataList;       
std::mutex ContainerDataCollection::ContainerDataList_lock; 


ContainerDataCollection::ContainerDataCollection(){}
ContainerDataCollection::~ContainerDataCollection(){}


std::vector<std::string> ContainerDataCollection::split(const char *buffer){
    std::stringstream ss;
    ss.str(buffer);
    std::string item;
    std::vector<std::string> elems;
    while(ss >> item){
        elems.push_back(item);
    }
    return elems;
}

unsigned long long ContainerDataCollection::readSimpleData(std::string fileName){
    unsigned long long data;
    std::fstream fs;
    fs.open(fileName, std::ios::in);
    if(fs.is_open()){
        char buffer[256];
        fs.getline(buffer, 200);
        data = atoll(buffer);
        fs.close();
    }else{
        hasError = true;
        data = 0;
    }
    return data;
}


unsigned long long ContainerDataCollection::readConTimeSlice(std::string ContainerID){
    std::string fileName = "/sys/fs/cgroup/cpuacct/docker/" + ContainerID + "/cpuacct.usage";
    return readSimpleData(fileName);
}


unsigned long long ContainerDataCollection::readTotalTimeSlice(){
    std::string fileName = "/proc/stat";
    unsigned long long totalTimeSlice;
    std::fstream fs;
    fs.open(fileName, std::ios::in);
    if(fs.is_open()){
        totalTimeSlice = 0;
        char buffer[256];
        fs.getline(buffer, 200);
        std::vector<std::string> elems = split(buffer);
        for(int i = 1; i < 10; i++){
            totalTimeSlice += atoll(elems[i].c_str());
        }
        fs.close();
    }else{
        hasError = true;
        totalTimeSlice = 0;
    }
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
    unsigned long long *diskData = new unsigned long long[2];
    std::fstream fs;
    fs.open(fileName, std::ios::in);
    if(fs.is_open()){
        char buffer[256];
        fs.getline(buffer, 200);
        std::vector<std::string> elems1 = split(buffer);
        if(elems1[1] == "0"){
            // 处理当前容器没有读写磁盘的情况
            diskData[0] = 0;
            diskData[1] = 0;
        }else{
            // 读取文件第一行，磁盘读速率数据
            diskData[0] = atoll(elems1[2].c_str());
            // 读取文件第二行，磁盘写速率数据
            fs.getline(buffer, 200);
            std::vector<std::string> elems2 = split(buffer);
            diskData[1] = atoll(elems2[2].c_str());
        }
        fs.close();
    }else{
        hasError = true;
        diskData[0] = 0;
        diskData[1] = 0;
    }
    return diskData;
}


unsigned long long *ContainerDataCollection::readNetData(std::string ContainerID){
    CallPython call;
    std::string pid = call.getConPid(ContainerID);

    std::string fileName = "/proc/" + pid + "/net/dev";
    unsigned long long *netData = new unsigned long long[2];
    std::fstream fs;
    fs.open(fileName, std::ios::in);
    if(fs.is_open()){
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
        // closeFile();
        fs.close();
    }else{
        hasError = true;
        netData[0] = 0;
        netData[1] = 0;
    }
    return netData;
}


float ContainerDataCollection::getCpuLoadAvg(std::string ContainerID){
    float cpuUsage;
    if(!hasError){
        unsigned long long preConTimeSlice = ContainerDataList[ContainerID].containerTimeSlice;
        unsigned long long preTotalTimeSlice = ContainerDataList[ContainerID].totalTimeSlice;
        unsigned long long nowConTimeSlice = readConTimeSlice(ContainerID);
        unsigned long long nowTotalTimeSlice = readTotalTimeSlice();
        cpuUsage = ((nowConTimeSlice - preConTimeSlice)/(nowTotalTimeSlice - preTotalTimeSlice))*getCoreNum()*100;
        // 更新ContainerDataList
        ContainerDataList[ContainerID].containerTimeSlice = nowConTimeSlice;
        ContainerDataList[ContainerID].totalTimeSlice = nowTotalTimeSlice;
    }else{
        cpuUsage = 0;
    }
    return cpuUsage;
}


float ContainerDataCollection::getMemLoadAvg(std::string ContainerID){
    float memUsage;
    if(!hasError){
        unsigned long long memUsed = readMemUsed(ContainerID);
        unsigned long long memLimit = readMemLimit(ContainerID);
        memUsage = (memUsed / memLimit) * 100;
    }else{
        memUsage = 0;
    }
    return memUsage;
}


float *ContainerDataCollection::getDiskRateAvg(std::string ContainerID){
    float *diskRate = new float[2];
    if(!hasError){
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
        // 更新ContainerDataList
        ContainerDataList[ContainerID].diskRead = nowReadBytes;
        ContainerDataList[ContainerID].diskWrite = nowWriteBytes;
    }else{
        diskRate[0] = 0;
        diskRate[0] = 0;
    }
    return diskRate;
}


float *ContainerDataCollection::getNetRateAvg(std::string ContainerID){
    float *netRate = new float[2];
    if(!hasError){
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
        // 更新ContainerDataList
        ContainerDataList[ContainerID].preTime = nowTime;
        ContainerDataList[ContainerID].netReceive = nowReceiveBytes;
        ContainerDataList[ContainerID].netTransmit = nowTransmitBytes;
    }else{
        netRate[0] = 0;
        netRate[0] = 0;
    }
    return netRate;
}


void ContainerDataCollection::setCoreNum(int num){
    core_num = num;
}


int ContainerDataCollection::getCoreNum(){
    return core_num;
}


void ContainerDataCollection::initContainerStatus(std::string ContainerID){
    if(!hasError){
        preContainerData tempStatus;
        tempStatus.preTime = time(NULL);
        tempStatus.containerTimeSlice = readConTimeSlice(ContainerID);
        tempStatus.totalTimeSlice = readTotalTimeSlice();
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
    }else{
        std::cout << "error opening file!" << std::endl;
        exit(1);
    }
}


void ContainerDataCollection::eraseContainerStatus(std::string ContainerID){
    std::lock_guard<std::mutex> lock(ContainerDataList_lock);
    auto it = ContainerDataList.find(ContainerID);
    ContainerDataList.erase(it);
}


void ContainerDataCollection::processData(std::string ContainerID){
    Json::Value root, item;
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;
    if(!hasError){
        ContainerDataList_lock.lock();
        root["cmd"] = "ContainerData";
        // 打包数据
        item["agentID"] = c_transmission->getAgentID();
        item["ContainerID"] = ContainerID;
        int nowTime = time(NULL);
        item["Timestamp"] = nowTime;
        item["CpuLoadAvg"] = getCpuLoadAvg(ContainerID);
        item["MemLoadAvg"] = getMemLoadAvg(ContainerID);
        float *diskRate = getDiskRateAvg(ContainerID);
        item["DiskReadAvg"] = diskRate[0];
        item["DiskWriteAvg"] = diskRate[1];
        delete[] diskRate;
        float *netRate = getNetRateAvg(ContainerID);
        item["NetReceiveAvg"] = netRate[0];
        item["NetTransmitAvg"] = netRate[1];
        delete[] netRate;
        ContainerDataList_lock.unlock();
        root["data"] = item;
    }else{
        root["cmd"] = "Error";
        hasError = false;
    }

    std::unique_ptr<Json::StreamWriter> jsonWriter(writerBuilder.newStreamWriter());
    jsonWriter->write(root, &os);
    std::string processedData = os.str();

    // 调用传输模块，将json数据传输给监控服务器
    c_transmission->send(processedData);
}
