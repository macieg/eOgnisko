#include "server_attributes.h"

server_attributes::server_attributes()
{

}

int server_attributes::port = 3856;
int server_attributes::fifo_size = 10560;
int server_attributes::fifo_low_watermark = 0;
int server_attributes::fifo_high_watermark = fifo_size;
int server_attributes::buf_len = 10;
int server_attributes::tx_interval = 1000; //TODO zmienić na 5

