#include "DataStorage.h"

DataStorage::DataStorage(){
    this->createControllerTab();
}

DataStorage::~DataStorage(){}

void DataStorage::createControllerTab(){
    influxdb_cpp::server_info client("127.0.0.1", 8086, "Docker_monitor");
    time_t nowTime = time(NULL);
    std::string resp;
    int ret = influxdb_cpp::builder()
            .meas("ControllerTable")
            .tag("agentID", "x")
            .tag("ContainerID", "y")
            .field("lines", 0)
            .timestamp(nowTime)
            .post_http(client, &resp);
    if(ret){
        std::cout << resp << std::endl;
    }else{
        ControllerTabExist = true;
    }
}

void DataStorage::storeData(StorageFormat data){
    influxdb_cpp::server_info client("127.0.0.1", 8086, "Docker_monitor");
    if(!ControllerTabExist){
        createControllerTab();
    }
    // 查找存储控制表，判断当前agentID和ContainerID是否已经存在
    std::string resp;
    std::string agentID = data.agentID;
    std::string ContainerID = data.ContainerID;
    std::string queryStatement = "select * from ControllerTable where agentID='"
                 + agentID +"' and ContainerID='" + ContainerID + "'";
    influxdb_cpp::query(resp, queryStatement, client);
    time_t timestamp = time(NULL);
    // 解析查询结果
    bool res;
    JSONCPP_STRING errs;
    Json::Value root, results, item, series, values;
    Json::CharReaderBuilder readerBuilder;
    std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
    res = jsonReader->parse(resp.c_str(), resp.c_str()+resp.length(), &root, &errs);
    if(!res || !errs.empty()){
        std::cout << "parse err: " << errs << std::endl; 
        exit(1);
    }
    results = root["results"];
    item = results[0];
    series = item["series"];
    if(series.empty()){
        // 当前agentID和ContainerID不存在
        int ret = influxdb_cpp::builder()
            .meas("ControllerTable")
            .tag("agentID", agentID)
            .tag("ContainerID", ContainerID)
            .field("lines", 1)
            .timestamp(timestamp)
            .post_http(client, &resp);
        if(ret){
            std::cout << resp << std::endl;
        }
    }else{
        // 当前agentID和ContainerID存在，获取当前行数并加一
        values = series[0]["values"];
        int nowLines = values[0][2].asInt() + 1;
        int ret = influxdb_cpp::builder()
            .meas("ControllerTable")
            .tag("agentID", agentID)
            .tag("ContainerID", ContainerID)
            .field("lines", nowLines)
            .timestamp(timestamp)
            .post_http(client, &resp);
        if(ret){
            std::cout << resp << std::endl;
        }
        if(nowLines == 100){
            // 查询并解析agentID表中相应ContainerID的数据
            queryStatement = "select * from " + agentID + 
                        "where ContainerID='" + ContainerID + "'";
            influxdb_cpp::query(resp, queryStatement, client);
            res = jsonReader->parse(resp.c_str(), resp.c_str()+resp.length(), &root, &errs);
            if(!res || !errs.empty()){
                std::cout << "parse err: " << errs << std::endl; 
                exit(1);
            }
            results = root["results"];
            item = results[0];
            series = item["series"];
            values = series[0]["values"];
            for(int i = 0; i < values.size(); i++){
                // todo: 解析100行数据传入孤立森林中

            }
        }else if(nowLines > 100 && nowLines % 10 == 0){
            queryStatement = "select * from " + agentID + 
                        "where ContainerID='" + ContainerID + "'";
            influxdb_cpp::query(resp, queryStatement, client);
            res = jsonReader->parse(resp.c_str(), resp.c_str()+resp.length(), &root, &errs);
            if(!res || !errs.empty()){
                std::cout << "parse err: " << errs << std::endl; 
                exit(1);
            }
            results = root["results"];
            item = results[0];
            series = item["series"];
            values = series[0]["values"];
            for(int i = nowLines - 10; i < values.size(); i++){
                // todo: 超过100行后，每10行数据传入孤立森林中
                
            }
        }
    }
    // 将数据存入相应的agentID表中
    int ret = influxdb_cpp::builder()
            .meas(data.agentID)
            .tag("ContainerID", data.ContainerID)
            .field("CpuLoadAvg", data.CpuLoadAvg)
            .field("MemLoadAvg", data.MemLoadAvg)
            .field("DiskReadAvg", data.DiskReadAvg)
            .field("DiskWriteAvg", data.DiskWriteAvg)
            .field("NetReceiveAvg", data.NetReceiveAvg)
            .field("NetTransmitAvg", data.NetTransmitAvg)
            .timestamp(data.Timestamp)
            .post_http(client, &resp);
    if(ret){
        std::cout << resp << std::endl;
    }
}


void DataStorage::scanControllerTab(){
    influxdb_cpp::server_info client("127.0.0.1", 8086, "Docker_monitor");
    while(true){
        std::string resp;
        std::string queryStatement = "select * from ControllerTable";
        influxdb_cpp::query(resp, queryStatement, client);
        bool res;
        JSONCPP_STRING errs;
        Json::Value root, results, item, series, values;
        Json::CharReaderBuilder readerBuilder;
        std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
        res = jsonReader->parse(resp.c_str(), resp.c_str()+resp.length(), &root, &errs);
        if(!res || !errs.empty()){
            std::cout << "parse err:" << errs << std::endl;
            exit(1);
        }
        results = root["results"];
        item = results[0];
        series = item["series"];
        for(int i = 0; i < series.size(); i++){
            values = series[i]["values"];
            time_t nowTime = time(NULL);
            // todo: 需要进一步确认数据对应关系
            time_t timestamp = values[0][0].asInt();
            std::string ContainerID = values[0][1].asString();
            std::string agentID = values[0][2].asString();
            if(nowTime - timestamp >= 600){
                // 删除ControllerTable表中对应的行
                queryStatement = "delete from ControllerTable where agentID='"
                    + agentID + "' and ContainerID=" + ContainerID + "'";
                influxdb_cpp::query(resp, queryStatement, client);
                // 删除agentID表中对应的ContainerID行
                queryStatement = "delete from " + agentID + " where ContainerID='"
                    + ContainerID + "'";
                influxdb_cpp::query(resp, queryStatement, client);
            }
        }
        sleep(600);
    }
}


void DataStorage::runDataController(){
    std::thread t(&DataStorage::scanControllerTab, this);
    t.detach();
}