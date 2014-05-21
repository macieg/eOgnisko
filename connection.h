
#ifndef CONNECTION_H
#define	CONNECTION_H
#include <boost/asio.hpp>

/**
 * Klasa reprezentująca połączenie klienta z serwerem
 * utrzymuje wszelkie dane na temat połączenia.
 */
class connection {
private:
    int client_id;
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
    boost::asio::ip::tcp::socket& get_socket();

};

#endif	/* CONNECTION_H */

