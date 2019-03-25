#ifndef SERVERTRANSMISSION_H
#define SERVERTRANSMISSION_H

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "json/json.h"

#include <websocketpp/common/thread.hpp>

typedef websocketpp::server<websocketpp::config::asio> server;

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::lock_guard;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;


struct Connection
{
    std::string agentID;
    websocketpp::connection_hdl hdl;
};


class ServerTransmission {
private:
    
    int id = 0;                                         // 分配给监控代理的id

    server m_server;
    std::vector<Connection> s_connections;              // 存储所有与监控服务器建立的连接
    std::map<connection_hdl, std::string> link;         // handler和监控代理id的关系
    
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> s_thread;

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, server::message_ptr msg);

public:
    ServerTransmission(uint16_t port);
    ~ServerTransmission();

    void sendMessage(std::string agentID);
};
#endif
