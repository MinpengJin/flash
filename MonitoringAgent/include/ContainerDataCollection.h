#ifndef DATACOLLECTION_H
#define DATACOLLECTION_H

#include "Python.h"
#include <fstream>
#include <ctime>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <mutex>
#include "json/json.h"
#include "CallPython.h"
#include "ClientTransmission.h"

/*
 * 名称：监控数据收集模块
 * 功能：根据监控容器选择模块发送的容器id收集其容器信息
 *      并将收集的容器数据信息转换成json格式发送给传输模块
 */


// 前一个监控周期的容器监控数据
struct preContainerData                                                
{
    time_t preTime;                                                     // 上一次容器状态更新时间戳
    unsigned long long containerTimeSlice, totalTimeSlice;              // 容器使用时间片和机器总的时间片
    unsigned long long diskRead, diskWrite;                             // 磁盘读速率和写速率
    unsigned long long netReceive, netTransmit;                         // 网络接收和发送字节数
};

// 监控数据格式
struct ProcessedData{
    std::string ContainerID;
    time_t Timestamp;
    float CpuLoadAvg;
    float MemLoadAvg;
    float DiskReadAvg;
    float DiskWriteAvg;
    float NetReceiveAvg;
    float NetTransmitAvg;
};


class ContainerDataCollection{
private:
    bool hasError = false;
    // 记录容器上一个监控周期的监控数据    
    static std::map<std::string, preContainerData> ContainerDataList;
    static std::mutex ContainerDataList_lock;

    int core_num = 1;                                                    // 宿主机CPU核数

    std::vector<std::string> split(const char* buffer);                 // 以空格为分隔符来分割字符串

    unsigned long long readSimpleData(std::string fileName);            // 读取不需要进行分割处理的文件数据流
    unsigned long long readConTimeSlice(std::string ContainerID);       // 读取容器使用CPU时间片
    unsigned long long readTotalTimeSlice();                            // 读取机器总的CPU时间片
    unsigned long long readMemUsed(std::string ContainerID);            // 读取容器已经使用的内存
    unsigned long long readMemLimit(std::string ContainerID);           // 读取机器分配给容器的内存
    unsigned long long *readDiskData(std::string ContainerID);          // 读取磁盘的读写数据,第一个值为读字节数，第二个值为写字节数
    unsigned long long *readNetData(std::string ContainerID);           // 读取网络接收和发送字节数，第一个值为接收字节数，第二个值为发送字节数

    float getCpuLoadAvg(std::string ContainerID);                       // 计算CPU使用率
    float getMemLoadAvg(std::string ContainerID);                       // 计算内存使用率
    float* getDiskRateAvg(std::string ContainerID);                     // 计算磁盘读速率和写速率，第一个值为读速率，第二个值为写速率
    float* getNetRateAvg(std::string ContainerID);                      // 计算网络接收速率和发送速率，第一个值为接受速率，第二个值为发送速率

public:
    ContainerDataCollection();                                          // 构造函数
    ~ContainerDataCollection();                                         // 析构函数，关闭当前打开的文件

    void setCoreNum(int num);
    int getCoreNum();

    // 监控容器选择模块接口
    void initContainerStatus(std::string ContainerID);                  // 添加或者更新指定的容器状态到容器状态列表中
    void eraseContainerStatus(std::string ContainerID);                 // 在容器状态列表中删除指定容器状态
    // 处理监控数据，将其转换成Json格式
    void processData(std::string ContainerID);                          // 将指定容器的监控数据转换成json格式
};

#endif