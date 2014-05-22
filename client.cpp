#include <boost/bind.hpp>
#include "client.h"

void client::receive_handler(const boost::system::error_code &ec, std::size_t bytes_transferred)
{
    //    std::cerr << "READ HANDLER " << bytes_transferred << "\n";
    if (!ec)
    {
        //std::cerr << "READ SUCCESS " << "\n";
        std::ostringstream ss;
        ss << &s;
        std::cerr << ss.str();
    }
    
    asio::async_read_until(sock,
            s, '\n',
            boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
            );
}

void client::connect_handler(const boost::system::error_code &ec)
{
    if (!ec)
    {
        std::cerr << "CONNECT SUCCESS \n";
        asio::async_read_until(sock,
                s, '\n',
                boost::bind(&client::receive_handler, this, asio::placeholders::error, asio::placeholders::bytes_transferred)
                );
    }
    else
    {
        std::cerr << "CONNECT FAIL\n";
//        connect_timer.expires_from_now(boost::posix_time::seconds(TIMER_INTERVAL));
        connect_timer.wait();
    }
}

void client::resolve_handler(const boost::system::error_code &ec, asio::ip::tcp::resolver::iterator it)
{
    if (!ec)
    {
        sock.async_connect(*it, boost::bind(&client::connect_handler, this, asio::placeholders::error));
    }
}

void client::sigint_handler(const boost::system::error_code& error, int signal_number)
{
    if (!error)
    {
        std::cerr << "Otrzymano sygnał " << signal_number << "konczenie pracy\n";
        io_service.stop();
    }
}

void client::setup_signals()
{
    asio::signal_set signals(io_service, SIGINT);
    signals.async_wait(boost::bind(&client::sigint_handler,
            this,
            asio::placeholders::error,
            asio::placeholders::signal_number)
            );
}

bool client::program_options_setup(int argc, char **argv)
{
    //Parsowanie argumentów programu.
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

void client::setup_networking()
{
    asio::ip::tcp::resolver::query query(server, port);
    resolver.async_resolve(query,
            boost::bind(&client::resolve_handler, this, asio::placeholders::error, asio::placeholders::iterator)
            );
    io_service.run();
}

client::client() : resolver(io_service), sock(io_service), connect_timer(io_service, boost::posix_time::seconds(TIMER_INTERVAL))
{
}

void client::setup(int argc, char **argv)
{
    if (program_options_setup(argc, argv))
        return;
    setup_signals();
    setup_networking();
}
