#include "DataStorage.h"

static bool ControllerTabExist = false;
static influxdb_cpp::server_info client("127.0.0.1", 8086, "Docker_monitor");

DataStorage::DataStorage(){}

DataStorage::~DataStorage(){}

void DataStorage::storeData(StorageFormat data){
    // 查找存储控制表，判断当前agentID和ContainerID是否已经存在
    std::string resp;
    std::string queryStatement = "select * from ControllerTable where agentID='"
                 + data.agentID +"' and ContainerID='" + data.ContainerID + "'";
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
            .tag("agentID", data.agentID)
            .tag("ContainerID", data.ContainerID)
            .field("lines", 1)
            .field("Timestamp", timestamp)
            .timestamp(timestamp)
            .post_http(client, &resp);
        if(ret){
            std::cout << resp << std::endl;
        }
    }else{
        // 当前agentID和ContainerID存在，获取当前行数并加一
        std::string queryStatement = "delete from ControllerTable where agentID='"
                 + data.agentID +"' and ContainerID='" + data.ContainerID + "'";
        influxdb_cpp::query(resp, queryStatement, client);

        values = series[0]["values"];
        int nowLines = values[0][4].asInt() + 1;
        int ret = influxdb_cpp::builder()
            .meas("ControllerTable")
            .tag("agentID", data.agentID)
            .tag("ContainerID", data.ContainerID)
            .field("lines", nowLines)
            .field("Timestamp", timestamp)
            .timestamp(timestamp)
            .post_http(client, &resp);
        if(ret){
            std::cout << resp << std::endl;
        }
        if(nowLines == 100){
            // 查询并解析agentID表中相应ContainerID的数据
            queryStatement = "select * from " + data.agentID + 
                        "where ContainerID='" + data.ContainerID + "'";
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
            queryStatement = "select * from " + data.agentID + 
                        "where ContainerID='" + data.ContainerID + "'";
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
            .field("Timestamp", data.Timestamp)
            .timestamp(data.Timestamp)
            .post_http(client, &resp);
    if(ret){
        std::cout << resp << std::endl;
    }
}


void DataStorage::scanControllerTab(){
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
        if(!series.empty()){
            values = series[0]["values"];
            time_t nowTime = time(NULL);
            for(int i = 0; i < values.size(); i++){
                std::string ContainerID = values[i][1].asString();
                time_t timestamp = values[i][2].asInt();
                std::string agentID = values[i][3].asString();

                if(nowTime - timestamp >= 600){
                    // 删除ControllerTable表中对应的行
                    queryStatement = "delete from ControllerTable where ContainerID='"
                        + ContainerID + "' and agentID='" + agentID + "'";
                    influxdb_cpp::query(resp, queryStatement, client);
                    // 删除agentID表中对应的ContainerID行
                    queryStatement = "delete from " + agentID + " where ContainerID='"
                        + ContainerID + "'";
                    influxdb_cpp::query(resp, queryStatement, client);
                }
            }
        }
        sleep(600);
    }
}


void DataStorage::runDataController(){
    std::thread t(&DataStorage::scanControllerTab, this);
    t.detach();
}

