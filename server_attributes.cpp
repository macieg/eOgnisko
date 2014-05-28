#include "server_attributes.h"

server_attributes::server_attributes()
{

}

unsigned server_attributes::port = 3856;
unsigned server_attributes::fifo_size = 10560;
unsigned server_attributes::fifo_low_watermark = 0;
unsigned server_attributes::fifo_high_watermark = fifo_size;
unsigned server_attributes::buf_len = 10;
unsigned server_attributes::tx_interval = 5;

