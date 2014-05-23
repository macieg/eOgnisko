#include <iostream>
#include <boost/program_options.hpp>
#include "server.h"

namespace po = boost::program_options;

int port = 3856;
int fifo_size = 10560;
int fifo_low_watermark = 0;
int fifo_high_watermark = fifo_size;
int buf_len = 10;
int tx_interval = 100; //TODO zmienić na 5
int TIMER_INTERVAL = 1;

bool parse_command_line(int argc, char** argv)
{
    po::options_description description("Server usage");

    description.add_options()
            ("help,h", "Display this help message")
            ("port,p", po::value<int>(&port), "Port number")
            ("fifosize,F", po::value<int>(&fifo_size), "Fifo size")
            ("fifolowwatermark,L", po::value<int>(&fifo_low_watermark), "Fifo low watermark")
            ("fifohighwatermark,H", po::value<int>(&fifo_high_watermark), "Fifo high watermark")
            ("buflen,X", po::value<int>(&buf_len), "Buffer length")
            ("txinterval,i", po::value<int>(&tx_interval), "Tx interval");

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
        fifo_low_watermark = fifo_size;

    return 0;
}

int main(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.
    asio::io_service io_service;

    asio::signal_set signals(io_service, SIGINT);
    signals.async_wait([&](boost::system::error_code ec, std::size_t sn) {
        if (!ec)
        {
            std::cerr << "Otrzymano sygnał " << sn << " konczenie pracy\n";
            io_service.stop();
        }
    });
    if (parse_command_line(argc, argv))
        return 0;
    
    server serwer(io_service, port);
    serwer.setup(fifo_size,
            fifo_low_watermark,
            fifo_high_watermark,
            buf_len,
            tx_interval);
    
    io_service.run();
    std::cerr << "Koniec wykonania" << std::endl;
}
