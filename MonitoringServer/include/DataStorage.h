#ifndef DATASTORAGE_H
#define DATASTORAGE_H

#include <ctime>
#include <string>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <thread>
#include "influxdb.h"
#include "json/json.h"

struct StorageFormat{
    std::string agentID;
    std::string ContainerID;
    time_t Timestamp;
    float CpuLoadAvg;
    float MemLoadAvg;
    float DiskReadAvg;
    float DiskWriteAvg;
    float NetReceiveAvg;
    float NetTransmitAvg;
};


class DataStorage {
public:
    DataStorage();
    ~DataStorage();

    void createControllerTab();
    void storeData(StorageFormat data);
    void scanControllerTab();
    void runDataController();
};

extern bool ControllerTabExist;
extern influxdb_cpp::server_info client;
extern std::unique_ptr<DataStorage> dataStorage;

#endif