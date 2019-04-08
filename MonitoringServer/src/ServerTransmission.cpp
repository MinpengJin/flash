#include "ServerTransmission.h"

ServerTransmission::ServerTransmission(uint16_t port) {
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_open_handler(bind(&ServerTransmission::on_open,this,::_1));
    m_server.set_close_handler(bind(&ServerTransmission::on_close,this,::_1));
    m_server.set_message_handler(bind(&ServerTransmission::on_message,this,::_1,::_2));

    // listen on specified port
    m_server.listen(port);
    // Start the server accept loop
    m_server.start_accept();
    // Start the ASIO io_service run loop
    s_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&server::run, &m_server);
}


ServerTransmission::~ServerTransmission(){
    s_thread->join();
}


void ServerTransmission::on_open(connection_hdl hdl) {
    // 将新打开的连接添加到s_connections
    Connection newConnection;
    std::stringstream ss;
    ss << id;
    newConnection.agentID = "x" + ss.str();
    newConnection.hdl = hdl;
    s_connections.push_back(newConnection);
    id++;
    // 创建handler和监控代理id的关系
    // link[newConnection.hdl] = newConnection.agentID;

    // 将数据转化成json格式
    Json::Value root;
    Json::StreamWriterBuilder writerBuilder;
    std::ostringstream os;

    root["cmd"] = "CreateID";
    root["agentID"] = "";
    root["data"] = newConnection.agentID;
    std::unique_ptr<Json::StreamWriter> writer(writerBuilder.newStreamWriter());
    writer->write(root, &os);
    std::string message = os.str();
    // 将转换好的json数据发送给监控代理
    websocketpp::lib::error_code ec;
    m_server.send(hdl, message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "[Server] Error sending message: " << ec.message() << std::endl;
        return;
    }
}


void ServerTransmission::on_close(connection_hdl hdl) {
    /*
    std::string agentID = link[hdl];
    // 删除s_connections中对应的连接
    auto it = s_connections.begin();
    while(it != s_connections.end()){
        if(it->agentID == agentID){
            break;
        }
    }
    s_connections.erase(it);
    // 删除hadler和监控代理id的关系
    auto it_link = link.find(hdl);
    link.erase(it_link);
    */
}


void ServerTransmission::on_message(connection_hdl hdl, server::message_ptr msg) {
    std::string message = msg->get_payload();

    /*
     * for test
     */
    std::cout<<"get msg from client: "<<message<<std::endl;

    // 解析传入数据
    bool res;
    JSONCPP_STRING errs;
    Json::Value root, data;
    Json::CharReaderBuilder readerBuilder;
    
    std::unique_ptr<Json::CharReader> const jsonReader(readerBuilder.newCharReader());
    res = jsonReader->parse(message.c_str(), message.c_str()+message.length(), &root, &errs);
    if(!res || !errs.empty()){
        std::cout << "parse err: " << errs << std::endl; 
        exit(1);
    }

    std::string cmd = root["cmd"].asString();
    // 判断传入数据类型
    if(cmd=="ContainerData"){
        data = root["data"];
        StorageFormat temp;
        // 将数据存入influxdb
        temp.agentID = data["agentID"].asString();
        temp.ContainerID = data["ContainerID"].asString();
        temp.Timestamp = data["Timestamp"].asInt();
        temp.CpuLoadAvg = data["CpuLoadAvg"].asFloat();
        temp.MemLoadAvg = data["MemLoadAvg"].asFloat();
        temp.DiskReadAvg = data["DiskReadAvg"].asFloat();
        temp.DiskWriteAvg = data["DiskWriteAvg"].asFloat();
        temp.NetReceiveAvg = data["NetReceiveAvg"].asFloat();
        temp.NetTransmitAvg = data["NetTransmitAvg"].asFloat();

        DataStorage dataStorage;
        dataStorage.storeData(temp);
    }else if(cmd=="ContainerLogs"){
        // todo：将数据存入mysql

    }
}


void ServerTransmission::sendMessage(std::string agentID) {
    // todo: 监控服务器向监控代理传输数据的接口

}