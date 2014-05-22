
#ifndef CLIENT_H
#define	CLIENT_H
#include <string>
#include <boost/program_options.hpp>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 

namespace po = boost::program_options;
namespace asio = boost::asio;
/**
 * Klasa reprezentująca klienta.
 */
class client
{
private:
    //Odgórny ogranicznik na numer datagramu otrzymanego od serwera.
    int retransmit_limit = 10;

    //Nr portu serwera.
    std::string port = "3856";

    //Nazwa serwera.
    std::string server;
    
    int TIMER_INTERVAL = 0.5; //czas dla timera (w sekundach) do ponownego łączenia
    
    /////////////////////////////////////////////////////////////////
    asio::io_service io_service;
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket sock;
    boost::array<char, 4096> buffer;
    boost::asio::streambuf s;
    ///////////////////////////////////////////////////////////////
    
    
    asio::deadline_timer connect_timer; //

    void receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);
    
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

    client();

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

