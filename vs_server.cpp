#include "vs_server.h"

VS_Server::VS_Server(u_int32_t ip, SERVER_STATE state)
{
    server_ip = ip;
    server_status = state;
    time_to_death = TIME_TO_DEATH;
}

VS_Server::~VS_Server()
{

}

u_int32_t VS_Server::getIp()
{
    return server_ip;
}

SERVER_STATE VS_Server::getState()
{
    return server_status;
}

u_int8_t VS_Server::getTTD()
{
    return time_to_death;
}

void VS_Server::setState(SERVER_STATE state)
{
    server_status = state;
}

void VS_Server::refreshTTD()
{
    time_to_death = TIME_TO_DEATH;
}

void VS_Server::decrementTTD()
{
    if(time_to_death != 0)
        time_to_death --;
}
