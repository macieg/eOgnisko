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
std::string port = "3856";

//Nazwa serwera.
std::string server;

//Zmienna pomocnicza do obsługi sygnałów.
struct sigaction sigint_action;

//Informacja o tym, czy złapano sygnał SIG_INT.
bool is_stopped;

asio::io_service io_service;
//asio::io_service::work work(io_service);
asio::ip::tcp::resolver resolver(io_service);
asio::ip::tcp::socket sock(io_service);
boost::array<char, 4096> buffer;

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

//Metoda obsługująca przechwycenie sygnału SIG_INT.

void sigint_handler(int i)
{
    is_stopped = true;
    fprintf(stderr, "Przechwycono SIG_INT %d\n", i);
    io_service.stop();
}

/**
 * Ustawienia dotyczące wyłapywania sygnałów.
 */
void setup_signals()
{
    is_stopped = false;

    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sigint_action, 0) == -1)
        syserr("Error in signal\n");
}

bool program_options_setup(int argc, char **argv)
{
    //Parsowanie argumentów programu.
    po::options_description description("Client usage");
    po::variables_map vm;

    try
    {
        description.add_options()
                ("help,h", "Display this help message")
                ("server,s", po::value<std::string>(&server)->required(), "Server name")
                ("port,p", po::value<std::string>(&port), "Port number")
                ("retransmit,X", po::value<int>(&retransmit_limit), "Retransmit limit");


        po::store(po::command_line_parser(argc, argv).options(description).run(), vm);

        if (vm.count("help"))
            std::cerr << description;

        po::notify(vm);
    }
    catch (std::exception& e)
    {
        if (!vm.count("help"))
            std::cerr << description;
        return true;
    }

    return false;
}
/**
 * Metoda służąca do wykonania ustawień.
 * 
 * @param argc licznik argumentów
 * @param argv tablica argumentów
 * @return true jeżeli napotkano błąd, false wpp
 */
bool setup(int argc, char **argv)
{
    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.
    if (program_options_setup(argc, argv))
        return true;
    setup_signals();
    return false;
}

int main(int argc, char** argv)
{
    if (setup(argc, argv))
        return 1;

    asio::ip::tcp::resolver::query query(server, port);
    resolver.async_resolve(query, resolve_handler);
    io_service.run();
}
