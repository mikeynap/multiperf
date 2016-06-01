/*  multiperf - Performance Profiler for Multiple Multicast Streams. 
    Michael Napolitano, Cisco Systems, Inc. (c) 2016
   
    Usage Examples: 
	1)  Default: Test 1 address (231.1.1.1), over one port (13000)
	    trasmitting at 10mbps for 10 seconds:
	
		Receiver: multiperf
		Sender: multiperf -S
   
   
	2)  Test 10 simulatenous multicast addresses (231.2.2.1 -> 231.2.2.10),
	    over 5 ports each (231.2.2.1:13000-4 -> 231.2.2.10:13049)
	    with each of the 50 connections transmitting at 2mbps, for 8 seconds:

		Receiver: multiperf -R -a 10 -s 5 -m 231.2.2.1 
		Sender: multiperf -S -a 10 -s 5 -m 231.2.2.1 -b 2.0 -t 8 

	3)  Multitest: Do nine tests up to 80 addresses,
	    incrementing by 10 each "round" (1 addr, 10 addr... 80 addr)
		Over 5 streams, starting at port 13000 (default)
		each transmitting at 10mbps for 10 seconds each (default). 

		Receiver: multiperf -R -a 80 -s 5 -i 10 
		Sender: multiperf -S -a 80 -s 5 -i 10

	This will run 9 tests varying over the number of addresses in increments of 10
	and give a CSV report at the end.
		
*/
   

#ifndef __MINGW32__
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "sender.h"
#include "reporter.h"
#include "receiver.h"

FILE *output;
McastResult **results;
int json;
Reporter* reporter;

void sig_func(int sig){
	int n_tests = 0;
	while(results[n_tests] != NULL){
		n_tests++;
	}
	print_results(results, n_tests, output, json);
	if (results && reporter){
 		reportResults(reporter, results,n_tests, json);
 	}
	closeSockets();
	exit(0);
}


int main(int argc, char **argv){
    extern char *optarg;
    extern int optind, optopt;
	
    int opt;
    enum { SENDER,RECEIVER,LISTENER } mode = RECEIVER;
	char *multicast_start = "231.1.1.1";
	char report_addr[20];
	strcpy(report_addr, "0.0.0.0");
	int timeout = 20;
	output = stdout;
	int restart = 0;
	reporter = NULL;
	json = 0;
	int n_addr = 1, n_stream = 1, test_inc = 0, test_time = 10, start_port=13000, buf_len = 1470, ind=0;
	int report_port = 6300, packet_size = 1316;
	int verbose = 1;
	float mbps = -1;
	results = NULL;
    while ((opt = getopt(argc, argv, "SRLa:s:i:b:t:P:p:m:l:o:je:r:x:z")) != -1) {
		char *rport;
		int foundport = 0;
		int foundaddress = 0;
        switch (opt) {
		case 'S': 
			mode = SENDER; 
			break;
			
   		case 'R': 
			mode = RECEIVER; 
			break;
			
		case 'z':
			restart = 1;
			break;
			
		case 'L':
			mode = LISTENER;
			break;
	
		case 'a':
			n_addr = atoi(optarg);
			break;
			
		case 's':
			n_stream = atoi(optarg);
			break;
		
		case 'r':
		    rport = optarg;
			while ( *(rport++) != '\0') {
				if (*rport == ':'){
					rport++;
					foundport = 1;
					break;
				}
				if (*rport == '.'){
					foundaddress = 1;
				}
			}
			
			if (foundport) report_port = atoi(rport);
			if (foundaddress){ 
				strcpy(report_addr, optarg);
			}
			else report_port = atoi(optarg);
			break;
			
		case 'e':
			timeout = atoi(optarg);
			break;
		
		case 'i':
			test_inc = atoi(optarg);
			break;
			
		case 'b':
			for (; ind < strlen(optarg); ind++){
				if (optarg[ind] == '.') continue;
				if (optarg[ind] < '0' || optarg[ind] > '9'){
					optarg[ind] = '\0';
					break;
				} 
			}
			mbps = atof(optarg);				
			break;
		
		case 't':
			test_time = atoi(optarg);
			break;
		
		case 'P':
			start_port = atoi(optarg);
			break;
		
		case 'p':
			packet_size = atoi(optarg);
			break;
			
		case 'l':
			buf_len = atoi(optarg);
			break;
			
		case 'm':
			multicast_start = optarg;	
			break;
		
		case 'x':
			verbose = 0;
			break;
			
		case 'j':
			json = 1;	
			break;
		
	    case '?':
		    printf("unknown arg %c\n", optopt);
		    exit(EXIT_FAILURE);
			break;
		
		case 'h':
			printf("Help: ...\n");
			exit(0);
			break;
		
		case 'o':
			output = fopen(optarg, "w");			
			if (output == NULL){
				fprintf(stderr, "Cannot open output file for writing: %s. Using stdout instead.", optarg);
				output = stdout;
			}
			break;
		default:
        	fprintf(stderr, "Usage: %s [-S|-R] [options]\nTry `%s -h` for more information.", argv[0], argv[0]);
			exit(EXIT_FAILURE);
        }
    }
	
    signal(SIGINT,sig_func);
start:
	if (mbps == -1) mbps = 10;
	reporter = createReporter(report_addr,report_port, mbps, packet_size);
	
	if (mode == RECEIVER){
		if (test_inc < 0) test_inc *= -1;
		if (test_inc > n_addr || test_inc == 0) test_inc = n_addr;
		results = calloc((3 + n_addr/test_inc), sizeof(McastResult *));
		int n_tests = receiver(results, n_addr, n_stream, test_inc, multicast_start, start_port, buf_len, mbps, timeout, verbose);
		if (report_addr[0] != '0'){
	 		reportResults(reporter, results,n_tests, json);
			freeReporter(reporter);
		}
		print_results(results, n_tests, output, json);
		int i = 0;
		for (; i < n_tests; i++){
			free(results[i]);
		}
		free(results);
		if (restart == 1) {
			if (n_tests > 0){
				goto start;
			}
			return 1;
		}
		
	}
	else {
		reporterListen(reporter);
		sender(n_addr, n_stream, mbps, packet_size, test_time, test_inc, multicast_start, start_port, verbose);
		sleep(2);
		crunchReports(reporter, output);
		freeReporter(reporter);
	}
	return 0;
}

