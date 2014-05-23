#include <boost/bind.hpp>
#include "client.h"
#include "server.h"

void client::receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    //    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //std::cerr << "READ SUCCESS " << "\n";
        std::ostringstream ss;
        ss << &stream_buffer_tcp;
        std::cerr << ss.str();
    }

    asio::async_read_until(sock_tcp,
            stream_buffer_tcp, '\n',
            boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
            );
}

void client::send_udp_handler(const boost::system::error_code& ec, std::size_t bt)
{
    if (!ec)
    {
        std::cerr << "UDP SEND SUCCESS\n";
    }
    else
    {
        std::cerr << "UDP SEND FAIL " << ec << "\n";
    }
}

void client::connect_handler_tcp(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS TCP\n";
        asio::async_read_until(sock_tcp,
                stream_buffer_tcp, '\n',
                boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "CONNECT FAIL TCP\n";
    }
}

void client::connect_handler_udp(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS UDP\n";
    }
    else
    {
        std::cerr << "CONNECT FAIL UDP\n";
    }
}

void client::timer_handler(const boost::system::error_code& error)
{
    if (!error)
    {
        std::cerr << "TIMER HANDLER\n";
        std::string a("a"); //TODO poprawić to całe diabelstwo na 
        asio::async_write(sock_tcp, asio::buffer(a), [this](boost::system::error_code err, std::size_t s) {
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
    connect_timer.expires_from_now(boost::posix_time::millisec(connect_interval));
    connect_timer.async_wait(boost::bind(&client::timer_handler, this, asio::placeholders::error));
}

void client::setup_networking()
{
    asio::ip::tcp::resolver::query query_tcp(server, port);
    resolver_tcp.async_resolve(query_tcp,
            [this](boost::system::error_code ec, asio::ip::tcp::resolver::iterator it) {
                if (!ec)
                {
                    sock_tcp.async_connect(*it, boost::bind(&client::connect_handler_tcp, this, asio::placeholders::error));
                }
            }
    );

    asio::ip::udp::resolver::query query_udp(server, port);
    
    resolver_udp.async_resolve(query_udp,
            [this](boost::system::error_code ec, asio::ip::udp::resolver::iterator it) {
                if (!ec)
                {
                    std::cerr << "KURWA DZIALA " << "\n";
                    sock_udp.async_send_to(asio::buffer("yy kurwy"), *it,
                            boost::bind(&client::send_udp_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                            );
                }
            }
    );
}

client::client(asio::io_service& io_service) : resolver_tcp(io_service),
sock_tcp(io_service),
connect_timer(io_service, boost::posix_time::seconds(connect_interval)),
resolver_udp(io_service),
sock_udp(io_service),
keepalive_timer(io_service, boost::posix_time::seconds(keepalive_interval))
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
