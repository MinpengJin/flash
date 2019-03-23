#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>

#include <websocketpp/common/thread.hpp>
#include <websocketpp/common/memory.hpp>

#include <cstdlib>
#include <iostream>
#include <map>
#include <vector>
#include <string>
#include <sstream>
#include "json/json.h"

typedef websocketpp::client<websocketpp::config::asio_client> client;

class connection_metadata {
private:
    // int m_id;
    std::string connectID;
    websocketpp::connection_hdl m_hdl;
    std::string m_status;
    std::string m_uri;
    std::string m_server;
    std::string m_error_reason;
    std::vector<std::string> m_messages;

public:
    typedef websocketpp::lib::shared_ptr<connection_metadata> ptr;

    connection_metadata(websocketpp::connection_hdl hdl, std::string uri)
      : m_hdl(hdl)
      , m_status("Connecting")
      , m_uri(uri)
      , m_server("N/A")
    {}

    void on_open(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Open";

        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
    }

    void on_fail(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Failed";

        client::connection_ptr con = c->get_con_from_hdl(hdl);
        m_server = con->get_response_header("Server");
        m_error_reason = con->get_ec().message();
    }
    
    void on_close(client * c, websocketpp::connection_hdl hdl) {
        m_status = "Closed";
        client::connection_ptr con = c->get_con_from_hdl(hdl);
        std::stringstream s;
        s << "close code: " << con->get_remote_close_code() << " (" 
          << websocketpp::close::status::get_string(con->get_remote_close_code()) 
          << "), close reason: " << con->get_remote_close_reason();
        m_error_reason = s.str();
    }

    void on_message(websocketpp::connection_hdl, client::message_ptr msg) {
        if (msg->get_opcode() == websocketpp::frame::opcode::text) {
            // 解析监控服务器发送的消息
            std::string message = msg->get_payload();
            std::string cmd, data;

            Json::Value jsonRoot;
            Json::CharReaderBuilder readerBuilder;
            unique_ptr<Json::CharReader> const reader(readerBuilder.newCharReader());
            JSONCPP_STRING errs;
            bool res = reader->parse(message.c_str(), message.c_str() + message.length(), &jsonRoot, &errs);
            if (!res || !errs.empty()) 
            {
                std::cerr << "parse Json error! " << errs << std::endl;
                Exit(-1);
            }else{
                cmd = jsonRoot["cmd"].asString();
                data = jsonRoot["data"].asString();
            }
            // 解析命令
            if(cmd=="id"){
                // 监控服务器分配给监控代理一个唯一的主机id
                connectID = msg->get_payload();
            }else if(cmd=="log"){
                // todo: 日志命令接口

            }else if(cmd=="cycle"){
                std::stringstream ss;
                ss.str(data);
                std::string item;
                std::vector<std::string> temp;
                while(getline(ss, item, ':')){
                    temp.push_back(item);
                }
                std::string ContainerID = temp[0];
                std::string cycleStr = temp[1];
                int cycle = atoi(cycleStr.c_str());
                ContainerSelection sel;
                sel.adjustContainerCycle(ContainerID, cycle);
            }
        } else {
            m_messages.push_back("<< " + websocketpp::utility::to_hex(msg->get_payload()));
        }
    }

    websocketpp::connection_hdl get_hdl() const {
        return m_hdl;
    }
    
    std::string get_id() const {
        return connectID;
    }
    
    std::string get_status() const {
        return m_status;
    }
};


class Transmission {
private:
    client t_client;
    websocketpp::lib::shared_ptr<websocketpp::lib::thread> t_thread;
    connection_metadata::ptr metadata_ptr;

public:
    Transmission ();
    ~Transmission();
    // 建立连接
    int connect(std::string const & uri);
    // 关闭连接
    inline void close();
    // 发送消息
    inline void send(std::string message);
};
# endif
