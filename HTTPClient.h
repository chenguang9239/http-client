//
// Created by admin on 2018/10/14.
//

#ifndef CPPSERVER_HTTPClient_H
#define CPPSERVER_HTTPClient_H

#include <string>
#include <boost/asio/ip/tcp.hpp>

using tcp = boost::asio::ip::tcp;

class HTTPClient {
    std::string host_;
    int port_;
    std::string url_;
    std::string key_;
    boost::asio::io_context ioc; // The io_context is required for all I/O

public:
    HTTPClient();

    ~HTTPClient();

    void setHost(const std::string &host) { host_ = host; }

    void setPort(int port) { port_ = port; }

    void setURL(const std::string &url) { url_ = url; }

    void setKey(const std::string &key) { key_ = key; }

    std::string get();

    std::string get(const std::string &host, int port, const std::string &url, const std::string &key = "");

    static HTTPClient *getInstance();
};


#endif //CPPSERVER_HTTPClient_H
