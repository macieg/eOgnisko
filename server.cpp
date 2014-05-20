#include <boost/program_options.hpp>
#include "err.h"
#include "mixer.h"
#include "connection.h"
#include "utils.h"
#include "server.h"

namespace po = boost::program_options;

void server::sigint_handler(int i)
{
    fprintf(stderr, "Przechwycono SIG_INT %d\n", i);
    io_service.stop();
}

void server::write_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cout << "WRITE HANDLER\n";
}

void server::read_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //    sock.async_read_some(boost::asio::buffer(buffer), read_handler); 
        std::cout << std::string(buffer.data(), bytes_transferred) << std::endl;

    }
}

void server::accept_handler(const boost::system::error_code &ec)
{

    if (!ec)
    {

        connection conn(id_sequence++);

        asio::async_write(sock,
                asio::buffer(create_accept_response(conn.get_client_id())),
                boost::bind(&server::write_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
        //        sock.async_read_some(asio::buffer(buffer), read_handler);
        std::cerr << "NOWY KLIENT " << sock.remote_endpoint() << "\n";

    }
    else
    {
        //        std::cerr << "ACCEPT HANDLER FAILURE " << ec << "\n";
    }

    acceptor.async_accept(
            sock,
            boost::bind(&server::accept_handler, this, asio::placeholders::error)
            );

    sock = asio::ip::tcp::socket(io_service);
}

void server::sigint_handler(const boost::system::error_code& error, int signal_number)
{
    if (!error)
    {
        std::cerr << "Otrzymano sygnał " << signal_number << "konczenie pracy\n";
        io_service.stop();
    }
}

void server::signals_setup()
{
    asio::signal_set signals(io_service, SIGINT);
    signals.async_wait(
            boost::bind(&server::sigint_handler, this,
            asio::placeholders::error, asio::placeholders::signal_number)
            );
}

void server::program_options_setup(int argc, char** argv)
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

void server::network_setup()
{
    //    asio::io_service::work work(io_service);
    //Trick, zapobiega blokowaniu się gniazda.
    acceptor.set_option(asio::socket_base::reuse_address(true));
    acceptor.listen();
    acceptor.async_accept(sock,
            boost::bind(&server::accept_handler, this, asio::placeholders::error)
            );
    io_service.run();
}

void::server::setup(int argc, char** argv)
{
    program_options_setup(argc, argv);
    signals_setup();
    network_setup();
}

