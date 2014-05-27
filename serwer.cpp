#include <iostream>
#include <boost/program_options.hpp>
#include "server_attributes.h"
#include "server.h"

namespace po = boost::program_options;

bool parse_command_line(int argc, char** argv)
{
    po::options_description description("Server usage");

    description.add_options()
            ("help,h", "Display this help message")
            ("port,p", po::value<unsigned>(&server_attributes::port), "Port number")
            ("fifosize,F", po::value<unsigned>(&server_attributes::fifo_size), "Fifo size")
            ("fifolowwatermark,L", po::value<unsigned>(&server_attributes::fifo_low_watermark), "Fifo low watermark")
            ("fifohighwatermark,H", po::value<unsigned>(&server_attributes::fifo_high_watermark), "Fifo high watermark")
            ("buflen,X", po::value<unsigned>(&server_attributes::buf_len), "Buffer length")
            ("txinterval,i", po::value<unsigned>(&server_attributes::tx_interval), "Tx interval");

    po::variables_map vm;
    try
    {
        po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
        po::notify(vm);
    }
    catch (std::exception& ex)
    {
        if (!vm.count("help"))
            std::cerr << description;
        return 1;
    }

    if (vm.count("help"))
    {
        std::cerr << description;
        return 1;
    }

    if (!vm.count("fifohighwatermark"))
        server_attributes::fifo_high_watermark = server_attributes::fifo_size;

    return 0;
}

int main(int argc, char** argv)
{

    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.
    asio::io_service io_service;

    if (parse_command_line(argc, argv))
        return 0;

    server serwer(io_service);

    try
    {
        serwer.setup();
        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "[Error] '" << e.what() << "'" << std::endl;
    }

    std::cerr << "[FINISHED]" << std::endl;
}
