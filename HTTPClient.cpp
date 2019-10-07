//
// Created by admin on 2018/10/14.
//

#include "HTTPClient.h"
#include "Utils.h"
#include "commen.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace http = boost::beast::http;    // from <boost/beast/http.hpp>

HTTPClient::HTTPClient() : port_(-1) {
}

std::string HTTPClient::get() {
    return get(host_, port_, url_, key_);
}

std::string HTTPClient::get(const std::string &host, int port, const std::string &url, const std::string &key) {
    std::string resString;
    try {
        if (host.empty() || url.empty() || port > 65535 || port < 0) {
            LOG_ERROR << "host: " << host << ", port: " << port << ", request url: " << url;
            resString = "host or port or url invalid!";
            return resString;
        }

        LOG_INFO << "host: " << host << ", port: " << port << ", request URL: " << url;

        tcp::socket socket{ioc};
        // These objects perform our I/O
        tcp::resolver resolver{ioc};
        // Look up the domain name
        auto results = resolver.resolve(host, std::to_string(port));

        // Make the connection on the IP address we get from a lookup
        boost::asio::connect(socket, results.begin(), results.end());

        try {
            // Set up an HTTP GET request message
            int version = 11; //http 1.1
            http::request<http::string_body> req{http::verb::get, url, version};
            // todo secket.wait socket.async
            req.set(http::field::host, socket.local_endpoint().address().to_string());
            req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
            // Send the HTTP request to the remote host
            http::write(socket, req);
            // This buffer is used for reading and must be persisted
            boost::beast::flat_buffer buffer;
            // Declare a container to hold the response
            http::response<http::dynamic_body> res;
            // Receive the HTTP response
            http::read(socket, buffer, res);

            if (res.result() != boost::beast::http::status::ok) throw (res);

            std::stringstream ss;
            ss << boost::beast::buffers(res.body().data());
            if (key.empty()) {
                resString = ss.str();
            } else {
                try {
                    boost::property_tree::ptree pt;
                    boost::property_tree::read_json<boost::property_tree::ptree>(ss, pt);
                    auto keys = Utils::splitString(key, ";");
                    for (int i = 0, n = keys.size(); i < n; ++i) {
                        if (i == n - 1) {
                            resString = pt.get<std::string>(keys[i].c_str());
                        } else {
                            pt = pt.get_child(keys[i].c_str());
                        }
                    }
                } catch (std::exception const &e) {
                    LOG_ERROR << "error when parsing result: " << e.what();
                }
            }

            try {
                // Gracefully close the socket
                boost::system::error_code ec;
                socket.shutdown(tcp::socket::shutdown_both, ec);

                // not_connected happens sometimes
                // so don't bother reporting it.
                if (ec && ec != boost::system::errc::not_connected)
                    throw boost::system::system_error{ec};

                // If we get here then the connection is closed gracefully
            } catch (std::exception const &e) {
                LOG_ERROR << "close socket error: " << e.what();
            }
        } catch (std::exception const &e) {
            LOG_ERROR << "error when requesting url: " << url << ", " << e.what();
        } catch (const http::response<http::dynamic_body> &e) {
            LOG_ERROR << "error when requesting url: " << url;
            LOG_ERROR << e;
        }
    } catch (std::exception &e) {
        LOG_ERROR << "http connect error! " << e.what()
                  << ", host: " << host << ", size: " << host.size()
                  << ", port: " << port
                  << ", request url: " << url;
    }
    return resString;
}

HTTPClient *HTTPClient::getInstance() {
    static HTTPClient instance;
    return &instance;
}

HTTPClient::~HTTPClient() {

}
