
#ifndef CLIENT_H
#define	CLIENT_H
#include <string>
#include <boost/asio.hpp> 
#include <boost/array.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "parser.h"

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

    int max_raport_interval = 1000; //maksymalna tolerancja dla odstepu miedzy raportami
    int connect_interval = 500; //czas dla timera (w milisekundach) do ponownego łączenia
    int keepalive_interval = 100; //odstęp czasu dla timera do wysyłania KEEPALIVE
    const std::string KEEPALIVE;
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
    asio::ip::udp::endpoint ep_udp;
    asio::ip::udp::endpoint server_udp_endpoint;
    boost::asio::streambuf stream_buffer_udp;
    boost::array<char, 1 << 16 > udp_buf;
    boost::array<char, 1 << 16 > udp_receive_buffer;
    asio::deadline_timer keepalive_timer; //timer do wysyłania keepalive
    ////////////////////////////////////////////////////////////////////
    //STDIN STDOUT
    asio::posix::stream_descriptor std_input;
    asio::posix::stream_descriptor std_output;
    boost::array<char, 1 << 16 > stdin_buf;
    /////////////////////////////////////////////////////
    boost::posix_time::ptime last_raport_time; //czas ostatniego raportu

    parser client_parser;
    /**
     * Obsługa zdarzenia odbioru wiadomości TCP.
     * 
     * @param ec error code
     * @param bytes_transferred liczba przesłanych bajtów
     */
    void receive_tcp_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);

    /**
     * Obsługa zdarzenia łączenia z serwerem po TCP.
     * 
     * @param ec error code
     */
    void connect_tcp_handler(const boost::system::error_code &ec);

    /**
     * Obsługa zdarzenia wysłania 
     * @param 
     * @param 
     */
    void send_udp_handler(const boost::system::error_code&, std::size_t);

    /**
     * Obsługa zdarzenia łączenia z serwerem po UDP.
     * 
     * @param ec error code
     */
    void receive_udp_handler(const boost::system::error_code &ec);

    /**
     * Rozpoczyna nasłuchiwanie ze standardowego wejścia.
     */
    void read_std_in();
    
    /**
     * Nasłuchuje na komunikaty udp i odpowiednio nimi zarządza.
     */
    void udp_listening();

    /**
     * Metoda do wykrywania problemów z połączeniem TCP.
     * 
     * @param boost::system::error_code error code
     */
    void connection_timer_handler(const boost::system::error_code&);

    /**
     * Metoda do wywoływania timera wysyłające wiadomości KEEPALIVE.
     */
    void run_keepalive_timer();
    /**
     * Ustawia i uruchamia timer sprawdzający stan połączenia.
     */
    void run_connection_timer();

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

