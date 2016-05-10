/**************************************************************
**  File        : main.c (client)                            **
**  Version     : 1.0                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 10.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "timeoutlib.h"
#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"
#include "VS_LAB/Macros.h"

#define BLOCKLEN 5000

// Arguments:
// vslabc <infile> <outfile> <ip>
int main(int argc, char **argv)
{
    uint8_t gp[2], **out_data, iReturn, *ip_ptr;
    uint16_t in_data[BLOCKLEN], resBlockID, blockID = 0;
	uint32_t senderIP, length, i;
	bool newDataWanted = true;
	FILE *fpIn, *fpOut;
	msg msg_ptr;
	FID msg_type;

    out_data = malloc(sizeof(uint8_t*));

	// introduce yourself
	printf("VSLab client, build %s %s\n", __DATE__, __TIME__);

	// check command line parameters
	if (argc < 4) {
		printf("Missing arguments!\n");
		printf("Usage: main.c infile outfile ip\n");
		return -1;
	}

	// open in-filepointer
	fpIn=fopen(argv[1],"rb");
	if(!fpIn)	{
		printf("Unable to open file!");
		return -1;
	}

	// open out-filepointer
	fpOut = fopen(argv[2], "w");

    // init socket
    init_client(1 /*clientID*/, 1 /*priority*/, 0 /*broadcast address*/);

    /********** SETTING GP ***************************************************/

    // read in generator polynom
    length = fread(&gp,sizeof(uint8_t),2,fpIn);
    if(length != 2) {
    	printf("Client: Reading in gp was not possible\n");
    	return ERROR;
    }

    while(1)
    {  	// send it out
    	iReturn = send_gp_req((gp[0] << 8) + gp[1], inet_network(argv[3]));
    	if(iReturn != NO_ERROR) {
    		printf("Client: GP (%#x) sent out failed: %d\n", (gp[0] << 8) + gp[1], iReturn);
    		sleep(1); continue;
    	} else {
    		printf("Client: GP (%#x) sent out worked\n", (gp[0] << 8) + gp[1]);
    	}

    	// receive gp-set-response
    	tol_start_timeout(TOL_TIMEOUT_SECS);
    	iReturn = recv_msg(&msg_ptr, &senderIP);
    	tol_stop_timeout();
        if (tol_is_timed_out()) {
            tol_reset_timeout();
            printf("Client: Timeout --> Send GP-Request again\n");
            continue;
        }
    	msg_type = get_msg_type(&msg_ptr);
    	if(iReturn != NO_ERROR || msg_type != GP_RSP) {
    		printf("Client: No gp response received: %d\n\n", iReturn);
    		sleep(1); continue;
    	} else {
    		ip_ptr = (uint8_t*)&senderIP;
    		printf("Client: Received gp response from %d.%d.%d.%d\n\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    		free_msg(&msg_ptr);
    		break;
    	}
    }

	/********** SCRAMBLING DATA **********************************************/

	while(1)
	{	// read in data
		if(newDataWanted) {
			length = fread(&in_data,sizeof(uint16_t),BLOCKLEN,fpIn);
			if(length < 1)
				break;
			for(i=0; i<length; i++)
			{
				in_data[i] = htons(in_data[i]); // correcting byteorder
			}
			printf("Client: Read in data: ");
			if(BLOCKLEN < 20)
				for(i=0; i<length; i++)
					printf("%#x ", in_data[i]);
			else
				printf("...");
			printf("\n");
			newDataWanted = false;
		}

		// send it out
		iReturn = send_dec_req(blockID, in_data, length, inet_network(argv[3]));
		if(iReturn != NO_ERROR) {
			printf("Client: Sending out data was not successful: %d\n", iReturn);
			sleep(0.5); continue;
		} else {
			printf("Client: Sending out data worked\n");
		}

		// receive scrambled data
		tol_start_timeout(TOL_TIMEOUT_SECS);
		iReturn = recv_msg(&msg_ptr, &senderIP);
		tol_stop_timeout();
        if (tol_is_timed_out()) {
            tol_reset_timeout();
            printf("Client: Timeout --> Send data again (Block-ID %d)\n", blockID);
            continue;
        }
		msg_type = get_msg_type(&msg_ptr);
		if(iReturn != NO_ERROR || msg_type != DECRYPT_RSP) {
			printf("Client: No data response received: %d\n", iReturn);
			sleep(1); continue;
		} else {
			ip_ptr = (uint8_t*)&senderIP;
			printf("Client: Received data response from %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
		}

		// print & write out result
		iReturn = extract_dec_rsp(&msg_ptr, &resBlockID, out_data, &length);
		if(iReturn != NO_ERROR) {
			printf("Client: An error occured during extracting the data packet: %d\n\n", iReturn);
			free_msg(&msg_ptr);
			sleep(1); continue;
		} else if(resBlockID != blockID) {
			printf("Client: Received blockID %d, expected blockID %d\n\n", resBlockID, blockID);
			free_msg(&msg_ptr);
			sleep(1); continue;
		} else {
			printf("        with length %d and block id %d\n", length, blockID);
			(*out_data)[length] = '\0';
			fwrite(*out_data, 1, length, fpOut);
			if(BLOCKLEN < 20)
				printf("Client: Received data: %s \n", *out_data);
			printf("\n");
			blockID++;
			newDataWanted = true;
			free_msg(&msg_ptr);
		}
	}

	/********** UNLOCK SERVER ************************************************/

   	// unlock server
   	iReturn = send_unlock_req(inet_network(argv[3]));
   	if(iReturn != NO_ERROR) {
   		printf("Client: Sending unlock request was not successful: %d\n", iReturn);
   	} else {
   		printf("Client: Sending unlock request worked\n");
   	}

   	// receive unlock response
   	tol_start_timeout(TOL_TIMEOUT_SECS);
   	iReturn = recv_msg(&msg_ptr, &senderIP);
	tol_stop_timeout();
    if (tol_is_timed_out()) {
        tol_reset_timeout();
        printf("Client: Timeout --> No Unlock Response received\n");
    } else {
    	msg_type = get_msg_type(&msg_ptr);
    	if(iReturn != NO_ERROR || msg_type != UNLOCK_RSP) {
    		printf("Client: No unlock response received: %d\n\n", iReturn);
    	} else {
    		ip_ptr = (uint8_t*)&senderIP;
    		printf("Client: Received unlock response from %d.%d.%d.%d\n\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    	}
    }

    /*****************************************************************************/

    // deinit socket
	deinit_client();

	// close filepointer
	fclose(fpIn);
	fclose(fpOut);

    printf("Client: Scrambling %s is done!\n", argv[1]);

    return 0;
}
