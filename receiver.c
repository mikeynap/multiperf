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

#include "msock.h"
#include "stats.h"
#include "receiver.h"



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




int receiver(int n_addr, int n_stream, int test_inc, char *start_addr, int start_port, int buf_len, int mbps){
	// test_inc = 0, just do one test
	// test_inc = n, test  1,1+n, 1+2n... n_addr addresses.
	
	if (test_inc < 0) test_inc *= -1;
	if (test_inc > n_addr) test_inc = n_addr;
	
	int n_tests =  3 + n_addr/(test_inc + 1);
	char **csv = malloc(sizeof(char *) * n_tests);
	McastStat *test_results[n_tests];
	int j = 1;
	int ind = 0;
	//test level.
	if (test_inc == 0){
		j = n_addr;
		n_tests = 1;
	}
	while(j <= n_addr ){
		test_results[ind] = createMcastStat();
		int rc = run_tests(j, n_stream, start_addr, start_port, buf_len, test_results[ind]);
		if (rc == EXIT_FAILURE){
			freeMcastStat(test_results[ind]);
			test_results[ind] = NULL;
			continue;
		}
		disp_results(j, n_stream, test_results[ind]);
		csv_results(j, n_stream, test_results[ind], csv + ind);
		freeMcastStat(test_results[ind]);
		n_tests++;
		ind++;
		if (j == n_addr) break;
		if (j == 1 && test_inc != 1) j = 0;
		j += test_inc;
		if (j > n_addr) j = n_addr;
	}
	
	print_free_csv_results(csv, ind, stdout);
	return 0;
	
}

void print_free_csv_results(char **csv, int n_tests, FILE *fd){
	fprintf(fd, "Addresses, Streams, Packet Loss, Average Bitrate/Stream (mbps), Aggregate Bitrate (mbps), Rolling Jitter (s), MinJit, Q1Jit, MedJit, Q3Jit, MaxJit, StddevJit, MeanJit\n");
	int i =0;
	for (; i < n_tests; i++){
		fprintf(fd, "%s\n", csv[i]);
		free(csv[i]);
	}
	free(csv);
}

void csv_results(int n_addr, int n_stream, McastStat *stat, char **output){
	*output = malloc(sizeof(char) * 200);
	char *o = *output;
	McastJitterStat *j = stat->jstat;
	sprintf(o, "%d,%d,%f%%,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", n_addr, n_stream, computeLoss(stat) * 100, computeBitrate(stat), computeBitrate(stat) * n_addr * n_stream, 
													 stat->rollingJitter, j->min, j->q1, j->median, j->q3, j->max, j->stddev, j->mean);
	
	
}

void disp_results(int n_addr, int n_stream, McastStat *stat){
	printf("Packet Loss: %.3f%%, Average Bitrate/stream: %.3f mbps, Aggregate Bitrate: %.3f mbps\n", computeLoss(stat) * 100, computeBitrate(stat),computeBitrate(stat)*n_addr*n_stream);
	McastJitterStat *j = stat->jstat;
	printf("Jitter Stats: Rolling: %f, Min: %f, Q1: %f, Med:%f, Q3: %f, Max: %f, Stddev: %f, Mean: %f\n\n",stat->rollingJitter, j->min, j->q1, j->median, j->q3, j->max, j->stddev, j->mean);
}

int run_tests(int n_addr, int n_stream, char *start_addr, int startPort, int bufLen, McastStat *tres){
	int i,j,rc;
	int n_thread = n_addr * n_stream;
	pthread_t thr[n_thread];
	mthread_data_t thr_data[n_thread];
	char *plural_s = "";
	if (n_stream > 1) plural_s = "s";
	char *plural_a = "";
	if (n_addr > 1) plural_a = "es";
	
	printf("Receiving from %d Multicast Address%s (starting at %s:%d) over %d stream%s.\n", n_addr, plural_a, start_addr, startPort, n_stream, plural_s);
	for (i = 0; i < n_addr; i++){
		for (j = 0; j < n_stream; j++){
			int ind = i * n_stream + j;
			sprintf(thr_data[ind].port, "%d", startPort + ind);
			sprintf(thr_data[ind].addr, "%s", increment_address(start_addr, i));
			thr_data[ind].bufLen = bufLen;
		} 
		
	}
	
	// loop again to ensure things start at same time.
	for (i = 0; i < n_thread; i++){
		if ((rc = pthread_create(&thr[i], NULL, run_subtest, &thr_data[i]))) {
			fprintf(stderr, "error: pthread_create, rc: %d\n", rc);
			return EXIT_FAILURE;
		}
	} 
	
	for (i = 0; i < n_thread; i++){
		pthread_join(thr[i], (void **)&thr_data[i].stat);
		if (thr_data[i].stat == NULL){
			return EXIT_FAILURE;
		}
	}
	
	for (i = 0; i < n_thread; i++){	
		McastStat *stat = thr_data[i].stat;
		
		tres->rcvd += stat->rcvd;
		tres->lost += stat->lost;
		tres->ttime += stat->ttime;
		tres->bytes += stat->bytes;
		float rj = tres->rollingJitter;
		for (j = 0; j < stat->used; j++){
			insertJitter(tres, stat->jitters[j]);
		}
		tres->rollingJitter = rj + stat->rollingJitter;
		freeMcastStat(stat);
		
	}	
	tres->rollingJitter /= n_thread;
	computeMcastStat(tres);
	return 0;
}

void* run_subtest(void *arg){
	int     sock;                     /* Socket */
    
	mthread_data_t *args = (mthread_data_t *)arg;
    char *recvBuf = malloc(args->bufLen*sizeof(char));
    
    
    sock = mcast_recv_socket(args->addr, args->port, MULTICAST_SO_RCVBUF);
    if(sock < 0)
        pthread_exit(ReturnWithError("mcast_recv_socket() failed", sock, recvBuf));
    
    int rcvd=0;
    int lost=0;
    long nbytes = 0;
    int last_p=-1;
    int this_p = 0;
	float si = 0,ri = 0;
    struct timespec start;
	start.tv_sec = 0;
	struct timespec recv_time;
	McastStat *mstat = createMcastStat();
    while(1) {
        int bytes = 0;    
        /* Receive a single datagram from the server */
        if ((bytes = recvfrom(sock, recvBuf, args->bufLen, 0, (struct sockaddr *)NULL, 0)) < 0){
            if (rcvd == 0){
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
	free(recvBuf);
	close(sock);
	pthread_exit(mstat);
	return mstat;
}
