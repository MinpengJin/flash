#include "ClientTransmission.h"

ClientTransmission::ClientTransmission () {}


ClientTransmission::~ClientTransmission() {
    t_client.stop_perpetual();
    // 如果连接没有关闭，则关闭连接
    if (metadata_ptr->get_status() == "Open") {
        std::cout << " Closing connection! " << std::endl;
        websocketpp::lib::error_code ec;
        t_client.close(metadata_ptr->get_hdl(), websocketpp::close::status::going_away, "", ec);
        if (ec) {
            std::cout << "[Client] Error closing connection " << ": "  
                        << ec.message() << std::endl;
        }
    }
    // 阻塞线程
    t_thread->join();
}


void ClientTransmissin::initTransmission() {
    t_client.clear_access_channels(websocketpp::log::alevel::all);
    t_client.clear_error_channels(websocketpp::log::elevel::all);
    // 初始化传输模块并将其设置为永久模式
    t_client.init_asio();
    t_client.start_perpetual();
    // 给传输模块创建监听线程
    t_thread = websocketpp::lib::make_shared<websocketpp::lib::thread>(&client::run, &t_client);
}


int ClientTransmission::connect(std::string const & uri) {
    // 创建一个新的连接
    websocketpp::lib::error_code ec;
    client::connection_ptr con = t_client.get_connection(uri, ec);
    if (ec) {
        std::cout << "[Client] Connect initialization error: " << ec.message() << std::endl;
        return 1;
    }
    metadata_ptr = websocketpp::lib::make_shared<connection_metadata>(con->get_handle(), uri);
    // 设置连接句柄
    con->set_open_handler(websocketpp::lib::bind(
        &connection_metadata::on_open,
        metadata_ptr,
        &t_client,
        websocketpp::lib::placeholders::_1
    ));
    con->set_fail_handler(websocketpp::lib::bind(
        &connection_metadata::on_fail,
        metadata_ptr,
        &t_client,
        websocketpp::lib::placeholders::_1
    ));
    con->set_close_handler(websocketpp::lib::bind(
        &connection_metadata::on_close,
        metadata_ptr,
        &t_client,
        websocketpp::lib::placeholders::_1
    ));
    con->set_message_handler(websocketpp::lib::bind(
        &connection_metadata::on_message,
        metadata_ptr,
        websocketpp::lib::placeholders::_1,
        websocketpp::lib::placeholders::_2
    ));
    // 开始给定连接的连接过程
    t_client.connect(con);
    return 0;
}


void ClientTransmission::close() {
    websocketpp::lib::error_code ec;
    //  关闭连接
    t_client.close(metadata_ptr->get_hdl(), websocketpp::close::status::going_away, "", ec);
    if (ec) {
        std::cout << "[Client] Error initiating close: " << ec.message() << std::endl;
        return;
    }
}


void ClientTransmission::send(std::string message) {
    websocketpp::lib::error_code ec;
    t_client.send(metadata_ptr->get_hdl(), message, websocketpp::frame::opcode::text, ec);
    if (ec) {
        std::cout << "[Client] Error sending message: " << ec.message() << std::endl;
        return;
    }
}


std::string ClientTransmission::getAgentID(){
    return agentID;
}

/* 
 *----test----
int main(){
    ClientTransmission trans;
    std::string uri;
    std::cout << "> please input the uri:";
    std::cin >> uri;
    while(trans.connect(uri)){
        std::cout << "> please input the uri again:";
        std::cin >> uri;
    }
    std::string cmd;
    bool flag = true;
    while(flag){
        std::cout << "> please input the command:";
        std::cin >> cmd;
        if(cmd == "quit"){
            flag = false;
        }else if(cmd == "message"){
            std::string message;
            std::cout << "> please input the message:";
            std::cin >> message;
            trans.send(message);
        }else{
            std::cout << "> Unrecognized Command";
        }
    }
    trans.close();
    return 0;
}
*/
