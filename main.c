/*  perfabr - Performance Profiler for Multiple Multicast Streams. 
    Michael Napolitano, Cisco Systems, Inc. (c) 2016
   
    Usage Examples: 
	1)  Default: Test 1 address (231.1.1.1), over one port (13000)
	    trasmitting at 10mbps for 10 seconds:
	
		Receiver: perfabr
		Sender: perfabr -S
   
   
	2)  Test 10 simulatenous multicast addresses (231.2.2.1 -> 231.2.2.10),
	    over 5 ports each (231.2.2.1:13000-4 -> 231.2.2.10:13049)
	    with each of the 50 connections transmitting at 2mbps, for 8 seconds:

		Receiver: perfabr -R -a 10 -s 5 -m 231.2.2.1 
		Sender: perfabr -S -a 10 -s 5 -m 231.2.2.1 -b 2.0 -t 8 

	3)  Multitest: Do nine tests up to 80 addresses,
	    incrementing by 10 each "round" (1 addr, 10 addr... 80 addr)
		Over 5 streams, starting at port 13000 (default)
		each transmitting at 10mbps for 10 seconds each (default). 

		Receiver: perfabr -R -a 80 -s 5 
		Sender: perfabr -S -a 80 -s 5

	This will run 9 tests varying over the number of addresses in increments of 10
	and give a CSV report at the end.
		
*/
   

#ifndef __MINGW32__
#include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "sender.h"
#include "receiver.h"


int main(int argc, char **argv){
    extern char *optarg;
    extern int optind, optopt;
	
    int opt;
    enum { SENDER,RECEIVER } mode = RECEIVER;
	char *multicast_start = "231.1.1.1";
	int n_addr = 1, n_stream = 1, test_inc = 0, test_time = 10, start_port=13000, buf_len = 1470;
	float mbps = -1;
    while ((opt = getopt(argc, argv, "SRa:s:i:b:t:p:m:l:")) != -1) {
        switch (opt) {
		case 'S': 
			mode = SENDER; 
			break;
			
   		case 'R': 
			mode = RECEIVER; 
			break;
	
		case 'a':
			n_addr = atoi(optarg);
			break;
			
		case 's':
			n_stream = atoi(optarg);
			break;
		
		case 'i':
			test_inc = atoi(optarg);
			break;
			
		case 'b':
			mbps = atof(optarg);				
			break;
		
		case 't':
			test_time = atoi(optarg);
			break;
		
		case 'p':
			start_port = atoi(optarg);
			break;
			
		case 'l':
			buf_len = atoi(optarg);
			break;
			
		case 'm':
			multicast_start = optarg;	
			break;
	    case '?':
		    printf("unknown arg %c\n", optopt);
		    exit(EXIT_FAILURE);
		
		default:
        	fprintf(stderr, "Usage: %s [-S|-R] [options]\nTry `%s -h` for more information.", argv[0]);
			exit(EXIT_FAILURE);
        }
    }
	if (mode == RECEIVER){
		receiver(n_addr, n_stream, test_inc, multicast_start, start_port, buf_len,mbps);
	}
	else {
		if (mbps == -1) mbps = 10;
		sender(n_addr, n_stream, mbps, test_time, test_inc, multicast_start, start_port);
	}
	
}

