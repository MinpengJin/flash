#include "Transmission.h"

Transmission::Transmission(uint16_t port) {
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_open_handler(bind(&broadcast_server::on_open,this,::_1));
    m_server.set_close_handler(bind(&broadcast_server::on_close,this,::_1));
    m_server.set_message_handler(bind(&broadcast_server::on_message,this,::_1,::_2));

    // listen on specified port
    m_server.listen(port);
    // Start the server accept loop
    m_server.start_accept();
    // Start the ASIO io_service run loop
    s_thread(bind(&server::run, &m_server);
    // try {
    // } catch (const std::exception & e) {
    //     std::cout << e.what() << std::endl;
    // }
}


Transmission::~Transmission(){

    s_thread.join();
}


void Transmission::on_open(connection_hdl hdl) {
    Connection newConneciton;
    stringstream ss;
    ss << id;
    newConneciton.agentID = ss.str()
    newConneciton.hdl = hdl;
    s_connections.push_back(newConneciton);
    link[newConneciton.hdl] = newConneciton.agentID;
    id++;
}


void Transmission::on_close(connection_hdl hdl) {
    std::string agentID = link[hdl];
    auto it = s_connections.begin();
    while(it != s_connections.end()){
        if(it->agentID == agentID){
            break;
        }
    }
    s_connections.erase(it);
}


void Transmission::on_message(connection_hdl hdl, server::message_ptr msg) {
    // todo: 解析监控代理传来的数据

}


void sendMessage(agentID) {
    // todo: 监控服务器向监控代理传输数据的接口

}