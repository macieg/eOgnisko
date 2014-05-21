#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/program_options.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/timer.hpp>
#include "err.h"
#include "mixer.h"
#include "server.h"

namespace po = boost::program_options;

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

void server::sigint_handler(const boost::system::error_code& error, int signal_number)
{
    if (!error)
    {
        std::cerr << "Otrzymano sygnał " << signal_number << "konczenie pracy\n";
        io_service.stop();
    }
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

void server::signals_setup()
{
    asio::signal_set signals(io_service, SIGINT);
    signals.async_wait(
            boost::bind(&server::sigint_handler, this,
            asio::placeholders::error, asio::placeholders::signal_number)
            );
}

void server::program_options_setup(int argc, char** argv)
{
    po::options_description description("Server usage");

    description.add_options()
            ("help,h", "Distplay this help message")
            ("port,p", po::value<int>(&port), "Port number")
            ("fifosize,F", po::value<int>(&fifo_size), "Fifo size")
            ("fifolowwatermark,L", po::value<int>(&fifo_low_watermark), "Fifo low watermark")
            ("fifohighwatermark,H", po::value<int>(&fifo_high_watermark), "Fifo high watermark")
            ("buflen,X", po::value<int>(&buf_len), "Buffer length")
            ("txinterval,i", po::value<int>(&tx_interval), "Tx interval");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
        std::cerr << description;
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
    io_service.run();
}

server::server() : endpoint(asio::ip::tcp::v4(), port),
acceptor(io_service, endpoint),
sock(io_service),
timer(io_service)
{
}

void server::setup(int argc, char** argv)
{
    program_options_setup(argc, argv);
    signals_setup();
    timer_setup();
    network_setup();
}

