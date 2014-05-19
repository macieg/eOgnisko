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
#include "connection.h"
#include "utils.h"
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

///////////////////////////////////////////////////////////////////
asio::io_service io_service;
asio::ip::tcp::endpoint endpoint(asio::ip::tcp::v4(), port);
asio::ip::tcp::acceptor acceptor(io_service, endpoint);
asio::ip::tcp::socket sock(io_service);
boost::array<char, 4096> buffer;
///////////////////////////////////////////////////////////////////

//Pomocnicza sekwencja do tworzenia id klientów.
int id_sequence = 1;

//Metoda obsługująca przechwycenie sygnału SIG_INT.

void sigint_handler(int i)
{
    is_stopped = true;
    fprintf(stderr, "Przechwycono SIG_INT %d\n", i);
    io_service.stop();
}

void write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cout << "WRITE HANDLER\n";
}

void read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //    sock.async_read_some(boost::asio::buffer(buffer), read_handler); 
        std::cout << std::string(buffer.data(), bytes_transferred) << std::endl;

    }
}

/**
 * Funkcja wywoływana, io_service otrzyma do obsługi połączenie.
 * 
 * @param ec error code
 */
void accept_handler(const boost::system::error_code &ec)
{
    static int i = 1;

    std::cerr << "ACCEPT HANDLER\n";
    if (!ec)
    {
        std::cerr << "ACCEPT HANDLER SUCCESS\n";

        connection conn(id_sequence++);

        asio::async_write(sock, asio::buffer(create_accept_response(conn.get_client_id())), write_handler);
        sock.async_read_some(asio::buffer(buffer), read_handler);
    }
    else
    {
        std::cerr << "ACCEPT HANDLER FAILURE " << ec << "\n";
    }

    acceptor.async_accept(sock, accept_handler);
    sock = asio::ip::tcp::socket(io_service);
    std::cerr << "ACCEPT " << i++ << std::endl;
}


//Ustawienia dotyczące wyłapywania sygnałów.

void signals_setup()
{
    is_stopped = false;

    sigint_action.sa_handler = sigint_handler;
    sigemptyset(&sigint_action.sa_mask);
    sigint_action.sa_flags = SA_RESTART;

    if (sigaction(SIGINT, &sigint_action, 0) == -1)
        syserr("Error in signal\n");
}

//Parsowanie argumentów programu.

void program_options_setup(int argc, char** argv)
{
    po::options_description description("Server usage");

    description.add_options()
            ("help,h", "Distplay this help message")
            ("port,p", po::value<int>(&port), "Port number")
            ("fifosize,F", po::value<int>(&fifo_size), "Fifo size")
            ("fifolowwatermark,L", po::value<int>(&fifo_low_watermark), "Fifo low watermark")
            ("fifohighwatermark,H", po::value<int>(&fifo_high_watermark), "Fifo high watermark")
            ("buflen,X", po::value<int>(&buf_len), "Buffer length")
            ("txinterval,i", po::value<int>(&tx_interval), "Tx interval");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(description).run(), vm);
    po::notify(vm);

    if (vm.count("help"))
        std::cerr << description;
}

//Ustawienia sieciowe.

void network_setup()
{
    //    asio::io_service::work work(io_service);
    //Trick, zapobiega blokowaniu się gniazda.
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.listen();
    acceptor.async_accept(sock, accept_handler);
    io_service.run();
}

//Metoda służąca do ustawienia startowych opcji w programie.

void setup(int argc, char** argv)
{
    std::ios_base::sync_with_stdio(false); //Przyspieszenie cerr.
    program_options_setup(argc, argv);
    signals_setup();
    network_setup();
}

int main(int argc, char** argv)
{
    setup(argc, argv);
    std::cerr << "Koniec wykonania" << std::endl;
}
