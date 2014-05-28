#include "connection.h"

connection::connection(int client_id, boost::asio::ip::tcp::socket sock) :
client_id(client_id),
current_bytes_in_fifo(0),
min_bytes_in_fifo(0),
max_bytes_in_fifo(0),
sock(std::move(sock)),
datagram_number(0),
fifo_state(FIFO_STATE::FILLING)
{
    fifo = new char[server_attributes::fifo_size];
}

connection::~connection()
{
    //std::cout << "[Info] Lost client with client_id = '" << client_id << "'" << std::endl;
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

void connection::append(const char* data, int header_size, int data_size)
{
    memcpy(fifo + current_bytes_in_fifo, data + header_size, data_size);
    //if (data_size < 0)
      //  throw 1;
    current_bytes_in_fifo += data_size;
    min_bytes_in_fifo = std::min(min_bytes_in_fifo, current_bytes_in_fifo);
    max_bytes_in_fifo = std::max(max_bytes_in_fifo, current_bytes_in_fifo);

    if (current_bytes_in_fifo >= server_attributes::fifo_high_watermark)
        fifo_state = FIFO_STATE::ACTIVE;
}

void connection::clear_history()
{
    min_bytes_in_fifo = current_bytes_in_fifo;
    max_bytes_in_fifo = current_bytes_in_fifo;
}

bool connection::is_fifo_active()
{
    return (fifo_state == FIFO_STATE::ACTIVE);
}

void connection::consume_fifo()
{
    size_t n = mixer_input.consumed;
    //    if (n) std::cerr << "[Info] user(" << client_id << ") fifo_state1 (" << n << "/ " << current_bytes_in_fifo << ")" << std::endl;
    memmove(fifo, fifo + n, current_bytes_in_fifo - n);
    current_bytes_in_fifo -= n;
    //    std::cerr << "[Info] user(" << client_id << ") fifo_state2 (" << current_bytes_in_fifo << ")" << std::endl;
    if (current_bytes_in_fifo <= server_attributes::fifo_low_watermark)
        fifo_state = FIFO_STATE::FILLING;
}

struct mixer_input connection::get_mixer_input()
{
    mixer_input.data = fifo;
    mixer_input.len = current_bytes_in_fifo;
    mixer_input.consumed = 0;
    return mixer_input;
}

void connection::set_mixer_intput(struct mixer_input mi)
{
    this->mixer_input = mi;
}

int connection::left_bytes_in_fifo()
{
    return server_attributes::fifo_size - current_bytes_in_fifo;
}

unsigned connection::get_datagram_number() const
{
    return datagram_number;
}

void connection::set_datagram_number(unsigned datagram_number)
{
    this->datagram_number = datagram_number;
}