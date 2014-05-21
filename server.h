
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
    int port = 3856;
    int fifo_size = 10560;
    int fifo_low_watermark = 0;
    int fifo_high_watermark = fifo_size;
    int buf_len = 10;
    int tx_interval = 5;

    ///////////////////////////////////////////////////////////////////
    asio::io_service io_service;
    asio::ip::tcp::endpoint endpoint;
    asio::ip::tcp::acceptor acceptor;
    asio::ip::tcp::socket sock;
    boost::array<char, 4096> buffer;
    ///////////////////////////////////////////////////////////////////


    //Pomocnicza sekwencja do tworzenia id klientów.
    int id_sequence = 1;

    /**
     * Struktura danych przechowująca userów.
     * //TODO być może do zmiany
     */
    std::map<asio::ip::tcp::endpoint, boost::shared_ptr<connection> > connections_map;

    void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);

    void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred);

    /**
     * Funkcja wywoływana, io_service otrzyma do obsługi połączenie.
     * 
     * @param ec error code
     */
    void accept_handler(const boost::system::error_code &ec);

    void sigint_handler(const boost::system::error_code& error, int signal_number);

    //Ustawienia dotyczące wyłapywania sygnałów.

    void signals_setup();
    
    /**
     * Metoda tworząca wiadomość jako odpowiedź dla klienta, któremu udało się połączyć
     * z informacją o jego id.
     * 
     * @param client_id
     * @return string w formacie ""CLIENT clientid\n"
     */
    std::string create_accept_response(int client_id);

    //Parsowanie argumentów programu.

    void program_options_setup(int argc, char** argv);
    //Ustawienia sieciowe.

    void network_setup();

public:

    server() : endpoint(asio::ip::tcp::v4(), port), acceptor(io_service, endpoint), sock(io_service) {
    }

    //Metoda służąca do ustawienia startowych opcji w programie.

    void setup(int argc, char** argv);

};

#endif	/* SERVER_H */

