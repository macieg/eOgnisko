#include "connection.h"

connection::connection(int client_id) : client_id(client_id)
{}

int connection::get_client_id()
{
    return client_id;
}
