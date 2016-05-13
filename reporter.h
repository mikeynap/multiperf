#ifndef REPORTER_H
#define REPORTER_H 1

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include "stats.h"


typedef enum { 
	SENDER,
	RECEIVER
} Reporter_t;

typedef struct {
	char *address;
	int port;
	int sock;
	pthread_t thread;
	pthread_mutex_t lock;
	float **results;
	int nresults;
	int nreporters;
	int rsize;
	
} Reporter;

typedef struct {
	int sock;
	pthread_t thread_id;
	Reporter *reporter;
} listener_thread_data;


Reporter* createReporter(char *address, int port); //, Reporter_t type );
int reportResults(Reporter *r, McastResult **results, int n_tests, int json);
void* listen_for_results(void *args);
void crunchReports(Reporter *r, FILE *out);

void reporterListen(Reporter* r);
void freeReporter(Reporter *r);
void* handle_reporter(void *arg);


#endif