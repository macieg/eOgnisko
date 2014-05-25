
#ifndef PARSER_H
#define	PARSER_H
#include <boost/regex.hpp>

class parser {
private:
    const boost::regex pattern_client_id;
    const boost::regex pattern_ack;
    const boost::regex pattern_retransmit;
    const boost::regex pattern_keepalive;
    const boost::regex pattern_upload;
    const boost::regex pattern_data;

public:

    parser();

    /**
     * Sprawdza, czy zadany cstring pasuje do:
     * CLIENT clientid\n
     * 
     * @param zadany cstring
     * @param id klienta
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_client_id(const char*, int&);

    /**
     * Sprawdza, czy zadany string pasuje do:
     * CLIENT clientid\n
     * 
     * @param zadany string
     * @param id klienta
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_client_id(std::string&, int&);

    /**
     * Sprawdza, czy zadany string pasuje do:
     * ACK ack win\n
     * 
     * @param zadany string
     * @param ack
     * @param win
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_ack(const char*, int&, int&);

    /**
     * Sprawdza, czy zadany string pasuje do:
     * RETRANSMIT nr\n
     * 
     * @param zadany string
     * @param nr
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_retransmit(const char*, int&);

    /**
     * Sprawdza, czy zadany string pasuje do:
     * KEEPALIVE\n
     * 
     * @param zadany string
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_keepalive(const char*);

    /**
     * Sprawdza, czy zadany string pasuje do:
     * UPLOAD nr\n
     * [dane]
     * 
     * @param zadany string
     * @param nr
     * @param dane
     * @param liczna bajtów w stringu
     * @param rozmiar nagłowka
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_upload(const char*, int&, int, int&);

    /**
     * DATA nr ack win\n
     * [dane]
     * 
     * @param zadany string
     * @param nr
     * @param ack
     * @param win
     * @param liczba bajtów w stringu
     * @param rozmiar nagłówka
     * @return true jezeli pasuje, false w przeciwnym wypadku
     */
    bool matches_data(const char*, int&, int&, int&, int, int&);
};


#endif	/* PARSER_H */

