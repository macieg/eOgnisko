#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include "mixer.h"
#include "server.h"

void server::accept_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        boost::shared_ptr<connection> conn(new connection(id_sequence, std::move(sock_tcp)));

        connections_map_tcp.emplace(id_sequence, conn);

        asio::async_write(conn->get_tcp_socket(),
                asio::buffer(create_accept_response(id_sequence)),
                [this](boost::system::error_code err, std::size_t bt) {
                }
        );

        id_sequence++;
        std::cerr << "[INFO] NOWY KLIENT " << conn->get_tcp_socket().remote_endpoint() << "\n";
    }
    else
    {
        std::cerr << "[ERROR] ACCEPT HANDLER FAILURE " << ec << "\n";
    }

    acceptor_tcp.async_accept(
            sock_tcp,
            boost::bind(&server::accept_handler, this, asio::placeholders::error)
            );
}

std::string server::create_accept_response(int client_id)
{
    std::string response("CLIENT ");
    response.append(boost::lexical_cast<std::string>(client_id));
    response.append("\n");

    return std::move(response);
}

std::string server::create_raport_part(boost::shared_ptr<connection> conn) //TODO nic nie ma tu
{
    std::stringstream raport;
    raport << conn->get_tcp_socket().remote_endpoint() << " FIFO: ";
    raport << conn->get_current_bytes_in_fifo();
    raport << "/";
    raport << server_attributes::fifo_size;
    raport << " (min. ";
    raport << conn->get_min_bytes_in_fifo();
    raport << ", max. ";
    raport << conn->get_max_bytes_int_fifo();
    raport << ")\n";

    return std::move(raport.str());
}

std::string server::create_data_response(boost::shared_ptr<connection> conn) //TODO nic nie ma
{
    std::stringstream resp;
    resp << "DATA";
    resp << " 0"; //nr
    resp << " 0"; //ack
    resp << " " << server_attributes::fifo_size - conn->get_current_bytes_in_fifo();
    resp << "\n";
    resp << "hugo";

    return std::move(resp.str());
}

void server::mixer_timer_handler(const boost::system::error_code &ec)
{
    auto iter = std::begin(connections_map_udp);

    while (iter != std::end(connections_map_udp))
    {
        std::string response = create_data_response(iter->second);
        sock_udp.async_send_to(asio::buffer(std::move(response)), iter->first,
                [this](boost::system::error_code err, std::size_t bt) {
                });
        iter++;
    }

    mixer_timer_setup();
}

void server::tcp_timer_handler(const boost::system::error_code& error)
{
    if (!error)
    {
        std::stringstream message;
        message << "\n"; //Raport powinien być poprzedzony pustą linią.
        auto tcp_iter = std::begin(connections_map_tcp);


        while (tcp_iter != std::end(connections_map_tcp)) //Wyrzucamy z mapy w przypadku zerwania połączenia.
        {
            try
            {
                tcp_iter->second->get_tcp_socket().remote_endpoint();
                message << create_raport_part((*tcp_iter).second);
                tcp_iter->second->clear_history(); //"zapominam" o min/max kolejki z poprzednich raportów

                std::cerr << tcp_iter->second->get_tcp_socket().remote_endpoint() << "\n";
                tcp_iter++;
            }
            catch (std::exception& e)
            {
                std::cerr << "[INFO] STRACILISMY USERA (" << tcp_iter->first << ")\n";
                int client_id = tcp_iter->first;
                tcp_iter++;
                connections_map_tcp.erase(client_id);
            }
            //            tcp_iter++;
        }

        auto udp_iter = std::begin(connections_map_udp);

        while (udp_iter != std::end(connections_map_udp)) //jadę sobie po wszystkich klientach udp
        {
            int client_id = (*udp_iter).second->get_client_id();
            //            std::cerr << "[INFO] USER: " << client_id << ", TIME: " << (*udp_iter).second->get_time_from_last_udp() << "\n";

            if (connections_map_tcp.count(client_id) == 0) //jezeli jakiegoś nie ma w mapie tcp (zostal wyrzucony), to tu tez go wyrzucam
            {
                std::cerr << "[INFO] WYRZUCAM USERA \n";
                connections_map_udp.erase(udp_iter);
            }
            if ((*udp_iter).second->get_time_from_last_udp() > udp_max_interval) //jezeli ostatni udp był później niż [1000] ms temu
            {
                std::cerr << "[INFO] WYRZUCAM USERA\n";
                int conn = (*udp_iter).second->get_client_id();

                connections_map_udp.erase(udp_iter);
                connections_map_tcp.erase(conn);
            }
            udp_iter++;
        }

        for (auto& pair : connections_map_tcp)
        {
            asio::async_write(pair.second->get_tcp_socket(),
                    asio::buffer(std::move(message.str())),
                    [this](boost::system::error_code err, std::size_t bt) {
                    }
            );
        }

        tcp_timer_setup();
    }
}

void server::tcp_timer_setup()
{
    timer_tcp.expires_from_now(boost::posix_time::seconds(raport_timer_interval));
    timer_tcp.async_wait(boost::bind(&server::tcp_timer_handler, this, asio::placeholders::error));
}

void server::mixer_timer_setup()
{
    timer_mixer.expires_from_now(boost::posix_time::milliseconds(server_attributes::tx_interval));
    timer_mixer.async_wait(boost::bind(&server::mixer_timer_handler, this, asio::placeholders::error));
}

void server::resolve_client_id_udp(int client_id)
{
//    std::cerr << "CLIENT ID " << client_id << "\n";

    if (connections_map_udp.count(ep_udp) == 0) //Jeżeli nie ma już takiego klienta w mapie to go dodaje
    {
        auto conn = connections_map_tcp[client_id];
        conn->set_udp_endpoint(ep_udp);
        connections_map_udp.emplace(ep_udp, conn);

    }
    else //ktoś jeszcze raz wysłał to samo CLIENT id
    {
        std::cerr << "[ERROR] CLIENT ID BAD\n";
    }
}

void server::send_ack_udp(int client_id, int nr, int left_bytes_in_fifo)
{
    std::cerr << "[ACK_SEND] (" << client_id << ") fifo(" << left_bytes_in_fifo << ")\n";
    auto conn = connections_map_tcp[client_id];

    std::string ack_message;
    ack_message.append("ACK ");
    ack_message.append(std::to_string(nr));
    ack_message.append(" ");
    ack_message.append(std::to_string(left_bytes_in_fifo));
    ack_message.append("\n");

    sock_udp.async_send_to(asio::buffer(std::move(ack_message)), conn->get_udp_endpoint(),
            [this](boost::system::error_code err, std::size_t bt) {
            });
}

void server::resolve_upload_udp(int client_id, int nr, std::string dane)
{
    std::cerr << "[UPLOAD] CLIENT_ID(" << client_id << "), NR(" << nr << ")\n";
    //    std::cerr << "DANE (" << dane << ")\n";
    auto conn = connections_map_tcp[client_id];
    conn->append(dane);

    send_ack_udp(client_id, nr + 1, server_attributes::fifo_size - conn->get_current_bytes_in_fifo());
}

void server::resolve_retransmit_udp(int client_id, int nr)
{
    std::cerr << "[RETRANSMIT]\n";
    //TODO
}

void server::resolve_keepalive_udp(int client_id)
{
    //    std::cerr << "[KEEPALIVE]\n";
    //TODO nic :)
}

void server::udp_receive_handler(const boost::system::error_code& error, std::size_t bt)
{
    if (!error)
    {
        //        std::cerr << ep_udp << " UDP (" << bt << ")\n";
        //        std::cerr << "[ " << udp_buf.data() << " ]\n";

        char* s = udp_buf.c_array();
        int client_id, nr;
        std::string dane;

        if (udp_parser.matches_client_id(s, client_id))
            resolve_client_id_udp(client_id);

        if (connections_map_udp.count(ep_udp))
        {
            client_id = connections_map_udp[ep_udp]->get_client_id();
            if (udp_parser.matches_keepalive(s))
                resolve_keepalive_udp(client_id);

            else if (udp_parser.matches_upload(s, nr, dane))
                resolve_upload_udp(client_id, nr, std::move(dane));

            else if (udp_parser.matches_retransmit(s, nr))
                resolve_retransmit_udp(client_id, nr);

            connections_map_tcp[client_id]->update_udp_time();
        }

        udp_buf.assign(0); //czyścimy bufor
        sock_udp.async_receive_from(asio::buffer(udp_buf), ep_udp,
                boost::bind(&server::udp_receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
    }
    else
    {
        std::cerr << "[ERROR] UDP RECEIVE HANDLER FAIL\n";
    }
}

void server::network_setup()
{
    acceptor_tcp.set_option(asio::socket_base::reuse_address(true)); //Trick, zapobiega blokowaniu się gniazda.
    acceptor_tcp.listen();
    acceptor_tcp.async_accept(sock_tcp,
            boost::bind(&server::accept_handler, this, asio::placeholders::error)
            );

    sock_udp.async_receive_from(asio::buffer(udp_buf), ep_udp,
            boost::bind(&server::udp_receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
}

server::server(asio::io_service& io_service) :
endpoint_tcp(asio::ip::tcp::v4(), server_attributes::port),
acceptor_tcp(io_service, endpoint_tcp),
sock_tcp(io_service),
timer_tcp(io_service),
endpoint_udp(asio::ip::udp::v4(), server_attributes::port),
sock_udp(io_service, endpoint_udp),
timer_mixer(io_service),
udp_parser()
{
}

void server::setup()
{
    tcp_timer_setup();
    mixer_timer_setup();
    network_setup();
}

