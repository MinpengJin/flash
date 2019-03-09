#include<vector>
#include<algorithm>
#include<unistd.h>
#include<windows.h>
#include"DataCollection.h"
using namespace std;

const int COLLECT_CYCLE = 3;                                    // 默认容器监控周期

struct ContainerItem
{
    string ContainerID;
    int collectCycle;                                           // 容器监控周期
    time_t nextTime;                                            // 下一次容器监控时间
    // 根据容器的下一次监控时间对待监控队列进行排序
    bool operator< (const ContainerItem &b){
        return this->nextTime < b.nextTime;
    }
};

class ContainerSelection{

private:
    vector<ContainerItem> ContainerList;                        // 待监控容器队列

public:
    ContainerSelection();
    ~ContainerSelection();
    
    void adjustContainerList(string ContainerID, int type);     // 根据容器信息搜集模块传入的数据调整待监控容器队列; 
                                                                // type为1表示start,type为0表示stop
    void adjustContainerCycle(string ContainerID, int cycle);   // 根据监控服务器发送的信息调整容器的监控周期
    void runContainerSelection();                               // 运行监控容器选择模块
};


ContainerSelection::ContainerSelection(){}
ContainerSelection::~ContainerSelection(){}


void ContainerSelection::adjustContainerList(string ContainerID, int type){
    DataCollection temp(ContainerID);
    if(type){
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
                DataCollection temp(ContainerID);
                temp.processData();
                it->nextTime = nowTime + it->collectCycle;
                sort(ContainerList.begin(), ContainerList.end());
            }
        }else{
            cout << "there are currently no running containers!" << endl;
        }
    }
}