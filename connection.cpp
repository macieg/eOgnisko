#include "connection.h"

connection::connection(int client_id, boost::asio::ip::tcp::socket sock) :
client_id(client_id),
sock(std::move(sock))
{
}

boost::asio::ip::tcp::socket& connection::get_socket()
{
    return sock;
}

int connection::get_client_id()
{
    return client_id;
}

