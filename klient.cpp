#include <iostream>
#include <boost/program_options.hpp>
#include "client.h"

namespace po = boost::program_options;

int retransmit_limit = 10;
std::string port = "3856";
std::string server;

bool parse_command_line(int argc, char **argv)
{
    po::options_description description("Client usage");
    po::variables_map vm;

    try
    {
        description.add_options()
                ("help,h", "Display this help message")
                ("server,s", po::value<std::string>(&server)->required(), "Server name")
                ("port,p", po::value<std::string>(&port), "Port number")
                ("retransmit,X", po::value<int>(&retransmit_limit), "Retransmit limit");


        po::store(po::command_line_parser(argc, argv).options(description).run(), vm);

        if (vm.count("help"))
            std::cerr << description;

        po::notify(vm);
    }
    catch (std::exception& e)
    {
        if (!vm.count("help"))
            std::cerr << description;
        return true;
    }

    return false;
}

int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.

    if (parse_command_line(argc, argv))
        return 1;

    while (true)
    {
        try
        {
            asio::io_service io_service;
            client klient(io_service);
            klient.setup(retransmit_limit, port, server);
            io_service.run();
        }
        catch (std::exception& ex)
        {
            std::cerr << "[Error] Exception - " <<  ex.what() << std::endl;
        }
        sleep(1);
    }

    std::cerr << "[Info] Finished" << std::endl;
    return 0;
}
