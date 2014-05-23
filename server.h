
#ifndef SERVER_H
#define	SERVER_H
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
#include <map>
#include "connection.h"
#include "parser.h"

namespace asio = boost::asio;

class server {
private:
    /**
     * Stałe z treści zadania.
     */
    int port;
    int fifo_size;
    int fifo_low_watermark;
    int fifo_high_watermark;
    int buf_len;
    int tx_interval;

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
    ///////////////////////////////////////////////////////////////////

    int id_sequence = 1; //Pomocnicza sekwencja do tworzenia id klientów.

    int TIMER_INTERVAL = 1; //czas dla timera (w sekundach) do rozsyłania raportów
    
    int udp_max_interval = 1000; //Tolerancja odstępu czasu między datagramami otrzymanymi od danego klienta.

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
     * @param ec error code
     */
    void accept_handler(const boost::system::error_code &ec);

    /**
     * Metoda budzona co TIMER_INTERVAL, służąca do rozsyłania raportów do klientów.
     * 
     * @param error error code
     * @param timer obiekt timera
     */
    void timer_handler(const boost::system::error_code& error);

    /**
     * Tworzy wiadomość jako odpowiedź dla klienta, któremu udało się połączyć
     * z informacją o jego id.
     * 
     * "CLIENT clientid\n"
     * 
     * @param client_id
     * @return string z potwierdzeniem
     */
    std::string create_accept_response(int client_id);

    /**
     * Buduje fragment raportu z informacjami o połączeniu.(nizej wyglad)
     * 127.0.0.1:50546 FIFO: 6040/7040 (min. 824, max. 7040)\n
     * 
     * @param shared pointer na connection
     * @return fragment raportu
     */
    std::string create_raport_part(boost::shared_ptr<connection>);

    /**
     * Obsługuje wiadomość typu CLIENT_ID
     * 
     * @param client_id
     */
    void resolve_client_id_udp(int);
    
    void udp_receive_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

    /**
     * Ustawia timer do wysyłania raportów.
     */
    void timer_setup();

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
     * @param nr portu
     */
    server(asio::io_service&, int);

    /**
     * Metoda służąca do ustawienia startowych opcji w programie.
     * 
     * @params parametry z treści
     */
    void setup(int fifo_size,
            int fifo_low_watermark,
            int fifo_high_watermark,
            int buf_len,
            int tx_interval);

};

#endif	/* SERVER_H */

