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

void client::send_upload_message(std::size_t bytes_read)
{
    if (bytes_read)
    {
        std::string upload_msg;
        upload_msg.append("UPLOAD ");
        upload_msg.append(std::to_string(nr_global));
        upload_msg.append("\n");
        upload_msg.append(stdin_buf.c_array());

        //        std::cerr << "UPLOAD MSG (" << bytes_read << ") (" << upload_msg << ")";
        sock_udp.async_send_to(asio::buffer(upload_msg, bytes_read + upload_msg.size()),
                server_udp_endpoint,
                [this](boost::system::error_code ec, std::size_t bt) {
                    if (!ec)
                    {
                        //                        std::cerr << "WYSLANO DANE Z WEJSCIA DO SERWERA (" << bt << ")\n";
                    }
                    else
                        std::cerr << "PROBLEM PODCZAS WYSYLANIA DANYCH Z WEJSCIA DO SERWERA " << ec << " " << bt << "\n";
                });
    }
    ++nr_global; //zwiększam licznik datagramow
}

void client::resolve_data_message(int nr, int ack, int win, std::string& data)
{
    win_global = win;
    //    std::cerr << "|" << data << "|\n";
    //TODO jeżeli otrzymam 2xDATA bez ani jednego ACK to wysylam raz jeszcze ostatnią wiadomość
    //TODO jeżeli otrzymam datagram z numerem jakimś tam to też coś zrobić RETRANMISJE

    //wypisyje na wyjście to co odebrałem
    //                            asio::async_write(std_output, asio::buffer(data),
    //                                [this](boost::system::error_code& ec, std::size_t bt) {
    //                                    if (ec)
    //                                        std::cerr << "PROBLEM Z ZAPISEM NA STDOUT" << ec << "\n";
    //                                });

    //czytam z wejścia pewną ilość danych odpowiadającą win
    asio::async_read(std_input, asio::buffer(stdin_buf, win_global),
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec || ec.value() == EOF_ERR_NO)
                {
                    //                    std::cerr << this->stdin_buf.c_array() << "\n";
                    if (bt) std::cerr << "[DATA] WCZYTANO Z WEJSCIA (" << bt << ") bajtow, a chcialem (" << win_global << ")\n";
                    send_upload_message(bt);
                }
                else
                    std::cerr << "[DATA] READ STDIN ERROR " << ec.value() << "\n";
            }
    );
}

void client::resolve_ack_message(int ack, int win)
{
    std::cerr << "[ACK] MESSAGE (" << ack << ") (" << win << ")\n";
    ack_global = ack;
    win_global = win;

    //TODO co z tym ack?
    asio::async_read(std_input, asio::buffer(stdin_buf, win_global),
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec || ec.value() == EOF_ERR_NO)
                {
                    //                    std::cerr << this->stdin_buf.c_array() << "\n";
                    if (bt)
                    {
                        std::cerr << "[ACK] WCZYTANO Z WEJSCIA (" << bt << ") bajtow, a chcialem (" << win_global << ")\n";
                        send_upload_message(bt);
                    }
                }
                else
                    std::cerr << "[ACK] READ STDIN ERROR " << ec.value() << "\n";
            }
    );

}

void client::udp_listening()
{
    sock_udp.async_receive_from(asio::buffer(udp_receive_buffer), ep_udp,
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec)
                {
                    if (this->ep_udp == this->server_udp_endpoint) //czy na pewno dostaje udp od dobrego serwera
                    {
                        //                        std::cerr << "OTRZYMANO UDP OD SERWERA (" << udp_receive_buffer.c_array() << ")\n";
                        int nr, ack, win;
                        std::string data;

                        if (this->client_parser.matches_data(udp_receive_buffer.c_array(), nr, ack, win, data))
                            resolve_data_message(nr, ack, win, data);

                        else if (this->client_parser.matches_ack(udp_receive_buffer.c_array(), ack, win))
                            resolve_ack_message(ack, win);
                    }

                    this->udp_receive_buffer.assign(0);
                    udp_listening();
                }
                else
                {
                    std::cerr << "[INFO] BLAD PRZY UZYSKIWANIU UDP " << ec << "\n";
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
            std::cerr << "[INFO] BRAK POLACZENIA\n";
            sock_tcp.close();
            setup_networking();
        }

        run_connection_timer();
    }
    else
    {
        std::cerr << "[ERROR] TIMER HANDLER ERROR " << error << " \n";
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
                            std::cerr << "[ERROR] BLAD PODCZAS WYSYLANIA KEEPALIVE " << ec << " " << bt << "\n";
                    });
        }
        else
        {
            std::cerr << "[ERROR] KEEPALIVE TIMER ERROR " << er << "\n";
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
    nr_global = 0; //ustawienia nie sieciowe co prawda, ale tez potrzebne   
    ack_global = 0;
    win_global = 0;

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
EOF_ERR_NO(2),
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
