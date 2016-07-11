/**************************************************************
**  File        : main.h (client)                            **
**  Version     : 1.0                                        **
**  Created     : 17.05.2016                                 **
**  Last change : 18.05.2016                                 **
**  Project     : Verteilte Systeme Labor                    **
**************************************************************/

#ifndef _MAIN_H_
#define _MAIN_H_

#include <stdint.h>

// general
#define BLOCKLEN 				5000	// bytes
#define MAX_NR_OF_SERVERS		19

// timelimits for timouts
#define BROADCAST_TIME			3		// sec
#define GP_ANSWER_TIME			3		// sec
#define MAX_TIME_FOR_SERVER		10 		// sec
#define UNLOCK_ANSWER_TIME		3		// sec

// server status
#define NOT_USED    0		///< This array slot is not used
#define BC_RES		1		///< This server is not locked for us, but sent broadcast response
#define IDLE		2		///< This server is locked for us
#define WORKING		3		///< This server is currently scrambling for us

// job status
#define TODO		0		///< The block is still left to do
#define IN_WORK		1		///< The block is currently in work
#define DONE		2		///< the block das already been scrambled

typedef struct server_list
{
    uint32_t ip;      		///< The IP-Address of the server that is working for us
    uint8_t  status;		///< The status of the server that (might) work for us
    uint16_t block_id;		///< The block id the server is working on
    int      time;			///< Starting time of the job, needed for timeout mechanisms
} server_list;

typedef struct job_list
{
	uint16_t data[BLOCKLEN];		///< The data to be scrambled
	uint32_t length;				///< The amount in bytes of valid data
	uint16_t bid;					///< The block id of the data-batzen
	uint8_t  status;				///< Status of this block
	void*    next;					///< Pointer to the next data block
} job_list;

typedef struct done_list
{
	uint8_t  data[BLOCKLEN];		///< The data that has been scrambled
	uint32_t length;				///< The amount in bytes of valid data
	uint16_t bid;					///< The block id of the data-batzen
	void*    next;					///< Pointer to the next data block
} done_list;

#endif /* _MAIN_H_ */
