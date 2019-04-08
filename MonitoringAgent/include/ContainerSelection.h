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

/*
 * 名称：监控容器选择模块
 * 功能：从待监控容器队列中选择待监控的容器，
 *      并将待监控的容器id发送给监控数据收集模块；
 *      同时该模块接收输出模块发送的监控周期调整信息
 */


// 待监控容器队列元素
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


class ContainerSelection{
private:                               
    // 默认容器监控周期
    int COLLECT_CYCLE = 3;         

    static std::vector<ContainerItem> ContainerList; 
    static std::mutex ContainerList_lock;  
public:
    ContainerSelection();
    ~ContainerSelection();
    
    // 容器信息收集模块接口：添加或删除监控代理中的容器
    void adjustContainerList(std::string ContainerID, std::string status);      // 根据容器信息搜集模块传入的数据调整待监控容器队列; type为1表示start,type为0表示stop
    // 传输模块接口：监控服务调整监控周期的接口
    void adjustContainerCycle(std::string ContainerID, int cycle);              // 根据监控服务器发送的信息调整容器的监控周期
    // 选择待监控容器
    void selectContainer();               
    // 创建线程运行监控容器选择程序
    void runContainerSelection();

    void showList();
};

#endif

