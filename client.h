
#ifndef CLIENT_H
#define	CLIENT_H
#include <string>
#include <boost/asio.hpp> 
#include <boost/array.hpp> 

namespace asio = boost::asio;

/**
 * Klasa reprezentująca klienta.
 */
class client {
private:
    /**
     * Parametry z treści.
     */
    int retransmit_limit;
    std::string port;
    std::string server;

    int TIMER_INTERVAL = 500; //czas dla timera (w milisekundach) do ponownego łączenia

    /////////////////////////////////////////////////////////////////
    asio::ip::tcp::resolver resolver;
    asio::ip::tcp::socket sock;
    boost::array<char, 4096> buffer;
    boost::asio::streambuf stream_buffer;
    ///////////////////////////////////////////////////////////////


    asio::deadline_timer connect_timer; //timer sprawdzający co jakiś czas stan połączenie tcp

    /**
     * Obsługa zdarzenia odbioru wiadomości TCP.
     * 
     * @param ec error code
     * @param bytes_transferred liczba przesłanych bajtów
     */
    void receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);

    /**
     * Obsługa zdarzenia łączenia z serwerem po TCP.
     * 
     * @param ec error code
     */
    void connect_handler(const boost::system::error_code &ec);

    /**
     * Metoda do wykrywania problemów z połączeniem TCP.
     * 
     * @param boost::system::error_code error code
     */
    void timer_handler(const boost::system::error_code&);

    /**
     * Ustawienia handlera.
     */
    void setup_timer();

    /**
     * Ustawienia sieci.
     */
    void setup_networking();

public:

    client(asio::io_service&);

    /**
     * Metoda służąca do wykonania ustawień.
     * 
     * @param retransmit_limit z treści
     * @param port z treści
     * @param server z treści
     */
    void setup(int retransmit_limit, std::string port, std::string server);
};

#endif	/* CLIENT_H */

