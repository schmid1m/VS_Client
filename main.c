#include <stdio.h>
#include <string.h>
#include "VS_LAB/Macros.h"
#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"

int main(void)
{
    // Parameters: client ID, client priority, broadcast address
    init_client(1, 1, parseIPV4string(BROADCAST_ADDRESS));

    // Parameters: generator polynom, server ip address
    send_gp_req(0x52AC, inet_network("127.0.0.1"));

    return 0;
}

