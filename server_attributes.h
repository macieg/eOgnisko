#ifndef SERVER_ATTRIBUTES
#define SERVER_ATTRIBUTES
#include <cstdio>

class server_attributes {
private:
    server_attributes();
public:
    static int port;
    static int fifo_size;
    static int fifo_low_watermark;
    static int fifo_high_watermark;
    static int buf_len;
    static int tx_interval;
};

#endif
