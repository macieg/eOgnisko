
//Narzędzia pomocnicze.
#ifndef UTILS_H
#define	UTILS_H

/**
 * Metoda tworząca wiadomość jako odpowiedź dla klienta, któremu udało się połączyć
 * z informacją o jego id.
 * 
 * @param client_id
 * @return string w formacie ""CLIENT clientid\n"
 */
std::string create_accept_response(int client_id);

#endif

