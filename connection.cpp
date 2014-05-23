#include "connection.h"

connection::connection(int client_id, boost::asio::ip::tcp::socket sock) :
client_id(client_id),
sock(std::move(sock))
{
}

boost::asio::ip::tcp::socket& connection::get_tcp_socket()
{
    return sock;
}

int connection::get_client_id()
{
    return client_id;
}

int connection::get_time_from_last_udp()
{
    boost::posix_time::time_duration tm(boost::posix_time::second_clock::local_time() - last_udp);
    return tm.total_milliseconds();
}

void connection::update_udp_time()
{
    last_udp = boost::posix_time::second_clock::local_time();
}
