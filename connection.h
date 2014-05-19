
#ifndef CONNECTION_H
#define	CONNECTION_H

//Klasa reprezentująca połączenie klienta z serwerem
//utrzymuje wszelkie dane na temat połączenia.
class connection {
private:
    int client_id;
    
public:
    connection(int client_id);
    
    //Zwraca id klienta.
    int get_client_id();
};

#endif	/* CONNECTION_H */

