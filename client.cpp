#include <boost/bind.hpp>
#include "client.h"
#include "server.h"
//#define DEBUG

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
                            #ifdef DEBUG
                            std::cerr << "[Info] ClientId successfully sended\n";
                            #endif

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
        sock_tcp.set_option((asio::ip::tcp::no_delay(true)));
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

        last_sended_msg = upload_msg;
        sock_udp.async_send_to(asio::buffer(upload_msg),
                server_udp_endpoint,
                [this](boost::system::error_code ec, std::size_t bt) {
                    if (!ec)
                    {
                        #ifdef DEBUG
                        std::cerr << "[Info] Upload message successfully sended with bt - (" << bt << ")\n";
                        #endif
                    }
                    else
                        std::cerr << "[Error] While sending upload message ec - '" << ec.message() << "', bt - " << bt << std::endl;
                });
    }
    ++my_nr_global; //zwiększam licznik datagramow
}

void client::resolve_data_message(int nr, int ack, int win, int header_size, int bytes_transferred)
{
    #ifdef DEBUG
    std::cerr << "[Info] Received DATA nr(" << nr << ") ack(" << ack << ")win (" << win << ")" << std::endl;
    std::cerr << "[Info] Additional info - server nr global - " << server_nr_global << std::endl;
    #endif

    int nr_expected = server_nr_global + 1;
    if (nr_expected <= nr) //jeżeli otrzymany datagram nie mniejszy niz oczekiwany to moge kontynuować
    {
        if (nr_expected >= nr - retransmit_limit && nr_expected != nr)
        {

            char msg[RETRANSMIT_MAX];
            sprintf(msg, "RETRANSMIT %d\n", nr_expected);
            if (nr > nr_max_seen)
            {
                sock_udp.async_send_to(asio::buffer(msg, strlen(msg)),
                        server_udp_endpoint,
                        [this, nr_expected](boost::system::error_code ec, std::size_t bt) {
                            if (ec)
                                std::cerr << "[Error] While sending upload message ec - '" << ec.message() << "', bt - " << bt << std::endl;
                            #ifdef DEBUG
                            else
                                std::cerr << "[Info] Successfully sent retransmit request with expected_nr = " << nr_expected << std::endl;
                            #endif
                        });
            }
        }
        else
        {


            ++data_counter;
            if (data_counter > 1)
            {
                sock_udp.async_send_to(asio::buffer(last_sended_msg),
                        server_udp_endpoint,
                        [this](boost::system::error_code ec, std::size_t bt) {
                            if (ec)
                                std::cerr << "[Error] While resending message '" << ec.message() << "', bt - " << bt << std::endl;
                            #ifdef DEBUG
                            else
                                std::cerr << "[Info] Successfully resent packet" << std::endl;
                            #endif

                        });
            }

            server_nr_global = nr;
            win_global = win;

            if (ack == my_nr_global)
            {
                #ifdef DEBUG
                std::cerr << "[Info] Before read from input, win = " << win_global << std::endl;
                #endif
                
                if (!is_reading)
                {
                    is_reading = true;
                    asio::async_read(std_input, asio::buffer(stdin_buf, std::min(win_global, 1 << 16)),
                            [this, win] (const boost::system::error_code& ec, std::size_t bt) {
                                if (!ec || ec.value() == EOF_ERR_NO)
                                {
                                    #ifdef DEBUG
                                    std::cerr << "[Info] Read (after data) from input bt(" << bt << ") win(" << win << ")" << std::endl;
                                    #endif
                                    is_reading = false;
                                    send_upload_message(bt);
                                }
                                else
                                    std::cerr << "[Info] Problem with reading from stdin, ec = '" << ec.message() << "'" << std::endl;
                            }
                    );
                }
            }

            const char* data = udp_receive_buffer.data();
            int bytes_to_transfer = bytes_transferred - header_size;

            asio::write(std_output, asio::buffer(data + header_size, bytes_to_transfer));

            nr_max_seen = std::max(nr, nr_max_seen);
        }
    }
}

void client::resolve_ack_message(int ack, int win)
{
    #ifdef DEBUG
    std::cerr << "[Info] received ACK ack(" << ack << ") win(" << win << ")" << std::endl;
    #endif

    win_global = win;

    data_counter = 0;

    if (win) //jezeli jest sens cokolwiek wysyłać
    {
        if (ack == my_nr_global)
        {
            if (!is_reading)
            {
                is_reading = true;
                asio::async_read(std_input, asio::buffer(stdin_buf, win_global),
                        [this, win] (const boost::system::error_code& ec, std::size_t bt) {
                            if (!ec || ec.value() == EOF_ERR_NO)
                            {
                                #ifdef DEBUG
                                std::cerr << "[Info] Read (after ack) from input bt(" << bt << ") win(" << win << ")" << std::endl;
                                #endif
                                
                                is_reading = false;
                                send_upload_message(bt);
                            }
                            else
                                std::cerr << "[Info] Problem with reading from stdin, ec = '" << ec.message() << "'" << std::endl;
                        }
                );
            }
        }
    }
}

void client::udp_listening()
{
    sock_udp.async_receive_from(asio::buffer(udp_receive_buffer), ep_udp,
            [this](const boost::system::error_code& ec, std::size_t bt) {
                if (!ec)
                {
                    if (!was_first_udp)
                    {
                        was_first_udp = true;
                        server_udp_endpoint = this->ep_udp;
                    }
                    if (this->ep_udp == this->server_udp_endpoint) //czy na pewno dostaje udp od dobrego serwera
                    {
                        #ifdef DEBUG
                        std::cerr << "[Info] Received datagram from server - (" << bt << ") bytes" << std::endl;
                        #endif
                        
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
    my_nr_global = 0; //ustawienia nie sieciowe co prawda, ale tez potrzebne   
    win_global = 0;

    asio::ip::tcp::resolver::query query_tcp(server, port,
            asio::ip::resolver_query_base::flags());
    asio::ip::tcp::resolver::iterator it_tcp = resolver_tcp.resolve(query_tcp);
    sock_tcp.async_connect(*it_tcp, boost::bind(&client::connect_tcp_handler, this, asio::placeholders::error));

    asio::ip::udp::resolver::query query_udp(server,
            boost::lexical_cast<std::string>(port),
            asio::ip::resolver_query_base::flags());
    asio::ip::udp::resolver::iterator it_udp = resolver_udp.resolve(query_udp);
    server_udp_endpoint = *it_udp;
}

client::client(asio::io_service& io_service) :
KEEPALIVE("KEEPALIVE\n"),
resolver_tcp(io_service),
sock_tcp(io_service),
connect_timer(io_service),
resolver_udp(io_service),
sock_udp(io_service, asio::ip::udp::endpoint(asio::ip::udp::v6(), 0)),
keepalive_timer(io_service),
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
