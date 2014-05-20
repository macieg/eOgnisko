#include <iostream>
#include "server.h"

//TODO przenieść program_options tutaj, albo do utils i zmienić metode serwera na run
int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.
    server serwer;
    serwer.setup(argc, argv);
    std::cerr << "Koniec wykonania" << std::endl;
}
