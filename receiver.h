#ifndef RECEIVER_H 
#define RECEIVER_H 1

#include "stats.h"
#include <stdint.h>
#include <stdio.h>

#define MULTICAST_SO_RCVBUF 208000

struct ntp_time_t {
    uint32_t second;
    uint32_t fraction;
};

typedef struct {
	char addr[16];
	char port[7];
	int bufLen;
	int jitterSize;
	int sock;
	int timeout;
	McastStat *stat;
} mthread_data_t;


int receiver(McastResult** test_results, int n_addr, int n_stream, int test_inc, char *start_addr, int start_port, int buf_len, int mbps, int timeout, int verbose);


void* ReturnWithError(char* errorMessage, int sock, char *recvBuf);
void *run_subtest(void *arg);

McastResult* run_tests(int n_addr, int n_steram, char *start_addr, int startPort, int bufLen, int *jitterSize, int timeout, int verbose);

char* increment_address(const char* address_string, int by);
int open_sockets(int,int,int,char*);
void close_sockets();
#endif