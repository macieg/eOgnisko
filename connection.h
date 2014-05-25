
#ifndef CONNECTION_H
#define	CONNECTION_H
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "server_attributes.h"
#include "mixer.h"

namespace asio = boost::asio;

/**
 * Klasa reprezentująca połączenie klienta z serwerem
 * utrzymuje wszelkie dane na temat połączenia.
 */
class connection {

    enum class FIFO_STATE {
        FILLING, ACTIVE
    };
    
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
    FIFO_STATE fifo_state;
    struct mixer_input mixer_input;

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
     * @param cstring dołączany do fifo
     * @param długość nagłówka
     * @param długość danych
     */
    void append(const char*, int, int);

    /**
     * Czyści historię "minimum/maksimum" danych w kolejce.
     */
    void clear_history();

    /**
     * Zwraca informację o mówiącą, czy kolejka FIFO
     * połączenia znajduje się w stanie ACTIVE.
     * 
     * @return true jezeli jest, false wpp
     */
    bool is_fifo_active();

    /**
     * Usuwa z kolejki pierwsze n bajtów.
     * 
     * @param ilość bajtów do usunięcia
     */
    void consume_fifo();

    /**
     * Zwraca obiekt mixer input powiązany z połączeniem.
     * 
     * @return obiekt struct mixer input
     */
    struct mixer_input get_mixer_input();
    
    /**
     * Ustawia mixer_input powiązany z połączeniem.
     * 
     * @param struct mixer_input
     */
    void set_mixer_intput(struct mixer_input);
    
    /**
     * Zwraca informację o ilości wolnego miejsca w kolejce.
     * 
     * @return ilość wolnego miejsca w kolejce w bajtach
     */
    int left_bytes_in_fifo();

};

#endif	/* CONNECTION_H */

