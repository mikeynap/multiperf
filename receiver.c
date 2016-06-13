/* 
 * Michael Napolitano (https://github.com/tldr193/) 2016(c)
 * Parts of run_subtest were lovingly borrowed from Christian Beier <dontmind@freeshell.org>.
 */



#ifndef __MINGW32__
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#ifdef __MACH__
#include "mach_gettime.h"
#endif
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <limits.h>
#include <signal.h>

#include "msock.h"
#include "stats.h"
#include "receiver.h"

int *sockets;
int restarted = 0;


void closeSockets(){
	int i = 0;
    while (1){
   	 if (sockets[i] > 0){
   		 close(sockets[i++]);
   	 }
   	 else break;
    }
	
	
}

void* ReturnWithError(char* errorMessage, int sock, char *recvBuf)
{
    fprintf(stderr, "%s\n", errorMessage);
    if(sock >= 0)
        close(sock);
    if(recvBuf)
        free(recvBuf);
    
    return NULL;
}

char* increment_address(const char* address_string, int by)
{
    // convert the input IP address to an integer
    in_addr_t address = inet_addr(address_string);

    // add one to the value (making sure to get the correct byte orders)
    address = ntohl(address);
    address += by;
    address = htonl(address);

    // pack the address into the struct inet_ntoa expects
    struct in_addr address_struct;
    address_struct.s_addr = address;

    // convert back to a string
    return inet_ntoa(address_struct);
}




int receiver(McastResult** test_results, int n_addr, int n_stream, int test_inc, char *start_addr, int start_port, int buf_len, int mbps, int timeout, int verbose){
	// test_inc = 0, just do one test
	// test_inc = n, test  1,1+n, 1+2n... n_addr addresses.
	int n_tests;	
	n_tests =  3 + n_addr/(test_inc);
	sockets = malloc(sizeof(int) * n_addr * n_stream + sizeof(int));
	sockets[n_addr * n_stream] = -1;
	int i, j, ind = 0;
	for (i = 0; i < n_addr; i++){
		char addr[25];
		
		for (j = 0; j < n_stream; j++){
			int ind = i * n_stream + j;
			char port[8];			
			sprintf(port, "%d", start_port + ind);
			sprintf(addr, "%s", increment_address(start_addr, i));
			sockets[ind] = mcast_recv_socket(addr, port, MULTICAST_SO_RCVBUF);
			if (sockets[ind] < 0) {
				exit(0);
			}
		} 
		
	}
	j = 1;
	//test level.
	n_tests = 0;
	int jitterSize = 0;
	
	while(j <= n_addr ){
		int tout = timeout;
		if (n_tests == 0 && restarted == 0) tout = 300;
		test_results[ind] =  run_tests(j, n_stream, start_addr, start_port, buf_len, &jitterSize, tout,verbose);
		if (test_results[ind] == NULL){
			break;
		}
		if (verbose == 1) disp_results(test_results[ind]);
		n_tests++;
		ind++;
		if (j == n_addr) break;
		if (j == 1 && test_inc != 1) j = 0;
		j += test_inc;
		if (j > n_addr) j = n_addr;
	}
	
	closeSockets();
	free(sockets);
	restarted = 1;
	return n_tests;
}


McastResult* run_tests(int n_addr, int n_stream, char *start_addr, int startPort, int bufLen, int *jitterSize, int timeout, int verbose){
	int i,j,rc;
	int n_thread = n_addr * n_stream;
	pthread_t thr[n_thread];
	mthread_data_t thr_data[n_thread];
    pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_attr_setstacksize(&thread_attr , PTHREAD_STACK_MIN );
	
	char *plural_s = "";
	if (n_stream > 1) plural_s = "s";
	char *plural_a = "";
	if (n_addr > 1) plural_a = "es";
	if (verbose == 1){
		printf("Receiving from %d Multicast Address%s (starting at %s:%d) over %d stream%s.\n", n_addr, plural_a, start_addr, startPort, n_stream, plural_s);
	}
	for (i = 0; i < n_addr; i++){
		for (j = 0; j < n_stream; j++){
			int ind = i * n_stream + j;
			sprintf(thr_data[ind].port, "%d", startPort + ind);
			sprintf(thr_data[ind].addr, "%s", increment_address(start_addr, i));
			thr_data[ind].bufLen = bufLen;
			thr_data[ind].sock = sockets[ind];
			thr_data[ind].jitterSize = *jitterSize;
			thr_data[ind].timeout = timeout;
			thr_data[ind].stat = createMcastStat(*jitterSize);
		} 
	}
	
	// loop again to ensure things start at same time.
	for (i = 0; i < n_thread; i++){
		if ((rc = pthread_create(&thr[i], /*&thread_attr*/NULL, run_subtest, &thr_data[i]))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return NULL;
		}
	} 
	
	int ntime = 0;
	for (i = 0; i < n_thread; i++){
		pthread_join(thr[i], NULL);
	}
	int nerr = 0;
	McastStat *res = createMcastStat(*jitterSize);
	for (i = 0; i < n_thread; i++){	
		McastStat *stat = thr_data[i].stat;
		if (thr_data[i].timeout == -1){
			nerr++;
		}
		else {
			ntime = stat->ttime;
			res->lost += stat->lost;
			res->rcvd += stat->rcvd;
			res->ttime += stat->ttime;
			res->bytes += stat->bytes;
			float rj = res->rollingJitter;
			for (j = 0; j < stat->used; j++){
				insertJitter(res, stat->jitters[j]);
			}
			res->rollingJitter = rj + stat->rollingJitter;		
			if (stat->used > *jitterSize){
				*jitterSize = stat->used;
			}
		}
		freeMcastStat(stat);
	}
	res->ttime += ntime * nerr;
	if (nerr >= (n_thread + 1) / 2 || computeBitrate(res) < 0.01){
		return (McastResult *)NULL;
	}	
	
	res->rollingJitter /= (n_thread - nerr);
	McastResult* r = computeMcastResult(res, n_addr, n_stream);
	freeMcastStat(res);
	return r;
}

void* run_subtest(void *arg){
	int     sock;                     /* Socket */
    
	mthread_data_t *args = (mthread_data_t *)arg;
    char recvBuf[args->bufLen*sizeof(char)];
    
    
    sock = args->sock;

    
    int rcvd=0;
    int lost=0;
    long nbytes = 0;
    int last_p=-1;
    int this_p = 0;
	float si = 0,ri = 0;
    struct timespec start;
	start.tv_sec = 0;
	struct timespec recv_time;
	struct timespec timeout;
    clock_gettime(CLOCK_REALTIME, &timeout);
	McastStat *mstat = args->stat;
    while(1) {
        int bytes = 0;    
        /* Receive a single datagram from the server */
        if ((bytes = recvfrom(sock, recvBuf, args->bufLen, 0, (struct sockaddr *)NULL, 0)) < 0){
            if (rcvd == 0){
			    clock_gettime(CLOCK_REALTIME, &recv_time);
                if (recv_time.tv_sec - timeout.tv_sec > args->timeout){
					args->timeout = -1;
					return NULL;
                }
				
				continue;
            }
            else{				
                break;
            }
        }
		
		nbytes += bytes;
		
	     
	    clock_gettime(CLOCK_REALTIME, &recv_time);
	
        ++rcvd;
        this_p = ntohl(*(int*)recvBuf);
    
        if(last_p >= 0) /* only check on the second and later runs */
        {
            if(this_p - last_p > 1)
                lost += this_p - (last_p+1);
        }
        last_p = this_p;
		/* Calculate Jitter */
		// get timestamp from UDP packet (RFC1889).
		
		
	    struct timeval tv;
		memcpy(&tv.tv_sec, recvBuf + 4,4);
		memcpy(&tv.tv_usec, recvBuf + 8,4);
		tv.tv_sec = ntohl(tv.tv_sec);
		tv.tv_usec = ntohl(tv.tv_usec);
	
		double sj = tv.tv_sec + tv.tv_usec/1e6;
	
	    double rj = recv_time.tv_sec + (recv_time.tv_nsec / 1e9);

		
	    if (start.tv_sec == 0){
	        clock_gettime(CLOCK_REALTIME, &start);
	    }
		else {
			double jitter = (rj - sj) - (ri - si);
			if (jitter < 0) jitter = -jitter;
			insertJitter(mstat, jitter);
		}
		si = sj;
		ri = rj;
		
	}
	
	struct timespec diff;
	diff.tv_sec = recv_time.tv_sec - start.tv_sec;
	diff.tv_nsec = recv_time.tv_nsec - start.tv_nsec;
    double elapsed_time = diff.tv_sec + (diff.tv_nsec / 1e9);
	
	mstat->rcvd = rcvd;
	mstat->lost = lost;
	mstat->ttime = elapsed_time;
	mstat->bytes = nbytes;
	//free(recvBuf);
	//close(sock);

	return NULL;
}
