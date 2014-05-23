
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

    int connect_interval = 500; //czas dla timera (w milisekundach) do ponownego łączenia
    int keepalive_interval = 100; //odstęp czasu dla timera do wysyłania KEEPALIVE
    /////////////////////////////////////////////////////////////////
    //TCP
    asio::ip::tcp::resolver resolver_tcp;
    asio::ip::tcp::socket sock_tcp;
    //    boost::array<char, 4096> buffer;
    boost::asio::streambuf stream_buffer_tcp;
    asio::deadline_timer connect_timer; //timer sprawdzający co jakiś czas stan połączenie tcp
    ///////////////////////////////////////////////////////////////
    //UDP
    asio::ip::udp::resolver resolver_udp;
    asio::ip::udp::socket sock_udp;
    boost::asio::streambuf stream_buffer_udp;
    boost::array<char, 1 << 16 > udp_buf;
    asio::deadline_timer keepalive_timer; //timer do wysyłania keepalive
    ////////////////////////////////////////////////////////////////////


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
    void connect_handler_tcp(const boost::system::error_code &ec);

    void send_udp_handler(const boost::system::error_code&, std::size_t);
    /**
     * Obsługa zdarzenia łączenia z serwerem po UDP.
     * 
     * @param ec error code
     */
    void connect_handler_udp(const boost::system::error_code &ec);

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

