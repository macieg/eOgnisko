#include <vector>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include "server.h"

void server::accept_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "[Info] New user - '" << sock_tcp.remote_endpoint() << "' - " << id_sequence << std::endl;

        boost::shared_ptr<connection> conn(new connection(id_sequence, std::move(sock_tcp)));
        std::cerr << "[Info] New user attrs - '" << conn.get()->get_current_bytes_in_fifo() << "' - " << id_sequence << std::endl;
        connections_map_tcp.emplace(id_sequence, conn);
        asio::async_write(conn->get_tcp_socket(),
                asio::buffer(create_accept_response(id_sequence)),
                [this](boost::system::error_code err, std::size_t bt) {
                }
        );
        id_sequence++;
    }
    else
    {
        std::cerr << "[Error] Accept handler failed '" << ec.message() << "'" << std::endl;
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

std::string server::create_raport_part(boost::shared_ptr<connection> conn)
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

size_t server::create_data()
{
    int active_queues_number = 0;
    size_t mixer_output_size = 0;

    for (auto& iter : connections_map_udp)
        active_queues_number += iter.second->is_fifo_active();

    //    std::cerr << "[Info] Active queues number = " << active_queues_number << std::endl;

    if (active_queues_number) //jezeli jakakolwiek kolejka jest aktywna to miksuje
    {
        mixer_inputs.clear();

        for (auto& iter : connections_map_udp) //wypelniam strukture danymi
            if (iter.second->is_fifo_active())
                mixer_inputs.push_back(iter.second->get_mixer_input());

        std::cerr << "[Info] Mixer - received mixer inputs " << std::endl;
        std::cerr << "[Info] Mixer - mixing..." << std::endl;
        mixer_output_size = mixer_output.size();
        mixer(mixer_inputs.data(), mixer_inputs.size(), mixer_output.data(), &mixer_output_size, server_attributes::tx_interval);
        std::cerr << "[Info] Mixer - mixed!" << std::endl;

        int i = 0;
        for (auto& iter : connections_map_udp)
        {
            if (iter.second->is_fifo_active())
            {
                iter.second->set_mixer_intput(mixer_inputs[i++]);
                iter.second->consume_fifo();
            }
        }
    }

    //    std::cerr << "[Info] Mixer created " << mixer_output_size << " bytes" << std::endl;
    return mixer_output_size;
}

void server::mixer_timer_handler(const boost::system::error_code &ec)
{
    mixer_timer_setup();
    size_t mixed_data_size = create_data();

    for (auto& iter : connections_map_udp)
    {
        //        std::cerr << "[Info] Sending data to user '" << iter.first << "'" << std::endl;

        int left_in_fifo = server_attributes::fifo_size - iter.second->get_current_bytes_in_fifo();
        int ACKTODO = 0; //TODO skąd wziąć?
        char* msg = new char[mixed_data_size + 100];
        sprintf(msg, "DATA %d %d %d\n", nr_mixer_global, ACKTODO, left_in_fifo);
        size_t header_size = strlen(msg);
        memmove(msg + header_size, mixer_output.data(), mixed_data_size);

        sock_udp.async_send_to(asio::buffer(msg, header_size + mixed_data_size), iter.first,
                [this, msg](boost::system::error_code err, std::size_t bt) {
                    delete msg;
                });
    }
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

                //                std::cerr << tcp_iter->second->get_tcp_socket().remote_endpoint() << "\n";
                tcp_iter++;
            }
            catch (std::exception& e)
            {
                std::cerr << "[Info] User '" << tcp_iter->first << "' left" << std::endl;
                int client_id = tcp_iter->first;
                tcp_iter++;
                connections_map_tcp.erase(client_id);
            }
        }

        auto udp_iter = std::begin(connections_map_udp);

        while (udp_iter != std::end(connections_map_udp)) //jadę sobie po wszystkich klientach udp
        {
            int client_id = (*udp_iter).second->get_client_id();
            //            std::cerr << "[Info] USER: " << client_id << ", TIME: " << (*udp_iter).second->get_time_from_last_udp() << "\n";

            if (connections_map_tcp.count(client_id) == 0) //jezeli jakiegoś nie ma w mapie tcp (zostal wyrzucony), to tu tez go wyrzucam
                connections_map_udp.erase(udp_iter);
            if ((*udp_iter).second->get_time_from_last_udp() > udp_max_interval) //jezeli ostatni udp był później niż [1000] ms temu
            {
                std::cerr << "[Info] User ' " << tcp_iter->first << "' left" << std::endl;
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
    }
    tcp_timer_setup();
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
    if (connections_map_udp.count(ep_udp) == 0) //Jeżeli nie ma już takiego klienta w mapie to go dodaje
    {
        auto conn = connections_map_tcp[client_id];
        conn->set_udp_endpoint(ep_udp);
        connections_map_udp.emplace(ep_udp, conn);

    }
    else //ktoś jeszcze raz wysłał to samo CLIENT id
    {
        std::cerr << "[Error] Client id duplicated - " << client_id << std::endl;
    }
}

void server::send_ack_udp(int client_id, int nr, int free_bytes_in_fifo)
{
    //    std::cerr << "[Info] send ack client(" << client_id << ") fifo(" << free_bytes_in_fifo << ")" << std::endl;
    auto conn = connections_map_tcp[client_id];

    std::string ack_message;
    ack_message.append("ACK ");
    ack_message.append(std::to_string(nr));
    ack_message.append(" ");
    ack_message.append(std::to_string(free_bytes_in_fifo));
    ack_message.append("\n");

    sock_udp.async_send_to(asio::buffer(std::move(ack_message)), conn->get_udp_endpoint(),
            [this](boost::system::error_code err, std::size_t bt) {
            });
}

void server::resolve_upload_udp(int client_id, int nr, int bytes_transferred, int header_size)
{
    if (bytes_transferred) std::cerr << "[Info] Uploaded from clientId(" << client_id << "), NR(" << nr << ") bytes(" << bytes_transferred << ")" << std::endl;
    auto conn = connections_map_tcp[client_id];

    int data_size = bytes_transferred - header_size;
    if (conn->left_bytes_in_fifo() - data_size >= 0)
    {
        conn->append(udp_buf.data(), header_size, data_size);
        send_ack_udp(client_id, nr + 1, conn->left_bytes_in_fifo());
    }
    else
    {
        throw 1; //moj klient nie powinien wysylac za duzo danych
    }
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
        //        std::cerr << "[Info] Received udp message from '" << ep_udp << "' with (" << bt << ") bytes" << std::endl;

        const char* s = udp_buf.c_array();
        int client_id, nr, begin_point;
        std::string dane;

        if (udp_parser.matches_client_id(s, client_id))
            resolve_client_id_udp(client_id);

        if (connections_map_udp.count(ep_udp))
        {
            client_id = connections_map_udp[ep_udp]->get_client_id();

            if (udp_parser.matches_keepalive(s))
                resolve_keepalive_udp(client_id);

            else if (udp_parser.matches_upload(s, nr, bt, begin_point))
                resolve_upload_udp(client_id, nr, bt, begin_point);

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
mixer_output(MIXER_OUTPUT_BUFFER_SIZE),
udp_parser()
{
}

void server::setup()
{
    tcp_timer_setup();
    mixer_timer_setup();
    network_setup();
}

