#include "ServerTransmission.h"

int main(){
    // 创建监控服务器传输模块实例
    ServerTransmission s_transmission;
    // 运行数据库存储控制功能
    DataStorage dataStorage;
    dataStorage.runDataController();

    while(true){}
    return 0;
}