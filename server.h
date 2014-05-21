
#ifndef SERVER_H
#define	SERVER_H
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <map>
#include "connection.h"

namespace asio = boost::asio;

class server {
private:
    /**
     * Stałe z treści zadania.
     */
    int port = 3856;
    int fifo_size = 10560;
    int fifo_low_watermark = 0;
    int fifo_high_watermark = fifo_size;
    int buf_len = 10;
    int tx_interval = 5;
    int TIMER_INTERVAL = 1; //czas dla timera (w sekundach) do rozsyłania raportów

    ///////////////////////////////////////////////////////////////////
    asio::io_service io_service;
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket sock;
    boost::array<char, 4096> buffer;

    asio::deadline_timer timer;
    ///////////////////////////////////////////////////////////////////


    //Pomocnicza sekwencja do tworzenia id klientów.
    int id_sequence = 1;

    /**
     * Struktura danych przechowująca userów.
     * //TODO być może do zmiany
     */
    std::map<asio::ip::tcp::endpoint, boost::shared_ptr<connection> > connections_map;

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
     * Metoda wywoływana w przypadku wychwycenia sygnału.
     * 
     * @param error error code
     * @param signal_number kod sygnału
     */
    void sigint_handler(const boost::system::error_code& error, int signal_number);

    /**
     * Ustawienia dotyczące wyłapywania sygnałów.
     */
    void signals_setup();

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

    //Parsowanie argumentów programu.

    void program_options_setup(int argc, char** argv);

    /**
     * Ustawia timer do wysyłania raportów.
     */
    void timer_setup();

    /**
     * Ustawia zmienne dotyczące sieci.
     */
    void network_setup();

public:

    server();

    //Metoda służąca do ustawienia startowych opcji w programie.

    void setup(int argc, char** argv);

};

#endif	/* SERVER_H */

