
#ifndef CONNECTION_H
#define	CONNECTION_H
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

/**
 * Klasa reprezentująca połączenie klienta z serwerem
 * utrzymuje wszelkie dane na temat połączenia.
 */
class connection {
private:
    int client_id;
    boost::posix_time::ptime last_udp;
    
    boost::asio::ip::tcp::socket sock;


public:

    connection(int client_id, boost::asio::ip::tcp::socket sock);

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

};

#endif	/* CONNECTION_H */

