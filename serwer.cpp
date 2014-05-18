#include <cstdio>
#include <cstdlib>
#include <signal.h>
#include <unistd.h>
#include <boost/thread.hpp>
#include <boost/program_options.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "err.h"
#include "mixer.h"
namespace po = boost::program_options;
namespace asio = boost::asio;

int port = 3856;
int fifo_size = 10560;
int fifo_low_watermark = 0;
int fifo_high_watermark = fifo_size;
int buf_len = 10;
int tx_interval = 5;

//Zmienna pomocnicza do obsługi sygnałów.
struct sigaction sigint_action;

//Informacja o tym, czy złapano sygnał SIG_INT.
bool is_stopped;

asio::io_service io_service;

//Metoda obsługująca przechwycenie sygnału SIG_INT.
void sigint_handler(int i) {
    is_stopped = true;
    fprintf(stderr, "Przechwycono SIG_INT %d\n", i);
    io_service.stop();
}

//Metoda służąca do ustawienia startowych opcji w programie.

void setup(int argc, char** argv) {
    //Ustawienia dotyczące wyłapywania sygnałów.
    is_stopped = false;

    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sigint_action, 0) == -1)
        syserr("Error in signal\n");

    //Parsowanie argumentów programu.
    po::options_description description("Server usage");

    description.add_options()
            ("help,h", "Distplay this help message")
            ("port,p", po::value<int>(), "Port number")
            ("fifosize,F", po::value<int>(), "Fifo size")
            ("fifolowwatermark,L", po::value<int>(), "Fifo low watermark")
            ("fifohighwatermark,H", po::value<int>(), "Fifo high watermark")
            ("buflen,X", po::value<int>(), "Buffer length")
            ("txinterval,i", po::value<int>(), "Tx interval");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if (vm.count("help")) {
        std::cerr << description;
    }

    if (vm.count("port")) {
        port = vm["port"].as<int>();
        fprintf(stderr, "Port number - %d\n", port);
    }

    if (vm.count("fifosize")) {
        fifo_size = vm["fifosize"].as<int>();
        fprintf(stderr, "Fifo size - %d\n", fifo_size);
    }

    if (vm.count("fifolowwatermark")) {
        fifo_low_watermark = vm["fifohighwatermark"].as<int>();
        fprintf(stderr, "Fifo low watermark - %d\n", fifo_low_watermark);
    }

    if (vm.count("fifohighwatermark")) {
        fifo_high_watermark = vm["fifohighwatermark"].as<int>();
        fprintf(stderr, "Fifo high watermark - %d\n", fifo_high_watermark);
    }

    if (vm.count("buflen")) {
        buf_len = vm["buflen"].as<int>();
        fprintf(stderr, "Buffer length - %d\n", buf_len);
    }

    if (vm.count("txinterval")) {
        tx_interval = vm["txinterval"].as<int>();
        fprintf(stderr, "Tx interval - %d\n", tx_interval);
    }
}

/////////
asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
asio::ip::tcp::acceptor acceptor(io_service, endpoint);
asio::ip::tcp::socket sock(io_service);
boost::array<char, 4096> buffer; 
std::string data = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, world!\n"; 

void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cout << "WRITE HANDLER\n";
}

void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred) 
{ 
  if (!ec) 
  { 
    std::cout << std::string(buffer.data(), bytes_transferred) << std::endl; 
    sock.async_read_some(boost::asio::buffer(buffer), read_handler); 
  } 
} 

void accept_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        asio::async_write(sock, boost::asio::buffer(data), write_handler);
        sock.async_read_some(asio::buffer(buffer), read_handler);
    }
}

int main(int argc, char** argv) {
    setup(argc, argv);

    acceptor.listen();
    acceptor.async_accept(sock, accept_handler);
    io_service.run();

    fprintf(stderr, "Koniec wykonania\n");
    //    mixer_input* minput;
    //    void* pointer;
    //    size_t lele;
    //    mixer(minput, lele, pointer, &lele, 2);
    //while(!is_stopped)
    //{
    //	fprintf(stderr, "Serwer sobie działa\n");
    //	sleep(1);
    //}
}
