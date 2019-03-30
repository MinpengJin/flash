#ifndef CONTAINERINFOCOLLECTION_H
#define CONTAINERINFOCOLLECTION_H

#include <Python.h>
#include <sstream>
#include <algorithm>
#include "json/json.h"
#include "ContainerSelection.h"

/*
 * 名称：容器信息收集模块
 * 功能：收集监控代理中启动和关闭的容器信息，
 *      并将信息发送给监控容器选择模块
 */


// 容器信息数据格式，id和状态
struct InfoType{
    std::string id, status;
};

class ContainerInfoCollection{
private:
    int foundCycle = 10;            // 容器信息发现周期
public:
    ContainerInfoCollection();
    ~ContainerInfoCollection();

    void setFoundCycle(int cycle);
    int getFoundCycle();

    // 收集启动和关闭的容器信息
    void collectionContainerInfo();
    // 创建线程运行容器信息收集程序
    void runContainerInfoCollection();
};

#endif