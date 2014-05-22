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
        boost::shared_ptr<connection> conn(new connection(id_sequence, std::move(sock)));

        connections_map.emplace(conn.get()->get_socket().remote_endpoint(), conn);

        asio::async_write(conn->get_socket(),
                asio::buffer(create_accept_response(id_sequence)),
                [this](boost::system::error_code err, std::size_t bt) {
                }
        );

        id_sequence++;
        std::cerr << "NOWY KLIENT " << conn->get_socket().remote_endpoint() << "\n";
    }
    else
    {
        std::cerr << "ACCEPT HANDLER FAILURE " << ec << "\n";
    }


    acceptor.async_accept(
            sock,
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
    raport << conn->get_socket().remote_endpoint() << " FIFO: ";
    raport << "00"; //TODO ile dostepnego IFO
    raport << "/";
    raport << fifo_size;
    raport << " (min. ";
    raport << "000"; //TODO minimum od ostatniego raportu
    raport << ", max. ";
    raport << "999"; //TODO minimum od ostatniego raportu
    raport << ")\n";

    return raport.str();
}

void server::timer_handler(const boost::system::error_code& error)
{
    if (!error)
    {
        std::stringstream message;
        message << "\n"; //Raport powinien być poprzedzony pustą linią.
        auto pair = std::begin(connections_map);

        while (pair != std::end(connections_map)) //Wyrzucamy z mapy w przypadku zerwania połączenia.
        {
            try
            {
                (*pair).second->get_socket().remote_endpoint();
                message << create_raport_part((*pair).second);

                std::cerr << (*pair).second->get_socket().remote_endpoint() << "\n";
            }
            catch (std::exception& e)
            {
                std::cerr << "STRACILISMY USERA\n";
                connections_map.erase(pair);
            }
            pair++;
        }

        //        std::cerr << message.str();

        for (auto& pair : connections_map)
        {
            asio::async_write(pair.second->get_socket(),
                    asio::buffer(std::move(message.str())),
                    [this](boost::system::error_code err, std::size_t bt) {
                    }
            );
        }

        timer_setup();
    }
}

void server::timer_setup()
{
    timer.expires_from_now(boost::posix_time::seconds(TIMER_INTERVAL));
    timer.async_wait(boost::bind(&server::timer_handler, this, asio::placeholders::error));
}

void server::network_setup()
{
    acceptor.set_option(asio::socket_base::reuse_address(true)); //Trick, zapobiega blokowaniu się gniazda.
    acceptor.listen();
    acceptor.async_accept(sock,
            boost::bind(&server::accept_handler, this, asio::placeholders::error)
            );
}

server::server(asio::io_service& io_service, int port) : port(port), endpoint(asio::ip::tcp::v4(), port),
acceptor(io_service, endpoint),
sock(io_service),
timer(io_service)
{
}

void server::setup(int fifo_size,
        int fifo_low_watermark,
        int fifo_high_watermark,
        int buf_len,
        int tx_interval)
{
    timer_setup();
    network_setup();
    
    this->fifo_size = fifo_size;
    this->fifo_low_watermark = fifo_low_watermark;
    this->fifo_high_watermark = fifo_high_watermark;
    this->buf_len = buf_len;
    this->tx_interval = tx_interval;
}

