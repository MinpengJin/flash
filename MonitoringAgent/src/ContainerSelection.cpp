#include"ContainerSelection.h"

ContainerSelection::ContainerSelection(){
    runThread = std::make(std::make_unique<std::thread>(&ContainerSelection::runContainerSelection, this));
    std::unique_ptr<ContainerDataCollection> temp(new ContainerDataCollection(ContainerID));
    ContainerDataCollection_ptr.reset(temp.release());
}

ContainerSelection::~ContainerSelection(){
    runThread->join();
}


void ContainerSelection::adjustContainerList(std::string ContainerID, std::string status){   
    std::lock_guard<std::mutex> lock(ContainerList_lock);
    if(status == "start"){
        time_t nowTime = time(NULL);                            // 当前时间的时间戳，单位为秒
        ContainerItem tempItem;
        tempItem.ContainerID = ContainerID;
        tempItem.collectCycle = COLLECT_CYCLE;
        tempItem.nextTime = COLLECT_CYCLE + nowTime;
        ContainerList.push_back(tempItem);
        sort(ContainerList.begin(), ContainerList.end());
        ContainerDataCollection_ptr->updateContainerStatus(ContainerID);           // 将新建容器的监控数据保存到容器数据列表中
    }else{
        auto it = ContainerList.begin();
        while(it->ContainerID != ContainerID){
            it++;
        }
        ContainerList.erase(it);
        ContainerDataCollection_ptr->eraseContainerStatus(ContainerID);           // 在容器数据列表中删除相应容器监控数据
    }
}


void ContainerSelection::adjustContainerCycle(std::string ContainerID, int cycle){
    time_t nowTime = time(NULL);
    auto it = ContainerList.begin();
    while(it->ContainerID != ContainerID){
        it++;
    }
    it->collectCycle = cycle;
    it->nextTime = it->collectCycle + nowTime;

    std::lock_guard<std::mutex> lock(ContainerList_lock);
    sort(ContainerList.begin(), ContainerList.end());
}


void ContainerSelection::runContainerSelection(){
    while(true){
        ContainerList_lock.lock();
        if(!ContainerList.empty()){
            time_t nowTime = time(NULL);
            auto it = ContainerList.begin();
            if(it->nextTime > nowTime){
                int sleepTime = it->nextTime - nowTime;
                ContainerList_lock.unlock();
                sleep(sleepTime);
            }else{
                std::string ContainerID = it->ContainerID;
                it->nextTime = nowTime + it->collectCycle;
                sort(ContainerList.begin(), ContainerList.end());
                ContainerList_lock.unlock();
                // 将ContainerID传入数据搜集模块
                std::thread t(&ContainerDataCollection::peocessData, *ContainerDataCollection_ptr, ContainerID);
                t.detach();
            }
        }else{
            ContainerList_lock.unlock();
            std::cout << "there are currently no running containers!" << std::endl;
        }
    }
}