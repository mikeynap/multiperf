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

