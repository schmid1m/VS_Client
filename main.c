/**************************************************************
**  File        : main.c (client)                            **
**  Version     : 0.1                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 03.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdio.h>
#include <string.h>

#include "api/Macros.h"
#include "api/clientAPI.h"
#include "api/commonAPI.h"

int main(int argc, char **argv)
{
	uint8_t gp[2], out_data[4], iReturn, *ip_ptr;
	uint16_t in_data[4], blockID = 0;
	uint32_t senderIP, length;
	FILE *fpIn, *fpOut;
	msg msg_ptr;
	FID msg_type;

	// introduce yourself
	printf("VSLab client, build %s %s\n", __DATE__, __TIME__);

	// check command line parameters
	if (argc < 4) {
		printf("Missing arguments!\n");
		printf("Usage: vslabc infile outfile ip\n");
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
    init_client(1 /*clientID*/, 1 /*priority*/, 0/*broadcast address*/);

    /*****************************************************************************/

	// read in generator polynom & send it out
	fread(&gp,sizeof(uint8_t),2,fpIn);
	iReturn = send_gp_req((gp[0] << 8) + gp[1], inet_network(argv[3]));
	if(iReturn != NO_ERROR) {
		printf("Client: GP (%#x) sent out failed: %d\n", (gp[0] << 8) + gp[1], iReturn);
		return -1;
	} else {
		printf("Client: GP (%#x) sent out worked\n", (gp[0] << 8) + gp[1]);
	}

	// receive gp-set-response
	iReturn = recv_msg(&msg_ptr, &senderIP);
	msg_type = get_msg_type(&msg_ptr);
	if(iReturn != NO_ERROR && msg_type == GP_RSP) {
		printf("Client: No gp response received: %d\n", iReturn);
		return -1;
	} else {
		ip_ptr = (uint8_t*)&senderIP;
		printf("Client: Received gp response from %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
		free_msg(&msg_ptr);
	}

	/*****************************************************************************/

	// read in data & send it out
	fread(&in_data,sizeof(uint16_t),sizeof(in_data)/sizeof(uint16_t),fpIn);
	for(int i=0; i<sizeof(in_data)/sizeof(uint16_t); i++) in_data[i] = htons(in_data[i]); // correcting the byteorder
	printf("Client: Read in data: "); for(int i=0; i<sizeof(in_data)/sizeof(uint16_t); i++) printf("%#x ", in_data[i]); printf("\n");
	iReturn = send_dec_req(blockID, in_data, sizeof(in_data)/sizeof(uint16_t), inet_network(argv[3]));
	if(iReturn != NO_ERROR) {
		printf("Client: Sending out data was not successful: %d\n", iReturn);
		return -1;
	} else {
		printf("Client: Sending out data worked\n");
	}

	// receive scrambled data
	iReturn = recv_msg(&msg_ptr, &senderIP);
	msg_type = get_msg_type(&msg_ptr);
	if(iReturn != NO_ERROR && msg_type == DECRYPT_RSP) {
		printf("Client: No data response received: %d\n", iReturn);
		return -1;
	} else {
		ip_ptr = (uint8_t*)&senderIP;
		printf("Client: Received data response from %d.%d.%d.%d\n", ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
		free_msg(&msg_ptr);
	}

	// print result
	iReturn = extract_dec_rsp(&msg_ptr, &blockID, (uint8_t**)&out_data, &length);
	if(iReturn != NO_ERROR) {
		printf("Client: An error occured during extracting the data packet: %d\n", iReturn);
		return -1;
	} else {
		printf("        with length %d and seq nr %d\n", length, blockID);
		printf("Client: Received data: "); for(int i=0; i<sizeof(out_data); i++) printf("%#x ", out_data[i]); printf("\n");
		free_msg(&msg_ptr);
	}

	/*****************************************************************************/

	// deinit socket
	deinit_client();

	// close filepointer
	fclose(fpIn);
	fclose(fpOut);

    printf("Client: Scrambling %s is done!", argv[1]);

    return 0;
}
