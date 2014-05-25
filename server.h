
#ifndef SERVER_H
#define	SERVER_H
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <map>
#include "connection.h"
#include "parser.h"
#include "mixer.h"

namespace asio = boost::asio;

class server {
private:
    ///////////////////////////////////////////////////////////////////
    ///TCP
    asio::ip::tcp::endpoint endpoint_tcp;
    asio::ip::tcp::acceptor acceptor_tcp;
    asio::ip::tcp::socket sock_tcp;
    asio::deadline_timer timer_tcp;
    ///UDP
    asio::ip::udp::endpoint endpoint_udp;
    asio::ip::udp::socket sock_udp;
    boost::asio::streambuf stream_buffer;
    boost::array<char, 1 << 16 > udp_buf;
    asio::ip::udp::endpoint ep_udp;
    asio::deadline_timer timer_mixer;
    //////////////////////////////////////////////////////////////////

    static const int raport_timer_interval = 1; //czas dla timera (w sekundach) do rozsyłania raportów
    static const int udp_max_interval = 10000; //Tolerancja odstępu czasu między datagramami otrzymanymi od danego klienta. //TODO zmienic na 1000
    const float magic_number = 176.4f; // magiczna liczba do tworzenia rozmiaru bufora z miksera
    static const int MIXER_OUTPUT_BUFFER_SIZE = (1<<16) + 10;

    int id_sequence = 1; //Pomocnicza sekwencja do tworzenia id klientów.
    int nr_mixer_global = 0; //Numer ostatnio wygenerowanego datagramu przez mixer.

    std::vector <mixer_input> mixer_inputs; //dane do miksera

    std::vector<char> mixer_output; //dane wyjsciowe z miksera

    parser udp_parser;
    /**
     * Mapuje ID klienta przyznane przez serwer na połączenie z nim związane
     */
    std::map<int, boost::shared_ptr<connection> > connections_map_tcp;

    /**
     * Mapuje endpoint udp klienta na połączenie z nim związane
     */
    std::map<asio::ip::udp::endpoint, boost::shared_ptr<connection> > connections_map_udp;
    /**
     * Metoda wywoływana, io_service otrzyma do obsługi połączenie.
     * 
     * @param error code
     */
    void accept_handler(const boost::system::error_code&);

    /**
     * Metoda budzona co TX interval do wysyłania danych wyjściowych.
     * 
     * @param error code
     */
    void mixer_timer_handler(const boost::system::error_code&);

    /**
     * Metoda budzona co TIMER_INTERVAL, służąca do rozsyłania raportów do klientów.
     * 
     * @param error error code
     * @param timer obiekt timera
     */
    void tcp_timer_handler(const boost::system::error_code& error);

    /**
     * Tworzy wiadomość jako odpowiedź dla klienta, któremu udało się połączyć
     * z informacją o jego id.
     * 
     * "CLIENT clientid\n"
     * 
     * @param client_id
     * @return string z potwierdzeniem
     */
    std::string create_accept_response(int);

    /**
     * Buduje fragment raportu z informacjami o połączeniu.(nizej wyglad)
     * 127.0.0.1:50546 FIFO: 6040/7040 (min. 824, max. 7040)\n
     * 
     * @param shared pointer na connection
     * @return fragment raportu
     */
    std::string create_raport_part(boost::shared_ptr<connection>);

    /**
     * Buduje odpowiedź ze zmiksowanymi danymi.
     * 
     * @return rozmiar zmiksowanych danych
     */
    size_t create_data();

    /**
     * Obsługuje wiadomość typu CLIENT_ID.
     * 
     * @param client_id
     */
    void resolve_client_id_udp(int);

    /**
     * Wysyła wiadomość potwierdzającą odebranie danych.
     * 
     * @param id klienta
     * @param nr
     * @param pozostala liczba miejsc (bajtow) w fifo
     */
    void send_ack_udp(int, int, int);

    /**
     * Obsługuje wiadomość typu UPLOAD.
     * 
     * @param client_id
     * @param nr
     * @param bytes transferred
     * @param header size
     */
    void resolve_upload_udp(int, int, int, int);

    /**
     * Obsługuje wiadomość typu RETRANSMIT.
     * 
     * @param client_id
     * @param nr
     */
    void resolve_retransmit_udp(int, int);

    /**
     * Obsługuje wiadomość typu KEEPALIVE.
     * 
     * @param client_id
     */
    void resolve_keepalive_udp(int);

    /**
     * Obsługuje otrzymanie wiadomości UDP. 
     * 
     * @param error
     * @param bytes_transferred
     */
    void udp_receive_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * Ustawia timer do wysyłania raportów.
     */
    void tcp_timer_setup();

    /**
     * Ustawia timer do wysyłania zmiksowanych danych.
     */
    void mixer_timer_setup();

    /**
     * Ustawia zmienne dotyczące sieci.
     */
    void network_setup();

public:

    /**
     * Konstruktor przyjmujący referencje na io_service i nr portu,
     * na którym będzie nasłuchiwał TCP i UDP.
     * 
     * @param io_service
     */
    server(asio::io_service&);

    /**
     * Metoda służąca do ustawienia startowych opcji w programie.
     * 
     */
    void setup();

};

#endif	/* SERVER_H */

