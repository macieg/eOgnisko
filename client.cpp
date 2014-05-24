#include <boost/bind.hpp>
#include "client.h"
#include "server.h"

void client::receive_tcp_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (!ec)
    {
        //std::cerr << "READ SUCCESS " << "\n";
        last_raport_time = boost::posix_time::second_clock::local_time();

        std::ostringstream ss;
        ss << &stream_buffer_tcp;
        std::cerr << ss.str();
        
        std::string confirm = ss.str();
//        std::cerr << "|" << confirm << "|\n";
        int id;
        if (client_parser.matches_client_id(confirm, id)) //jezeli otrzymalem swoj id, to odsylam go po udp jako potwierdzenie
        {
            sock_udp.async_send_to(asio::buffer(confirm), server_udp_endpoint,
                    [this](boost::system::error_code ec, std::size_t bt) {
                        if (!ec)
                        {
                            std::cerr << "ODESLANO CLIENTID POPRAWNIE\n";
                            read_std_in();
                            udp_listening(); //zaczynam czekać na udp od serwera
                            run_keepalive_timer(); //i ustawiam timer do KEEPALIVE
                        }
                        else
                            std::cerr << "BLAD PODCZAS WYSYLANIA CLIENTID " << ec << " " << bt << "\n";
                    });
        }

        asio::async_read_until(sock_tcp,
                stream_buffer_tcp, '\n',
                boost::bind(&client::receive_tcp_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        keepalive_timer.cancel();
        std::cerr << "READ FAILED " << ec << "\n";
    }
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

void client::connect_tcp_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS TCP\n";
        asio::async_read_until(sock_tcp,
                stream_buffer_tcp, '\n',
                boost::bind(&client::receive_tcp_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "CONNECT FAIL TCP " << ec.value() << "\n";
    }
}

void client::read_std_in()
{
    asio::async_read(std_input, asio::buffer(stdin_buf),
            [this](const boost::system::error_code& ec, std::size_t bt) {
                std::cerr << stdin_buf.c_array();
                if (ec)
                    std::cerr << "READ STDIN FAILED " << ec << "\n";
            }
    );
}

void client::udp_listening()
{
    //    std::cerr << "START UDP LISTENING\n";

    sock_udp.async_receive_from(asio::buffer(udp_receive_buffer), ep_udp,
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec)
                {
                    if (this->ep_udp == this->server_udp_endpoint) //czy na pewno dostaje udp od dobrego serwera
                    {
//                        std::cerr << "OTRZYMANO UDP\n";
                        int nr, ack, win;
                        std::string data;
                        
                        if (this->client_parser.matches_data(udp_receive_buffer.c_array(), nr, ack, win, data))
                        {
                            std::cerr << "|" << data << "|\n";
//asynchroniczne pisanie na wyjscie                            
//                            asio::async_write(std_output, asio::buffer(data),
//                                [this](boost::system::error_code ec, std::size_t bt) {
//                                    if (ec)
//                                        std::cerr << "PROBLEM Z ZAPISEM NA STDOUT" << ec << "\n";
//                                });
                        }
                    }
                    udp_listening();
                }
                else
                {
                    std::cerr << "BLAD PRZY UZYSKIWANIU UDP " << ec << "\n";
                }
            });
}

void client::connection_timer_handler(const boost::system::error_code& error)
{
    if (!error)
    {
//        std::cerr << "CONNECTION TIMER HANDLER\n";
        boost::posix_time::time_duration tm(boost::posix_time::second_clock::local_time() - last_raport_time);
//        std::cerr << "TM: " << tm.total_milliseconds() << "\n";
        if (tm.total_milliseconds() > max_raport_interval)
        {
            std::cerr << "BRAK POLACZENIA\n";
            sock_tcp.close();
            setup_networking();
        }

        run_connection_timer();
    }
    else
    {
        std::cerr << "TIMER HANDLER ERROR\n";
    }
}

void client::run_keepalive_timer()
{
    keepalive_timer.expires_from_now(boost::posix_time::millisec(keepalive_interval));
    keepalive_timer.async_wait([this](boost::system::error_code er) {
        if (!er)
        {
            sock_udp.async_send_to(asio::buffer(KEEPALIVE), server_udp_endpoint,
                    [this](boost::system::error_code ec, std::size_t bt) {
                        if (!ec)
                        {
                            //                            std::cerr << "WYSLANO POPRAWNIE KEEPALIVE\n";
                            run_keepalive_timer();
                        }
                        else
                            std::cerr << "BLAD PODCZAS WYSYLANIA KEEPALIVE " << ec << " " << bt << "\n";
                    });
        }
        else
        {
            std::cerr << "KEEPALIVE TIMER ERROR " << er << "\n";
        }
    });
}

void client::run_connection_timer()
{
    connect_timer.expires_from_now(boost::posix_time::millisec(connect_interval));
    connect_timer.async_wait(boost::bind(&client::connection_timer_handler, this, asio::placeholders::error));
}

void client::setup_networking()
{
    last_raport_time = boost::posix_time::second_clock::local_time();

    asio::ip::tcp::resolver::query query_tcp(server, port);
    asio::ip::tcp::resolver::iterator it_tcp = resolver_tcp.resolve(query_tcp);
    sock_tcp.async_connect(*it_tcp, boost::bind(&client::connect_tcp_handler, this, asio::placeholders::error));

    asio::ip::udp::resolver::query query_udp(server, port);
    asio::ip::udp::resolver::iterator it_udp = resolver_udp.resolve(query_udp);
    server_udp_endpoint = *it_udp;
}

client::client(asio::io_service& io_service) :
KEEPALIVE("KEEPALIVE\n"),
resolver_tcp(io_service),
sock_tcp(io_service),
connect_timer(io_service, boost::posix_time::seconds(connect_interval)),
resolver_udp(io_service),
sock_udp(io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
keepalive_timer(io_service, boost::posix_time::seconds(keepalive_interval)),
std_input(io_service, ::dup(STDIN_FILENO)),
std_output(io_service, ::dup(STDOUT_FILENO)),
client_parser()
{
}

void client::setup(int retransmit_limit, std::string port, std::string server)
{
    this->retransmit_limit = retransmit_limit;
    this->port = std::move(port);
    this->server = std::move(server);

    setup_networking();
    run_connection_timer();
}
