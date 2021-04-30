#pragma once

#include <iostream>
#include <sstream>
#include <thread>
#include <string>
#include <vector>

#include <asio.hpp>

namespace net
{
    class Connection
    {
    private:
        asio::io_context &context;
        asio::error_code ec;

        asio::ip::tcp::socket socket;

        std::string client_endpoint;
        
    public:
        Connection(asio::io_context &, asio::ip::tcp::socket);
        ~Connection();

        bool is_alive();

        void disconnect();

        std::string get_endpoint();

        void recv(std::vector<uint8_t> &);
        void send(std::vector<uint8_t> &);
    };
    
    Connection::Connection(asio::io_context &server_context, asio::ip::tcp::socket sock): 
        context(server_context),
        socket(std::move(sock))
    {
        std::stringstream ss;
        ss << socket.remote_endpoint();
        ss >> client_endpoint;
    }
    
    Connection::~Connection()
    {
    }
    bool Connection::is_alive()
    {
        return socket.is_open();
    }
    std::string Connection::get_endpoint()
    {
        return client_endpoint;
    }
    void Connection::disconnect()
    {
        if (is_alive())
        {
            std::cout << "[server] client " << get_endpoint() << " Disconnect .. close socket" << std::endl;
            socket.close();
        }
    }
    
    void Connection::recv(std::vector<uint8_t> &msg)
    {
        msg.clear();
        msg.resize(1024);

        socket.async_read_some(
            asio::buffer(msg.data(), msg.size()),
            [&, this](asio::error_code ec, size_t len)
            {
                if(!ec)
                {
                    std::cout << "[" << get_endpoint() << "] : " << msg.data() << std::endl;
                
                    send(msg);
                    recv(msg);
                }
                else
                {
                    // std::cerr << "aaa"<< ec.message() << std::endl;
                    if (ec == asio::error::eof)
                    {
                        disconnect();
                    }
                }
            }
        );
    }
    void Connection::send(std::vector<uint8_t> &msg)
    {
        socket.async_write_some(
            asio::buffer(msg.data(), msg.size()),
            [&, this](asio::error_code ec, size_t len)
            {
                if(ec)
                {
                    std::cerr << "[" << get_endpoint() << "] : " << ec.message() << std::endl;
                }
            }
        );
    }
} // namespace net
