
#ifndef CONNECTION_H
#define	CONNECTION_H
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "server_attributes.h"

namespace asio = boost::asio;
/**
 * Klasa reprezentująca połączenie klienta z serwerem
 * utrzymuje wszelkie dane na temat połączenia.
 */
class connection {
private:
    int client_id;
    int current_bytes_in_fifo;
    int min_bytes_in_fifo;
    int max_bytes_in_fifo;
    boost::posix_time::ptime last_udp;
    boost::asio::ip::tcp::socket sock;
    asio::ip::udp::endpoint ep_udp;
    char* fifo;
    int nr_global;
    int ack_global;


public:

    connection(int client_id, boost::asio::ip::tcp::socket sock);

    ~connection();
    /**
     * Zwraca id klienta.
     * 
     * @return id klienta
     */
    int get_client_id();

    /**
     * Zwraca referencję na socket związany z połączeniem.
     * 
     * @return referencja na socket
     */
    boost::asio::ip::tcp::socket& get_tcp_socket();
    
    /**
     * Zwraca referecje na udp endpoint.
     * 
     * @return udp endpoint
     */
    asio::ip::udp::endpoint& get_udp_endpoint();
    
    /**
     * Ustawia endpoint udp połączenia (końcówka klienta).
     * 
     * @param ep endpoint klienta
     */
    void set_udp_endpoint(asio::ip::udp::endpoint& ep);

    /**
     * Zwraca różnicę czasu od ostatniej operacji UDP w
     * kierunku klient->serwer.
     * 
     * @return czas ostatniego datagramu klient -> serwer
     */
    int get_time_from_last_udp();

    /**
     * Uaktualnia czas ostatniej operacji udp wykonanej w połączeniu
     * w kierunku klient->serwer.
     * 
     */
    void update_udp_time();

    /**
     * Zwraca akutalną liczbę bajtów znajdującyh się w kolejce.
     * 
     * @return aktualna liczba bajtów w kolejce.
     */
    int get_current_bytes_in_fifo();

    /**
     * Zwraca minimialną liczbę bajtów dotychczas znajdujących się w kolejce.
     * 
     * @return minimalna dotychczasowa liczba bajtów w kolejce
     */
    int get_min_bytes_in_fifo();

    /**
     * Zwraca maksymalną liczbę bajtów dotychczas znajdujących się w kolejce.
     * 
     * @return maksymalna dotychczasowa liczba bajtów w kolejce
     */
    int get_max_bytes_int_fifo();
    
    /**
     * Dołącza do fifo danego w argumencie stringa.
     * 
     * @param string dołączany do fifo
     */
    void append(std::string&);
    
    /**
     * Czyści historię "minimum/maksimum" danych w kolejce.
     */
    void clear_history();

};

#endif	/* CONNECTION_H */

