#ifndef CONSELECTION_H
#define CONSELECTION_H

#include <vector>
#include <algorithm>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <thread>
#include <mutex>
#include "ContainerDataCollection.h"


const int COLLECT_CYCLE = 3;                                                    // 默认容器监控周期

struct ContainerItem
{
    std::string ContainerID;
    int collectCycle;                                                           // 容器监控周期
    time_t nextTime;                                                            // 下一次容器监控时间
    // 根据容器的下一次监控时间对待监控队列进行排序
    bool operator< (const ContainerItem &b){
        return this->nextTime < b.nextTime;
    }
};


std::mutex ContainerList_lock;

class ContainerSelection{
private:
    std::vector<ContainerItem> ContainerList;                                   // 待监控容器队列
    std::unique_ptr<ContainerDataCollection> ContainerDataCollection_ptr;                                
    std::unique_ptr<std::thread> runThread;

public:
    ContainerSelection();
    ~ContainerSelection();
    
    // interface for ContainerInfoCollection module
    void adjustContainerList(std::string ContainerID, std::string status);      // 根据容器信息搜集模块传入的数据调整待监控容器队列; type为1表示start,type为0表示stop
    // interface for Transmission module
    void adjustContainerCycle(std::string ContainerID, int cycle);              // 根据监控服务器发送的信息调整容器的监控周期
    
    void runContainerSelection();                                               // 运行监控容器选择模块
};

#endif

