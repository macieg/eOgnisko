#include <boost/bind.hpp>
#include "client.h"
#include "server.h"

void client::receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    //    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //std::cerr << "READ SUCCESS " << "\n";
        last_raport_time = boost::posix_time::second_clock::local_time();

        std::ostringstream ss;
        ss << &stream_buffer_tcp;
        std::cerr << ss.str();
        
        stream_buffer_udp = stream_buffer_tcp;
        std::string s = ss.str();
        int id;
        if (client_parser.matches_client_id(s, id)) {
            //TODO rano send udp
        }
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
        std::cerr << "CONNECT FAIL TCP " << ec.value() << "\n";
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
        boost::posix_time::time_duration tm(boost::posix_time::second_clock::local_time() - last_raport_time);
        std::cerr << "TM: " << tm.total_milliseconds() << "\n";
        if (tm.total_milliseconds() > max_raport_interval)
        {
            std::cerr << "BRAK POLACZENIA\n";
            sock_tcp.close();
            setup_networking();
        }

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
    last_raport_time = boost::posix_time::second_clock::local_time();

    asio::ip::tcp::resolver::query query_tcp(server, port);
    asio::ip::tcp::resolver::iterator it_tcp = resolver_tcp.resolve(query_tcp);
    sock_tcp.async_connect(*it_tcp, boost::bind(&client::connect_handler_tcp, this, asio::placeholders::error));

    asio::ip::udp::resolver::query query_udp(server, port);
    asio::ip::udp::resolver::iterator it_udp = resolver_udp.resolve(query_udp);
}

client::client(asio::io_service& io_service) : resolver_tcp(io_service),
sock_tcp(io_service),
connect_timer(io_service, boost::posix_time::seconds(connect_interval)),
resolver_udp(io_service),
sock_udp(io_service),
keepalive_timer(io_service, boost::posix_time::seconds(keepalive_interval)),
client_parser()
{
}

void client::setup(int retransmit_limit, std::string port, std::string server)
{
    this->retransmit_limit = retransmit_limit;
    this->port = std::move(port);
    this->server = std::move(server);

    setup_networking();
    setup_timer();
}
