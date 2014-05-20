
#ifndef CLIENT_H
#define	CLIENT_H
#include <string>
#include <boost/program_options.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 

namespace po = boost::program_options;
namespace asio = boost::asio;

class client
{
private:
    //Odgórny ogranicznik na numer datagramu otrzymanego od serwera.
    int retransmit_limit = 10;

    //Nr portu serwera.
    std::string port = "3856";

    //Nazwa serwera.
    std::string server;
    asio::io_service io_service;
    //asio::io_service::work work(io_service);
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket sock;
    boost::array<char, 4096> buffer;

    void receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
    
    void write_handler(const boost::system::error_code& ec, std::size_t bytes_transferred);

    void connect_handler(const boost::system::error_code &ec);

    void resolve_handler(const boost::system::error_code &ec, asio::ip::tcp::resolver::iterator it);

    //Metoda obsługująca przechwycenie sygnału SIG_INT.

    void sigint_handler(const boost::system::error_code& error, int signal_number);

    /**
     * Ustawienia dotyczące wyłapywania sygnałów.
     */
    void setup_signals();

    bool program_options_setup(int argc, char **argv);

    /**
     * Ustawienia sieci.
     */
    void setup_networking();

public:

    client() : resolver(io_service), sock(io_service)
    {
    }

    /**
     * Metoda służąca do wykonania ustawień.
     * 
     * @param argc licznik argumentów
     * @param argv tablica argumentów
     * @return true jeżeli napotkano błąd, false wpp
     */
    void setup(int argc, char **argv);
};

#endif	/* CLIENT_H */

