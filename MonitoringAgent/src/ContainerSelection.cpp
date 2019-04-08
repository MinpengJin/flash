#include"ContainerSelection.h"

std::vector<ContainerItem> ContainerSelection::ContainerList;
std::mutex ContainerSelection::ContainerList_lock;


ContainerSelection::ContainerSelection(){}
ContainerSelection::~ContainerSelection(){}


void ContainerSelection::adjustContainerList(std::string ContainerID, std::string status){   
    std::lock_guard<std::mutex> lock(ContainerList_lock);
    ContainerDataCollection temp; 
    if(status == "start"){
        time_t nowTime = time(NULL);                  // 当前时间的时间戳，单位为秒
        ContainerItem tempItem;
        tempItem.ContainerID = ContainerID;
        tempItem.collectCycle = COLLECT_CYCLE;
        tempItem.nextTime = COLLECT_CYCLE + nowTime;
        ContainerList.push_back(tempItem);
        sort(ContainerList.begin(), ContainerList.end());
        temp.initContainerStatus(ContainerID);       // 将新建容器的监控数据保存到容器数据列表中
    }else if(status == "stop"){
        auto it = ContainerList.begin();
        while(it->ContainerID != ContainerID){
            it++;
        }
        ContainerList.erase(it);
        temp.eraseContainerStatus(ContainerID);      // 在容器数据列表中删除相应容器监控数据
    }
}


void ContainerSelection::adjustContainerCycle(std::string ContainerID, int cycle){
    std::lock_guard<std::mutex> lock(ContainerList_lock);
    time_t nowTime = time(NULL);
    auto it = ContainerList.begin();
    while(it->ContainerID != ContainerID){
        it++;
    }
    it->collectCycle = cycle;
    it->nextTime = it->collectCycle + nowTime;
    sort(ContainerList.begin(), ContainerList.end());
}


void ContainerSelection::selectContainer(){
    while(true){
        if(!ContainerList.empty()){
            ContainerList_lock.lock();
            time_t nowTime = time(NULL);
            auto it = ContainerList.begin();
            if(it->nextTime > nowTime){
                ContainerList_lock.unlock();
                int sleepTime = it->nextTime - nowTime;
                sleep(sleepTime);
            }else{
                std::string ContainerID = it->ContainerID;
                it->nextTime = nowTime + it->collectCycle;
                sort(ContainerList.begin(), ContainerList.end());
                ContainerList_lock.unlock();
                // 将ContainerID传入数据搜集模块
                ContainerDataCollection temp; 
                std::thread t(&ContainerDataCollection::processData, &temp, ContainerID);
                t.detach();
            }
        }
    }
}


void ContainerSelection::runContainerSelection(){
    std::thread t(&ContainerSelection::selectContainer, this);
    t.detach();
}


void ContainerSelection::showList(){
    for(auto it = ContainerList.begin(); it != ContainerList.end(); it++){
        std::cout << "[ContainerID:" << it->ContainerID << ", "
                << "collectCycle:" << it->collectCycle << ", "
                << "nextTime:" << it->nextTime << "] ";
    }
}