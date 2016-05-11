#ifndef RECEIVER_H 
#define RECEIVER_H 1

#include "stats.h"
#include <stdint.h>

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
	McastStat *stat;
} mthread_data_t;


int receiver(int n_addr, int n_stream, int test_inc, char *start_addr, int start_port, int buf_len, int mbps);


void* ReturnWithError(char* errorMessage, int sock, char *recvBuf);
void *run_subtest(void *arg);
int run_tests(int n_addr, int n_steram, char *start_addr, int startPort, int bufLen, McastStat *tres, int *socks, int jitterSize);
void disp_results(int n_addr, int n_stream, McastStat *stat);
void csv_results(int n_addr, int n_stream, McastStat *stat, char **output);
void print_free_csv_results(char **csv, int n_tests, FILE * fd);

char* increment_address(const char* address_string, int by);

#endif