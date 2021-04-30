#pragma once

#include <iostream>
#include <thread>
#include <asio.hpp>

#include "connection.hpp"

namespace net
{
    enum Timer_Works: uint8_t
    {
        Clean_Up
    };

    class Server
    {
    private:
        asio::io_context io_context;
        asio::error_code ec;

        asio::ip::tcp::endpoint server_endpoint;
        asio::ip::tcp::acceptor server_acceptor;

        asio::steady_timer client_list_timer;
        int32_t client_list_timeout;

        uint8_t backlog;

        std::thread server_thread;

        std::vector<uint8_t> msg;
        std::vector<std::shared_ptr<Connection>> client_list;
    public:
        Server(uint16_t, uint8_t);
        ~Server();

        void waiting_connection();
        uint8_t connections();

        void start_timer(asio::steady_timer &, int32_t &, uint8_t);
        void cleanup_client();

        void asyn_recv(std::shared_ptr<Connection> &);
        // void asyn_send(std::shared_ptr<Connection> &);

        void run();
    };
    
    Server::Server(uint16_t port_num= 8989, uint8_t back= 8):
        server_endpoint(asio::ip::tcp::v4(), port_num), 
        server_acceptor(io_context, server_endpoint),
        client_list_timer(io_context),
        client_list_timeout(5),
        backlog(back)
    {
        // server_acceptor.listen(backlog);

        std::cout << "[server] startup .. " << std::endl;
        std::cout << "[server] listen at port : " << server_endpoint.port() << std::endl;
    }
    
    Server::~Server()
    {
        io_context.stop();
        if(server_thread.joinable())
        {
            server_thread.join();
        }

        std::cout << "[server] remaining connections : " << (int)connections() << std::endl;
        if (connections() > 0)
        {
            std::cout << "[server] clean up sockets .." << std::endl;

            for (std::vector<std::shared_ptr<Connection>>::iterator it = client_list.begin(); it != client_list.end(); it++)
            {
                (*it)->disconnect();
            }
        }

        std::cout << "[server] Shutdown Server .." << std::endl;
        server_acceptor.close();
    }
    uint8_t Server::connections()
    {
        return client_list.size();
    }
    void Server::waiting_connection()
    {
        server_acceptor.async_accept(
            [this](asio::error_code ec, asio::ip::tcp::socket sock)
            {
                if (!ec)
                {
                    if (connections() < backlog)
                    {
                        std::shared_ptr<Connection> new_conn = std::make_shared<Connection>(io_context, std::move(sock));
                        std::cout << "[client] " << new_conn->get_endpoint() << " connected !" << std::endl;

                        client_list.push_back(new_conn);
                        
                        // asyn_recv(new_conn);
                        new_conn->recv(msg);

                        waiting_connection();
                    }
                    else
                    {
                        std::cout << "[server] Connection refuse .." << std::endl;
                        sock.close();
                    }
                }
                else
                {
                    std::cerr << "[server] " << ec.message() << std::endl;
                }
                
            }
        );
    }
    void Server::asyn_recv(std::shared_ptr<Connection> &conn)
    {
        conn->recv(msg);
    }
    void Server::start_timer(asio::steady_timer &timer, int32_t &sec, uint8_t work)
    {
        timer.cancel();
        timer.expires_after(std::chrono::seconds(sec));
        timer.async_wait(
            [&, this](asio::error_code ec)
            {
                if (!ec)
                {
                    switch (work)
                    {
                    case Timer_Works::Clean_Up :
                        cleanup_client();
                        break;
                    
                    default:
                        break;
                    }
                    start_timer(timer, sec, work);
                }
            }
        );
    }
    void Server::cleanup_client()
    {
        for(std::vector<std::shared_ptr<Connection>>::iterator it = client_list.begin(); it!= client_list.end();)
        {
            if(!(*it)->is_alive())
            {
                std::cout << "[server] clean up 1 socket " << std::endl;
                it = client_list.erase(it); 
                // client_list.erase(std::remove(client_list.begin(), client_list.end(), *it), client_list.end());
            }
            else
            {
                it++;
            }
        }
    }
    void Server::run()
    {
        waiting_connection();
        start_timer(client_list_timer, client_list_timeout, Timer_Works::Clean_Up);

        server_thread = std::thread([this](){ io_context.run(); });

        std::cout << "[server] setup fin." << std::endl;
    }
    
} // namespace net
