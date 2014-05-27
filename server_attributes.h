#ifndef SERVER_ATTRIBUTES
#define SERVER_ATTRIBUTES
#include <cstdio>

class server_attributes {
private:
    server_attributes();
public:
    static unsigned port;
    static unsigned fifo_size;
    static unsigned fifo_low_watermark;
    static unsigned fifo_high_watermark;
    static unsigned buf_len;
    static unsigned tx_interval;
};

#endif
