/**************************************************************
**  File        : main.c (client)                            **
**  Version     : 3.0                                        **
**  Created     : 03.05.2016                                 **
**  Last change : 25.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>

#include "main.h"
#include "timeoutlib.h"
#include "VS_LAB/clientAPI.h"
#include "VS_LAB/commonAPI.h"
#include "VS_LAB/Macros.h"

// Arguments:
// vslabc <infile> <outfile> <cid> <prio> <bcip>
int main(int argc, char **argv)
{
	// Variables
	uint8_t iNumServersBcRes = 0;
	uint8_t iNumServersLocked = 0;
	uint32_t iNumBlocks = 0;

	server_list srvList[MAX_NR_OF_SERVERS];
	job_list  j_anchor; j_anchor.length=0;
	job_list  *j_current, *j_nextOne;
	done_list d_anchor; d_anchor.length=0;
	done_list *d_current, *d_nextOne;

    uint8_t gp[2], **out_data, iReturn, *ip_ptr, error;
    uint16_t in_data[BLOCKLEN], resBlockID, dummy_port;
	uint32_t senderIP, length, i, j;
	out_data = malloc(sizeof(uint8_t*));
	bool newJob = false;
	bool leftOver = false;
	FILE *fpIn, *fpOut;
	msg msg;
	FID msg_type;

	// introduce yourself
	printf("VSLab3 client, build %s %s\n\n", __DATE__, __TIME__);

	// check command line parameters
	if(argc < 6) {
		printf("Missing arguments!\n");
		printf("Usage: main.c infile outfile cliend-ID priority broadcast-ip\n");
		return ERROR;
	} else {
		printf("----- Command Line Parameters -----\n");
		printf("Infile:      %s\n",argv[1]);
		printf("Outfile:     %s\n",argv[2]);
		printf("Client ID:   %s\n",argv[3]);
		printf("Client Prio: %s\n",argv[4]);
		printf("BroadcastIP: %s\n",argv[5]);
		printf("-----------------------------------\n\n");
	}

	// open in-filepointer
	fpIn=fopen(argv[1],"rb");
	if(!fpIn) {
		printf("Unable to open file!");
		return ERROR;
	}

	// open out-filepointer
	fpOut = fopen(argv[2], "w");

    // read in generator polynom
    length = fread(&gp,sizeof(uint8_t),2,fpIn);
    if(length != 2) {
    	printf("Client %d: Reading in gp was not possible\n",atoi(argv[3]));
    	return ERROR;
    }

    /********** SETUP JOB LIST ***********************************************/

    for(i=0; i<10000000; i++)
    {
    	length = fread(&in_data,sizeof(uint16_t),BLOCKLEN,fpIn);
		for(j=0; j<length; j++)
			in_data[j] = htons(in_data[j]); // correcting byteorder
    	if(length < 1) break;
    	j_current = &j_anchor;
    	while(1)
    	{
    		if(j_current->length == 0) { // First entry
    			memcpy(j_current->data,in_data,length*2);
    			j_current->length = length;
    			j_current->bid = i+1;
    			j_current->status = TODO;
    			j_current->next = NULL;
    			iNumBlocks++;
    			break;
    		}
    		else if(j_current->next == NULL) { // Next one is end of list
    			j_current->next = (job_list*) malloc(sizeof(job_list));
    			memcpy(((job_list*)j_current->next)->data,in_data,length*2);
    			((job_list*)j_current->next)->length = length;
    			((job_list*)j_current->next)->bid = i+1;
    			((job_list*)j_current->next)->status = TODO;
    			((job_list*)j_current->next)->next = NULL;
    			iNumBlocks++;
    			break;
    		}
    		else { // go through list
    			j_current = j_current->next;
    		}
    	}
    }

    // init socket
    init_client(atoi(argv[3]) /*clientID*/, atoi(argv[4]) /*prio*/, inet_network(argv[5]) /*broadcast*/);

    /********** SETUP SERVER LIST ********************************************/

    // init server list
bc:	for(i=0; i<MAX_NR_OF_SERVERS; i++)
    	srvList[i].status = NOT_USED;

    // send out broadcast
    iReturn = send_brdcst_req(SERVER_PORT);
    printf("Client %d: Sending out broadcast",atoi(argv[3]));
	if(iReturn != NO_ERROR) {
		printf(" failed: %d\n",iReturn);
		return ERROR;
	} else {
		printf(" worked (wait %d sec for answers)\n",BROADCAST_TIME);
	}

	// wait for answers
    tol_start_timeout(BROADCAST_TIME);
    while(1)
    {
     	iReturn = recv_msg(&msg, &senderIP, &dummy_port);

     	// timeout handler
     	if(tol_is_timed_out()) {
     		tol_stop_timeout();
     		tol_reset_timeout();
     		if(iNumServersBcRes > 0) {
     			printf("Client %d: Number of broadcast responses: %d\n\n",atoi(argv[3]),iNumServersBcRes);
     			break; // done --> now lock them
     		} else {
     			printf("Client %d: No server answered broadcast --> Quit\n",atoi(argv[3]));
     			return ERROR;
     		}
        }

     	// integrate server to list
     	ip_ptr = (uint8_t*)&senderIP;
    	msg_type = get_msg_type(&msg);
    	if(iReturn != NO_ERROR || msg_type != BROADCAST_RSP)
    	{ 	// drop not expected packet
    		if(msg_type != BROADCAST_REQ) // don't mention bc requests
    			printf("Client %d: Received something, but not expected broadcast response from %d.%d.%d.%d, msg_type %d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],msg_type);
    	}
    	else
    	{ 	// broadcast response
    		printf("Client %d: Received bc response from %d.%d.%d.%d",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    		for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    			if(srvList[i].ip == senderIP) {
    				printf(" --> Server's already in the server list");
    				break;
    			}
    			if(srvList[i].status == NOT_USED) {
    				printf(" --> Included to server list");
    				srvList[i].ip = senderIP;
    				srvList[i].status = BC_RES;
    				iNumServersBcRes++;
    				break;
    			}
    		}
    		printf("\n");
    	}
    	free_msg(&msg);
    }

    /********** LOCK SERVERS / SET GP ****************************************/

  	// send lock- / gp-request to the servers who answered bc
    for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    	if(srvList[i].status != BC_RES) continue;
    	iReturn = send_gp_req((gp[0] << 8) + gp[1], srvList[i].ip, SERVER_PORT);
    	ip_ptr = (uint8_t*)&srvList[i].ip;
    	if(iReturn != NO_ERROR) {
    		printf("Client %d: GP %#x sent out to %d.%d.%d.%d failed: %d\n",atoi(argv[3]),(gp[0] << 8) + gp[1],ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],iReturn);
    		return ERROR;
    	} else {
    		printf("Client %d: GP %#x sent out to %d.%d.%d.%d\n",atoi(argv[3]),(gp[0] << 8) + gp[1],ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    	}
    }

    // receive answers (hopefully gp-set-response)
    tol_start_timeout(GP_ANSWER_TIME);
    printf("Client %d: Wait at most %d sec for GP answers\n",atoi(argv[3]),GP_ANSWER_TIME);
    while(1)
    {
     	iReturn = recv_msg(&msg, &senderIP, &dummy_port);

     	// timeout handler
     	if(tol_is_timed_out()) {
     		tol_stop_timeout();
     		tol_reset_timeout();
    		if(iNumServersLocked > 0) {
    			printf("Client %d: Number of locked servers: %d\n\n",atoi(argv[3]),iNumServersLocked);
    			break;
    		} else {
    			printf("\nClient %d: All servers busy\n",atoi(argv[3]));
    			return ERROR;
    		}
        }

     	// check type of received packet
     	ip_ptr = (uint8_t*)&senderIP;
    	msg_type = get_msg_type(&msg);
    	if(iReturn != NO_ERROR || msg_type != GP_RSP)
    	{ 	// drop not expected packet
    		if(msg_type == ERROR_RSP) {
    			extract_error_rsp(&msg,&error,&resBlockID);
    			if(error == ERR_SERVERINUSE)
    				printf("Client %d: Server @ %d.%d.%d.%d is already locked\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    			else
    				printf("Client %d: Received something else, not expected gp response from %d.%d.%d.%d: %d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],iReturn);
    		}
    	}
    	else
    	{ 	// gp packet
    		printf("Client %d: Received gp response from %d.%d.%d.%d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    		for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    			if(srvList[i].ip == senderIP) {
    				srvList[i].status = IDLE;
    				iNumServersLocked++;
    				break;
    			}
    		}
    	}

    	free_msg(&msg);

    	if(iNumServersLocked == iNumServersBcRes) {
    		printf("Client %d: Locked all known servers: %d\n\n",atoi(argv[3]),iNumServersLocked);
			break;
    	}
    }

	/********** INITIAL DATA REQUEST TO ALL LOCKED SERVERS *******************/

    j_current = &j_anchor;
    for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    	if(srvList[i].status == IDLE) {
    		// This server need's something to do
    		for(j=0; j<iNumBlocks; j++) {
    			if(j_current->status == TODO) {
    				// This block needs someone who scrambles
    				ip_ptr = (uint8_t*)&srvList[i].ip;
    				printf("Client %d: Send data block %d to %d.%d.%d.%d ",atoi(argv[3]),j_current->bid,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    	    		iReturn = send_dec_req(j_current->bid, j_current->data, j_current->length, srvList[i].ip, SERVER_PORT);
    	    		if(iReturn != NO_ERROR) {
    	    			printf("failed\n");
    	    		} else {
    	    			printf("worked\n");
    	    		}
    	    		j_current->status = IN_WORK;
    	    		srvList[i].status = WORKING;
    	    		srvList[i].block_id = j_current->bid;
    	    		srvList[i].time = (int)time(NULL);
    	    		break;
    			}
    			else
    				j_current = (job_list*)j_current->next;
    		}
    	}
    }

    /********** GO TO "RECEIVE & GIVE NEW JOB ON ANSWER MODE" ****************/

    while(1)
    {
    	tol_start_timeout(TOL_TIMEOUT_SECS);
    	iReturn = recv_msg(&msg, &senderIP, &dummy_port);
    	tol_stop_timeout();

    	/********** Second Handler *********************************/

    	if(tol_is_timed_out()) {
    		tol_stop_timeout();
    		tol_reset_timeout();

    		// Check if there are still servers locked for us
    		if(!iNumServersLocked) {
    			printf("\nClient %d: All servers left --> Goto to BC / Locking state\n\n",atoi(argv[3]));
    			// Reset ServerList
    			iNumServersBcRes = 0;
    			for(i=0; i<MAX_NR_OF_SERVERS; i++)
    				srvList[i].ip = 0;
    			goto bc;
    		}

    		// Check if there's still something to/in work
    		j_current = &j_anchor;
			for(i=0; i<iNumBlocks; i++) {
				if(j_current->status != DONE)
					break;
				else
					j_current = (job_list*)j_current->next;
			}
			if(i == iNumBlocks) {
				printf("\nClient %d: All blocks have been scrambled!\n\n",atoi(argv[3]));
				break;
			}

			// Check if all servers are idle and we still have work for them
			for(i=0; i<MAX_NR_OF_SERVERS; i++)
				if(srvList[i].status == WORKING)
					break;
			j_current = &j_anchor;
			for(j=0; j<iNumBlocks; j++)
				if(j_current->status == IN_WORK)
					break;
				else
					j_current = (job_list*)j_current->next;
			if(i == MAX_NR_OF_SERVERS && j == iNumBlocks)
				leftOver = true;

    	 	// Check servers for timeouts
    		for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    			if((srvList[i].status == WORKING) && (((int)time(NULL))-srvList[i].time) > MAX_TIME_FOR_SERVER) {
    				// ~~~> TIMEOUT <~~~
    				ip_ptr = (uint8_t*)&srvList[i].ip;
    				printf("\nClient %d: Timeout for server %d.%d.%d.%d working on block %d after %d sec\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],srvList[i].block_id,MAX_TIME_FOR_SERVER);
    				// Reset block settings
    				j_current = &j_anchor;
    				for(j=0; j<iNumBlocks; j++) {
    					if(j_current->bid == srvList[i].block_id) {
    						j_current->status = TODO;
    						break;
    					}
    					else
    						j_current = (job_list*)j_current->next;
    				}
    				if(j == iNumBlocks)
    					printf("Resetting block to TODO-State did not work!\n");
    				// Reset server settings
    				srvList[i].status = NOT_USED;
    				srvList[i].block_id = 0;
    				srvList[i].time = 0;
    				iNumServersLocked--;
    			}
    		}

    		if(!leftOver) continue;
    	} /* second handler */

    	/***********************************************************/

    	// ### Special case: ###
    	// Detected timeout for server after all packets were sent out
    	// Send manualy (left over) data decrypt request to servers
    	if(leftOver) {
    		// Find an 'idle' server
			for(i=0; i<MAX_NR_OF_SERVERS; i++) {
				if(srvList[i].status == IDLE) {
					// Determine TODO-block
					j_current = &j_anchor;
					for(j=0; j<iNumBlocks; j++) {
						if(j_current->status == TODO) {
							ip_ptr = (uint8_t*)&srvList[i].ip;
							printf("\nClient %d: Sending (leftover) data block %d to %d.%d.%d.%d ",atoi(argv[3]),j_current->bid,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
							iReturn = send_dec_req(j_current->bid, j_current->data, j_current->length, srvList[i].ip, SERVER_PORT);
							if(iReturn != NO_ERROR) {
								printf("failed\n");
							} else {
								printf("worked\n");
							}
							j_current->status = IN_WORK;
							srvList[i].status = WORKING;
							srvList[i].block_id = j_current->bid;
							srvList[i].time = (int)time(NULL);
							break;
				    	}
						else {
							j_current = (job_list*)j_current->next;
						}
					}
				if(j == iNumBlocks)
					break; // --> All jobs have been considered
				}
			}
    		leftOver = false;
    		continue; // --> go to receive mode
    	}

    	/********** EXTRACT PACKET & SAVE IT ***********************/

		msg_type = get_msg_type(&msg);
		ip_ptr = (uint8_t*)&senderIP;
		if(iReturn != NO_ERROR || msg_type != DECRYPT_RSP) {
			// We received an UNLOCK
			if(msg_type == UNLOCK_RSP || msg_type == ERROR_RSP) {
				printf("\nClient %d: Received an unlock / error frame from %d.%d.%d.%d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
	    		for(i=0; i<MAX_NR_OF_SERVERS; i++) {
	    			if((srvList[i].ip == senderIP) && (srvList[i].status == WORKING || srvList[i].status == IDLE)) {
	    				printf("Client %d: Unlocked server %d.%d.%d.%d",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
	    				// Reset working state of the block
	    				if(srvList[i].status == WORKING) {
	    					printf(" working on block %d\n",srvList[i].block_id);
	    					j_current = &j_anchor;
	    					for(j=0; j<iNumBlocks; j++) {
	    						if(j_current->bid == srvList[i].block_id) {
	    							j_current->status = TODO;
	    							break;
	    						}
	    						else
	    							j_current = (job_list*)j_current->next;
	    					}
	    					if(j == iNumBlocks)
	    						printf("Resetting block to TODO-State did not work!\n");
	    				}
	    				else
	    					printf("\n");
	    				// Mark server as 'Not in use'
	    				srvList[i].status = NOT_USED;
	    				srvList[i].block_id = 0;
	    				srvList[i].time = 0;
	    				iNumServersLocked--;
	    				break;
	    			}
	    		}
			}
			// We received __________
			else {
				if(msg_type != BROADCAST_REQ) // don't mention bc requests
					printf("\nClient %d: Received something from %d.%d.%d.%d, msg_type: %d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],msg_type);
			}
		}
		else {
			// We received DATA
			printf("\nClient %d: Received data response from %d.%d.%d.%d ",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
			iReturn = extract_dec_rsp(&msg, &resBlockID, out_data, &length);
			if(iReturn != NO_ERROR) {
				printf("Client %d: An error occured during extracting the data packet: %d\n\n",atoi(argv[3]),iReturn);
			}
			else {
				// Mark block as is 'done'
				j_current = &j_anchor;
				for(i=0; i<iNumBlocks; i++) {
					if(j_current->bid == resBlockID) {
						if(j_current->status == IN_WORK) {
							printf("(length: %d, block id: %d)\n", length, resBlockID);
							// save received data
							d_current = &d_anchor;
							while(1) {
					    		if(d_current->length == 0) { // First entry
					    			memcpy(d_current->data,*out_data,length);
					    			d_current->length = length;
					    			d_current->bid = resBlockID;
					    			d_current->next = NULL;
					    			break;
					    		}
					    		else if(d_current->next == NULL) { // Next one is end of list
					    			d_current->next = (done_list*) malloc(sizeof(done_list));
					    			memcpy(((done_list*)d_current->next)->data,*out_data,length);
					    			((done_list*)d_current->next)->length = length;
					    			((done_list*)d_current->next)->bid = resBlockID;
					    			((done_list*)d_current->next)->next = NULL;
					    			break;
					    		}
					    		else { // go through list
					    			d_current = (done_list*)d_current->next;
					    		}
							}
							j_current->status = DONE;
							newJob = true;
							break;
						}
					}
					else
						j_current = (job_list*)j_current->next;
				}
				// Mark server as 'idle'
				for(i=0; i<MAX_NR_OF_SERVERS; i++) {
					if(srvList[i].ip == senderIP) {
						srvList[i].status = IDLE;
						srvList[i].block_id = 0;
						srvList[i].time = 0;
					}
				}
			}
			free_data(*out_data);
		}
		free_msg(&msg);

		/********** COMMISSON NEW JOB ****************************************/

		if(newJob) {
			// Find block who needs someone to scramble
			j_current = &j_anchor;
    		for(i=0; i<iNumBlocks; i++) {
    			if(j_current->status == TODO) {
    				ip_ptr = (uint8_t*)&senderIP;
    				if(i == 141)
    				{
    					i = 141;
    				}
		    		printf("Client %d: Sending data block %d to %d.%d.%d.%d ",atoi(argv[3]),j_current->bid,ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
		    	    iReturn = send_dec_req(j_current->bid, j_current->data, j_current->length, senderIP, SERVER_PORT);
    	    		if(iReturn != NO_ERROR) {
    	    			printf("failed\n");
    	    		} else {
    	    			printf("worked\n");
    	    		}
		    	    j_current->status = IN_WORK;
		    	    // Mark server as 'working'
		    	    for(j=0; j<MAX_NR_OF_SERVERS; j++) {
		    	    	if(srvList[j].ip == senderIP) {
		    	    		srvList[j].status = WORKING;
		    	    		srvList[j].block_id = j_current->bid;
		    	    		srvList[j].time = (int)time(NULL);
		    	    		break;
		    	    	}
		    	    }
		    	    break;
		    	}
		    	else
		    		j_current = (job_list*)j_current->next;
		    }
		    newJob = false;
		}
    } /* while(recv_mgs(...)) */

    /********** UNLOCK SERVERS ***********************************************/

    for(i=0; i<MAX_NR_OF_SERVERS; i++) {
    	if(srvList[i].status == IDLE) {
    		iReturn = send_unlock_req(srvList[i].ip, SERVER_PORT);
    		ip_ptr = (uint8_t*)&srvList[i].ip;
    	   	if(iReturn != NO_ERROR) {
    	   		printf("Client %d: Sending unlock request to %d.%d.%d.%d was not successful: %d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],iReturn);
    	   	} else {
    	   		printf("Client %d: Sent unlock request to %d.%d.%d.%d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
    	   	}
    	}
    }

    // receive unlock response
    printf("Client %d: Wait at most %d sec for unlock answers\n",atoi(argv[3]),UNLOCK_ANSWER_TIME);
    for(i=0; i<iNumServersLocked; i++) {
       	tol_start_timeout(UNLOCK_ANSWER_TIME);
       	iReturn = recv_msg(&msg, &senderIP, &dummy_port);
    	tol_stop_timeout();
        if(tol_is_timed_out()) {
            tol_reset_timeout();
            printf("Client %d: Unlock Timeout\n",atoi(argv[3]));
            break;
        } else {
        	msg_type = get_msg_type(&msg);
        	ip_ptr = (uint8_t*)&senderIP;
        	if(iReturn != NO_ERROR || msg_type != UNLOCK_RSP) {
        		printf("Client %d: Received something from %d.%d.%d.%d, msg_type = %d, but no unlock response: %d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0],msg_type,iReturn);
        	} else {
        		printf("Client %d: Received unlock response from %d.%d.%d.%d\n",atoi(argv[3]),ip_ptr[3],ip_ptr[2],ip_ptr[1],ip_ptr[0]);
        	}
        }
    } printf("\n");

    /*************************************************************************/

    // Write (unordered) received data to file
    printf("Client %d: Write out received data to %s\n\n",atoi(argv[3]),argv[2]);
    for(i=0; i<iNumBlocks; i++) {
    	d_current = &d_anchor;
    	for(j=0; j<iNumBlocks; j++) {
    		if(d_current->bid == i+1) {
    			fwrite(d_current->data, 1, d_current->length, fpOut);
    			break;
    		}
    		else {
    			d_current = (done_list*)d_current->next;
    		}
    	}
    }

    // Free job- & result-list
    d_current = &d_anchor; d_current = (done_list*)d_current->next;
    j_current = &j_anchor; j_current = (job_list*) j_current->next;
    for(i=0; i<iNumBlocks-1; i++) {
    	d_nextOne = (done_list*)d_current->next;
    	j_nextOne = (job_list*) j_current->next;
    	free(d_current);
    	free(j_current);
    	d_current = d_nextOne;
    	j_current = j_nextOne;
    }

    // deinit socket
	deinit_client();

	// close filepointer
	fclose(fpIn);
	fclose(fpOut);

    printf("Client %d: Scrambling %s is done!\n",atoi(argv[3]),argv[1]);

    return 0;
}
