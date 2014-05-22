#include <boost/bind.hpp>
#include "client.h"

void client::receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    //    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //std::cerr << "READ SUCCESS " << "\n";
        std::ostringstream ss;
        ss << &stream_buffer;
        std::cerr << ss.str();
    }

    asio::async_read_until(sock,
            stream_buffer, '\n',
            boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
            );
}

void client::connect_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS \n";
        asio::async_read_until(sock,
                stream_buffer, '\n',
                boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "CONNECT FAIL\n";
    }
}

void client::timer_handler(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cerr << "TIMER HANDLER\n";
        std::string a("a");
        asio::async_write(sock, asio::buffer(a), [this](boost::system::error_code err, std::size_t s) {
            if (err)
            {
                std::cerr << "BRAK POLACZENIA\n";
                setup_networking();
            }
        });
        setup_timer();
    }
    else
    {
        std::cerr << "TIMER HANDLER ERROR\n";
    }
}

void client::setup_timer()
{
    connect_timer.expires_from_now(boost::posix_time::millisec(TIMER_INTERVAL));
    connect_timer.async_wait(boost::bind(&client::timer_handler, this, asio::placeholders::error));
}

void client::setup_networking()
{
    asio::ip::tcp::resolver::query query(server, port);
    resolver.async_resolve(query,
            [this](boost::system::error_code ec, asio::ip::tcp::resolver::iterator it) {
                if (!ec)
                {
                    sock.async_connect(*it, boost::bind(&client::connect_handler, this, asio::placeholders::error));
                }
            }
    );
}

client::client(asio::io_service& io_service) : resolver(io_service), sock(io_service), connect_timer(io_service, boost::posix_time::seconds(TIMER_INTERVAL))
{
}

void client::setup(int retransmit_limit, std::string port, std::string server)
{
    setup_timer();
    setup_networking();

    this->retransmit_limit = retransmit_limit;
    this->port = std::move(port);
    this->server = std::move(server);
}
