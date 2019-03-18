#include<iostream>
#include"ContainerSelection.h"

using namespace std;

ContainerSelection::ContainerSelection(){}
ContainerSelection::~ContainerSelection(){}


void ContainerSelection::adjustContainerList(string ContainerID, string status){
    ContainerDataCollection temp(ContainerID);
    if(status == "start"){
        time_t nowTime = time(NULL);                            // 当前时间的时间戳，单位为秒
        ContainerItem tempItem;
        tempItem.ContainerID = ContainerID;
        tempItem.collectCycle = COLLECT_CYCLE;
        tempItem.nextTime = COLLECT_CYCLE + nowTime;
        ContainerList.push_back(tempItem);
        sort(ContainerList.begin(), ContainerList.end());
        temp.updateContainerStatus();                           // 将新建容器的监控数据保存到容器数据列表中
    }else{
        auto it = ContainerList.begin();
        while(it->ContainerID != ContainerID){
            it++;
        }
        ContainerList.erase(it);
        temp.eraseContainerStatus();                            // 在容器数据列表中删除相应容器监控数据
    }
}


void ContainerSelection::adjustContainerCycle(string ContainerID, int cycle){
    time_t nowTime = time(NULL);
    auto it = ContainerList.begin();
    while(it->ContainerID != ContainerID){
        it++;
    }
    it->collectCycle = cycle;
    it->nextTime = it->collectCycle + nowTime;
    sort(ContainerList.begin(), ContainerList.end());
}


void ContainerSelection::runContainerSelection(){
    while(true){
        if(!ContainerList.empty()){
            time_t nowTime = time(NULL);
            auto it = ContainerList.begin();
            if(it->nextTime > nowTime){
                int sleepTime = it->nextTime - nowTime;
                sleep(sleepTime);
            }else{
                string ContainerID = it->ContainerID;
                // 将ContainerID传入数据搜集模块
                ContainerDataCollection temp(ContainerID);
                temp.processData();
                it->nextTime = nowTime + it->collectCycle;
                sort(ContainerList.begin(), ContainerList.end());
            }
        }else{
            cout << "there are currently no running containers!" << endl;
        }
    }
}