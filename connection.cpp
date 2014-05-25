#include "connection.h"

connection::connection(int client_id, boost::asio::ip::tcp::socket sock) :
client_id(client_id),
current_bytes_in_fifo(0),
min_bytes_in_fifo(0),
max_bytes_in_fifo(0),
sock(std::move(sock))
{
    fifo = new char[server_attributes::fifo_size];
}

connection::~connection()
{
    delete[] fifo;
}

asio::ip::tcp::socket& connection::get_tcp_socket()
{
    return sock;
}

asio::ip::udp::endpoint& connection::get_udp_endpoint()
{
    return ep_udp;
}

void connection::set_udp_endpoint(asio::ip::udp::endpoint& ep)
{
    this->ep_udp = ep;
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

int connection::get_current_bytes_in_fifo()
{
    return current_bytes_in_fifo;
}

int connection::get_min_bytes_in_fifo()
{
    return min_bytes_in_fifo;
}

int connection::get_max_bytes_int_fifo()
{
    return max_bytes_in_fifo;
}

void connection::append(std::string& data)
{
    strcpy(fifo, data.c_str());
    current_bytes_in_fifo += data.size();
    min_bytes_in_fifo = std::min(min_bytes_in_fifo, current_bytes_in_fifo);
    max_bytes_in_fifo = std::max(max_bytes_in_fifo, current_bytes_in_fifo);
}

void connection::clear_history()
{
    min_bytes_in_fifo = current_bytes_in_fifo;
    max_bytes_in_fifo = current_bytes_in_fifo;
}
