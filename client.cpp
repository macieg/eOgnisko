#include <boost/bind.hpp>
#include "client.h"
#include "server.h"

void client::receive_tcp_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    if (!ec)
    {
        update_last_server_msg();
        std::ostringstream ss;
        ss << &stream_buffer_tcp;
        std::string message = ss.str();

        int id;
        if (client_parser.matches_client_id(message, id)) //jezeli otrzymalem swoj id, to odsylam go po udp jako potwierdzenie
        {
            sock_udp.async_send_to(asio::buffer(message), server_udp_endpoint,
                    [this](boost::system::error_code ec, std::size_t bt) {
                        if (!ec)
                        {
                            std::cerr << "[Info] ClientId successfully sended\n";
                            udp_listening(); //zaczynam czekać na udp od serwera
                            run_keepalive_timer(); //i ustawiam timer do KEEPALIVE
                        }
                        else
                            std::cerr << "[Error] Problem with sending client_id ec - '" << ec.message() << "', bt - " << bt << std::endl;
                    });
        }
        else
        {
            std::cerr << message;
        }

        asio::async_read_until(sock_tcp,
                stream_buffer_tcp, NEWLINE_SIGN,
                boost::bind(&client::receive_tcp_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "[Error] Receive tcp message failed, ec = '" + ec.message() + "'" << std::endl;
        throw connection_exception();
    }
}

void client::connect_tcp_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "[Info] Connect success\n";
        asio::async_read_until(sock_tcp,
                stream_buffer_tcp, NEWLINE_SIGN,
                boost::bind(&client::receive_tcp_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "[Error] Connect to server fail, ec = '" << ec.message() << "'" << std::endl;
        throw connection_exception();
    }
}

void client::send_upload_message(std::size_t bytes_read)
{
    if (bytes_read)
    {
        std::string upload_msg;
        upload_msg.append("UPLOAD ");
        upload_msg.append(std::to_string(my_nr_global));
        upload_msg.append("\n");
        upload_msg.append(stdin_buf.c_array(), bytes_read);

        //        std::cerr << "[Info] (" << upload_msg.size() << ")" << std::endl;

        sock_udp.async_send_to(asio::buffer(upload_msg),
                server_udp_endpoint,
                [this](boost::system::error_code ec, std::size_t bt) {
                    if (!ec)
                    {
                        //                        std::cerr << "[Info] Upload message successfully sended with bt - (" << bt << ")\n";
                    }
                    else
                        std::cerr << "[Error] While sending upload message ec - '" << ec.message() << "', bt - " << bt << std::endl;
                });
    }
    ++my_nr_global; //zwiększam licznik datagramow
}

void client::resolve_data_message(int nr, int ack, int win, int header_size, int bytes_transferred)
{
    server_nr_global = nr; //TODO czy na pewno się zgadza?
    win_global = win;

    //TODO jeżeli otrzymam 2xDATA bez ani jednego ACK to wysylam raz jeszcze ostatnią wiadomość
    //TODO jeżeli otrzymam datagram z numerem jakimś tam to też coś zrobić RETRANMISJE

    //wypisyje na wyjście to co odebrałem

    const char* data = udp_receive_buffer.data();
    int bytes_to_transfer = bytes_transferred - header_size;

    //    std::cerr << "[Info] bytes to be written - " << bytes_to_transfer << std::endl;
    asio::write(std_output, asio::buffer(data + header_size, bytes_to_transfer));

    //czytam z wejścia pewną ilość danych odpowiadającą win
    asio::async_read(std_input, asio::buffer(stdin_buf, win_global),
            [this, win] (const boost::system::error_code& ec, std::size_t bt) {//TODO
                if (!ec || ec.value() == EOF_ERR_NO)
                {
                    //                    std::cerr << this->stdin_buf.c_array() << std::endl;
                    //                    if (bt) std::cerr << "[Info] Read from input bt(" << bt << ") win(" << win << ")" << std::endl;
                    send_upload_message(bt);
                }
                else
                    std::cerr << "[Info] Problem with reading from stdin, ec = '" << ec.message() << "'" << std::endl;
            }
    );
}

void client::resolve_ack_message(int ack, int win)
{
    //    if (win) std::cerr << "[Info] received ACK ack(" << ack << ") win(" << win << ")" << std::endl;
    //    ack_global = ack;
    win_global = win;

    //TODO co z tym ack?
}

void client::udp_listening()
{
    sock_udp.async_receive_from(asio::buffer(udp_receive_buffer), ep_udp,
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec)
                {
                    if (this->ep_udp == this->server_udp_endpoint) //czy na pewno dostaje udp od dobrego serwera
                    {
                        //                        std::cerr << "[Info] Received datagram from server - (" << bt << ") bytes" << std::endl;
                        update_last_server_msg();

                        int nr, ack, win, begin_point;

                        const char* beg_buffer = udp_receive_buffer.c_array();
                        if (this->client_parser.matches_data(beg_buffer, nr, ack, win, bt, begin_point))
                            resolve_data_message(nr, ack, win, begin_point, bt);
                        else if (this->client_parser.matches_ack(beg_buffer, ack, win))
                            resolve_ack_message(ack, win);
                    }

                    this->udp_receive_buffer.assign(0);
                    udp_listening();
                }
                else
                {
                    std::cerr << "[Error] Problem with receiving udp '" << ec.message() << "'" << std::endl;
                    throw connection_exception();
                }
            });
}

void client::run_keepalive_timer()
{
    keepalive_timer.expires_from_now(std::chrono::milliseconds(keepalive_interval));
    keepalive_timer.async_wait([this](boost::system::error_code er) {
        if (!er)
        {
            sock_udp.async_send_to(asio::buffer(KEEPALIVE), server_udp_endpoint,
                    [this](boost::system::error_code ec, std::size_t bt) {
                        if (!ec)
                            run_keepalive_timer();
                        else
                            std::cerr << "[Error] Send keepalive error '" << ec.message() << "' " << bt << std::endl;
                    });
        }
        else
        {
            std::cerr << "[Error] Keepalive timer error " << er << std::endl;
        }
    });
}

void client::run_connection_timer()
{
    connect_timer.expires_from_now(std::chrono::milliseconds(connect_interval));
    connect_timer.async_wait([this](const boost::system::error_code & ec) {
        if (!ec)
        {
            const auto now = std::chrono::system_clock::now();
            auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(last_server_msg - now).count();
            if (diff > max_raport_interval)
            {
                std::cerr << "[Error] Server idle..." << std::endl;
                throw connection_exception();
            }
        }
        else
            std::cerr << "[Error] Connection timer error - '" << ec.message() << "'" << std::endl;
        run_connection_timer();
    });
}

void client::update_last_server_msg()
{
    last_server_msg = std::chrono::system_clock::now();
}

void client::setup_networking()
{
//    std::cerr << "[Info] Setup networking" << std::endl;
    my_nr_global = 0; //ustawienia nie sieciowe co prawda, ale tez potrzebne   
    //    ack_global = 0;
    win_global = 0;

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
connect_timer(io_service),
resolver_udp(io_service),
sock_udp(io_service, asio::ip::udp::endpoint(asio::ip::udp::v4(), 0)),
keepalive_timer(io_service),
std_input(io_service, ::dup(STDIN_FILENO)),
std_output(io_service, ::dup(STDOUT_FILENO)),
client_parser()
{
}

void client::setup(int retransmit_limit, std::string port, std::string server)
{
//    std::cerr << "[Info] Setup" << std::endl;
    this->retransmit_limit = retransmit_limit;
    this->port = std::move(port);
    this->server = std::move(server);

    setup_networking();
    run_connection_timer();
}
