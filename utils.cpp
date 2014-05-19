#include <string>
#include <boost/lexical_cast.hpp>
#include "utils.h"

std::string create_accept_response(int client_id)
{
    std::string response("CLIENT ");
    response.append(boost::lexical_cast<std::string>(client_id));
    response.append("\n");
    return std::move(response);
}