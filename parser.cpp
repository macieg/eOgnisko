#include "parser.h"

parser::parser() :
pattern_client_id("CLIENT ([0-9]+)\n"),
pattern_ack("ACK ([0-9]*) ([0-9]+)\n"),
pattern_retransmit("RETRANSMIT ([0-9]+)\n"),
pattern_keepalive("KEEPALIVE\n"),
pattern_upload("UPLOAD ([0-9]+)"),
pattern_data("DATA ([0-9]+) ([0-9]+) ([0-9]+)")
{
}

bool parser::matches_client_id(const char* input_string, int& client_id)
{
    boost::cmatch groups;
    if (boost::regex_match(input_string, groups, pattern_client_id))
    {
        client_id = std::atoi(groups[1].first);
        return true;
    }

    return false;
}

bool parser::matches_client_id(std::string& input_string, int& client_id)
{
    boost::cmatch groups;
    if (boost::regex_match(input_string.c_str(), groups, pattern_client_id))
    {
        client_id = std::atoi(groups[1].first);
        return true;
    }

    return false;
}

bool parser::matches_ack(const char* input_string, int& ack, int& win)
{
    boost::cmatch groups;
    if (boost::regex_match(input_string, groups, pattern_ack))
    {
        ack = std::atoi(groups[1].first);
        win = std::atoi(groups[2].first);
        return true;
    }

    return false;
}

bool parser::matches_retransmit(const char* input_string, int& nr)
{
    boost::cmatch groups;
    if (boost::regex_match(input_string, groups, pattern_retransmit))
    {
        nr = std::atoi(groups[1].first);
        return true;
    }

    return false;
}

bool parser::matches_keepalive(const char* input_string)
{
    boost::cmatch groups;
    if (boost::regex_match(input_string, groups, pattern_keepalive))
        return true;
    return false;
}

bool parser::matches_upload(const char* input_string, int& nr, int size, int& start_point)
{
    boost::cmatch groups;
    const char* endp = (const char*) memchr(input_string, '\n', size);
    if (boost::regex_match(input_string, endp, groups, pattern_upload))
    {
        nr = std::atoi(groups[1].first);
        start_point = endp - input_string + 1;
        return true;
    }
    return false;
}

bool parser::matches_data(const char* input_string, int& nr, int& ack, int& win, int size, int& start_point)
{
    boost::cmatch groups;
    const char* endp = (const char*) memchr(input_string, '\n', size);
    if (boost::regex_match(input_string, endp, groups, pattern_data))
    {
        nr = std::atoi(groups[1].first);
        ack = std::atoi(groups[2].first);
        win = std::atoi(groups[3].first);
        start_point = endp - input_string + 1;
        return true;
    }
    return false;
}