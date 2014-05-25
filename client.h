
#ifndef CLIENT_H
#define	CLIENT_H
#include <string>
#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/array.hpp>
#include <chrono>
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
    static const char NEWLINE_SIGN = '\n';
    static const int EOF_ERR_NO = 2; //boostowy error number

    //wartości do pamiętania
    int my_nr_global; //numer wczytanego datagramu
    int server_nr_global; //number wczytanego datagramu serwera
    int win_global; //liczba wolnych bajtów w kolejce

    /////////////////////////////////////////////////////////////////
    //TCP
    asio::ip::tcp::resolver resolver_tcp;
    asio::ip::tcp::socket sock_tcp;
    //    boost::array<char, 4096> buffer;
    boost::asio::streambuf stream_buffer_tcp;
    asio::steady_timer connect_timer; //timer sprawdzający co jakiś czas stan połączenie tcp
    ///////////////////////////////////////////////////////////////
    //UDP
    asio::ip::udp::resolver resolver_udp;
    asio::ip::udp::socket sock_udp;
    asio::ip::udp::endpoint ep_udp;
    asio::ip::udp::endpoint server_udp_endpoint;
    boost::asio::streambuf stream_buffer_udp;
    boost::array<char, 1 << 16 > udp_receive_buffer;
    asio::steady_timer keepalive_timer; //timer do wysyłania keepalive
    ////////////////////////////////////////////////////////////////////
    //STDIN STDOUT
    asio::posix::stream_descriptor std_input;
    asio::posix::stream_descriptor std_output;
    boost::array<char, 1 << 16 > stdin_buf;
    ;
    /////////////////////////////////////////////////////
    std::chrono::system_clock::time_point last_server_msg; //czas ostatniej wiadomości od serwera
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
     * Obsługa zdarzenia łączenia z serwerem po UDP.
     * 
     * @param ec error code
     */
    void receive_udp_handler(const boost::system::error_code &ec);

    /**
     * Tworzy i wysyła wiadomość UPLOAD do serwera.
     * 
     * @param liczba danych otrzymanych z wejścia w bajtach
     */
    void send_upload_message(std::size_t);

    /**
     * Obsługa zdarzenia otrzymania wiadomości DATA od serwera.
     * 
     * @param nr
     * @param ack
     * @param win
     * @param długość nagłówka
     * @param liczba przesłanych bajtów
     */
    void resolve_data_message(int, int, int, int, int);

    /**
     * Obsługa zdarzenia otrzymania wiadomości ACK od serwera.
     * 
     * @param ack
     * @param win
     */
    void resolve_ack_message(int, int);

    /**
     * Nasłuchuje na komunikaty udp i odpowiednio nimi zarządza.
     */
    void udp_listening();

    /**
     * Metoda do wywoływania timera wysyłające wiadomości KEEPALIVE.
     */
    void run_keepalive_timer();
    /**
     * Ustawia i uruchamia timer sprawdzający stan połączenia.
     */
    void run_connection_timer();
    
    /**
     * Uaktualnia czas otrzymania ostatniej wiadomości od serwera.
     */
    void update_last_server_msg();

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

    class connection_exception : public std::exception {

        const char* what() const noexcept {
            return "[Error] Connection failed! Trying to reconnect...";
        }
    };
};

#endif	/* CLIENT_H */

