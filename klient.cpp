#include <iostream>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <string>
#include <boost/program_options.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 
#include "err.h"
#include "mixer.h"
namespace po = boost::program_options;
namespace asio = boost::asio;

//Odgórny ogranicznik na numer datagramu otrzymanego od serwera.
int retransmit_limit = 10;

//Nr portu serwera.
std::string port;

//Nazwa serwera.
std::string server;

//Zmienna pomocnicza do obsługi sygnałów.
struct sigaction sigint_action;

//Informacja o tym, czy złapano sygnał SIG_INT.
bool is_stopped;

asio::io_service io_service;
asio::io_service::work work(io_service);
asio::ip::tcp::resolver resolver(io_service);
asio::ip::tcp::socket sock(io_service);
boost::array<char, 4096> buffer;

//Metoda obsługująca przechwycenie sygnału SIG_INT.

void sigint_handler(int i)
{
    is_stopped = true;
    fprintf(stderr, "Przechwycono SIG_INT %d\n", i);
    io_service.stop();
}

//Metoda służąca do ustawienia startowych opcji w programie.
void setup(int argc, char **argv)
{
    //Przyspieszenie iostream.
    std::ios_base::sync_with_stdio(false);

    //Ustawienia dotyczące wyłapywania sygnałów.
    is_stopped = false;

    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sigint_action, 0) == -1)
        syserr("Error in signal\n");

    //Parsowanie argumentów programu.
    po::options_description description("Client usage");

    description.add_options()
            ("help,h", "Display this help message")
            ("server,s", po::value<std::string>(), "Server name")
            ("port,p", po::value<std::string>(), "Port number")
            ("retransmit,X", po::value<int>(), "Retransmit limit");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
    {
        std::cerr << description;
    }

    if (vm.count("server"))
    {
        server = vm["server"].as<std::string>();
        std::cerr << "Server name - " << server << std::endl;
    }

    if (vm.count("port"))
    {
        port = vm["port"].as<std::string>();
        std::cerr << "Port number - " << port << std::endl;
    }

    if (vm.count("retransmit"))
    {
        retransmit_limit = vm["retransmit"].as<int>();
        fprintf(stderr, "Retransmit limit - %d\n", retransmit_limit);
    }
}

void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        std::cerr << "READ SUCCESS\n";
        std::cerr << "ODEBRALEM " << std::string(buffer.data(), bytes_transferred);
    }
}

void write_handler(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    std::cerr << "WRITE HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        std::cerr << "WRITE SUCCESS\n";
        sock.async_read_some(asio::buffer(buffer), read_handler);
    }
    else
    {
        std::cerr << "WRITE FAIL\n";
    }
}

void connect_handler(const boost::system::error_code &ec)
{
    std::cerr << "CONNECT HANDLER \n";
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS \n";
        //        boost::asio::write(sock, boost::asio::buffer("sdfsafsafsa"));
        //        sock.write_some(boost::asio::buffer("aaaa"));
        sock.async_write_some(asio::buffer("prosze o polaczenie"), write_handler);
    }
    else
    {
        std::cerr << "CONNECT FAIL\n";
        //        sock.async_connect(connect_handler);
        //TODO probuj do skutku
    }
}

void resolve_handler(const boost::system::error_code &ec, boost::asio::ip::tcp::resolver::iterator it)
{
    if (!ec)
    {
        sock.async_connect(*it, connect_handler);
    }
}

int main(int argc, char** argv)
{
    setup(argc, argv);

    asio::ip::tcp::resolver::query query(server, port);
    resolver.async_resolve(query, resolve_handler);
    io_service.run();
}
