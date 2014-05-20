#include <iostream>
#include "err.h"
#include "client.h"

int main(int argc, char** argv)
{
    client klient;
    klient.setup(argc, argv);
    std::cerr << "Koniec\n";
}
