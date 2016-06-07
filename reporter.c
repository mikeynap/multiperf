#include <string.h>
#include <stdio.h>
#include "stats.h"
#include "reporter.h"
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#define NFIELDS 13


Reporter* createReporter(char *address, int port, float bandwidth, int packet_size){
	Reporter *r = malloc(sizeof(Reporter));
	r->address = address;
	r->port = port;
	r->bitrate = bandwidth;
	r->packet_size = packet_size;
	r->nreporters = 0;
	r->nresults = 0;
	r->rsize = 100;
	r->results = calloc(r->rsize, sizeof(float*));
	int i = 0;
	for (; i < r->rsize; i++) r->results[i] = NULL;
	return r;
}
void* listen_for_results(void *args){
	Reporter *r = (Reporter *)args;
    struct sockaddr_in serv_addr, cli_addr;
	socklen_t clilen;
	int sockAck;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
    if (sockfd < 0) {
      perror("ERROR opening socket, will not receive reports.");
     return NULL;
    }
   
    /* Initialize socket structure */
    bzero((char *) &serv_addr, sizeof(serv_addr));
   
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(r->port);
   
    /* Now bind the host address using bind() call.*/
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR binding to socket, will not receive reports.");
     return NULL;
    }
     
    /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
    */
   
    listen(sockfd,10);
    clilen = sizeof(cli_addr);
	
   
    /* Accept actual connection from the client */
	pthread_t pthread_id;
	listener_thread_data datas[100];
	int thread_ind = 0;
    while (1) {
      sockAck = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
      if (sockAck < 0) {
        perror("ERROR on accept");
        return NULL;
      }
     datas[thread_ind].sock = sockAck;
     datas[thread_ind].reporter = r;
     pthread_create(&pthread_id, NULL, handle_reporter, (void *)&datas[thread_ind]);
     
      /* Create child process */
	
    } /* end of while */	
}

void* handle_reporter(void *arg){
	listener_thread_data *data = (listener_thread_data *)arg;
	int n = 0;
	char buffer[100000];
	int size = 100;
	int n_tests = 0;
	int k = 1;
	float **results = calloc(size, sizeof(float *));
	int result_ind = 0;
	while (k != 0){
  	  k = read(data->sock,buffer + n,10000);
  	  n += k;
	}
	buffer[n] = '\0';

	char *ptk = strtok(buffer, ",\r\n");
	while (ptk){
	    if (result_ind == 0){
		    if (n_tests == size){
			    results = realloc(results, sizeof(float *) * size * 2);
			    size *= 2;
		    }
		    results[n_tests] = calloc(NFIELDS + 1, sizeof(float));
	    }
	    if (result_ind == 2){
		    char buff[30];
		    strcpy(buff, ptk);
		    buff[strlen(ptk) - 1] = '\0';
	        results[n_tests][result_ind++] = atof(ptk);
  	
	    }
	    else if (result_ind == NFIELDS - 1) {
		    results[n_tests][result_ind] = atof(ptk);
		    n_tests++;
		    result_ind = 0;
	    }	
	    else {
	    	results[n_tests][result_ind++] = atof(ptk);
	    }
  
	    ptk = strtok(NULL, ",\r\n");
  		
	}	
	
	
	Reporter *r = data->reporter;
    if (n_tests < 1){
	    free(results);
	    return NULL;
    }
    pthread_mutex_lock(&r->lock);
    r->nreporters++;
    int ind = 0;
    for (n = 0; n < n_tests;n++){
	    ind = (int)results[n][0];
        if (ind >= r->rsize){
			float **newresults = calloc(r->rsize * 2, sizeof(float *));
			int z = 0;
			for (; z < r->rsize; z++){
				newresults[z] = r->results[z];
			}
			free(r->results);
			r->results = newresults;			
			for (z=r->rsize; z < r->rsize * 2; z++) {
				r->results[z] = NULL;
			}
		    r->rsize *= 2;
			
        }
	    if (r->results[ind] == NULL){
		    r->results[ind] = calloc(NFIELDS + 2, sizeof(float));
		    r->nresults++;
	    }
	    r->results[ind][0] = ind;
	    r->results[ind][1] = (int)results[n][1];
	    for (k = 2; k < NFIELDS; k++){
		    r->results[ind][k] += results[n][k];
	    }
		r->results[ind][NFIELDS] += 1;
    }
    pthread_mutex_unlock(&r->lock);
	return NULL;
}

void reporterListen(Reporter* r){
	pthread_create(&r->thread, NULL, listen_for_results, r);
	
}

void crunchReports(Reporter *r, FILE *out){
	if (r->nresults == 0) return;
	int i = 0, j = 0;
	fprintf(out, "Receivers,Bitrate,Packet Size,%s\n",  RESULT_HEADERS);
	for (; i < r->rsize; i++){
		if (!r->results[i]) continue;
		fprintf(out, "%d,%f,%d,%d,%d,",(int)r->results[i][NFIELDS], r->bitrate, r->packet_size, (int)r->results[i][0], (int)r->results[i][1]);
		for (j = 2; j < NFIELDS; j++){
			r->results[i][j] /= r->results[i][NFIELDS];
			fprintf(out, "%f%s", r->results[i][j], (j == NFIELDS - 1) ? "\n" : ",");
		}
	}
	
}

int sendall(int s, char *buf, int len)
{
    int total = 0;       // how many bytes we've sent
    int bytesleft = len; // how many we have left to send
    int n;

    while(total < len) {
       n = send(s, buf+total, bytesleft, 0);
       if (n == -1) { 
		   break; 
	   }
       total += n;
       bytesleft -= n;
    }

    return n==-1?-1:0; // return -1 on failure, 0 on success
} 


int reportResults(Reporter *r, McastResult **results, int n_tests, int json){
    struct sockaddr_in server;
    if (n_tests < 1) return 1;
    //Create socket
    r->sock = socket(AF_INET , SOCK_STREAM , 0);
    if (r->sock == -1)
    {
       printf("Could not create socket");
    }
    
    server.sin_addr.s_addr = inet_addr(r->address);
    server.sin_family = AF_INET;
    server.sin_port = htons( r->port );
 
    //Connect to remote server
    if (connect(r->sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
       perror("connect failed. Error");
       return 1;
    }
	int i = 1,slen;
	char *csv = result_to_csv(results[0]);
	slen = strlen(csv);
	char *message = calloc((n_tests + 3) * (slen + 10), sizeof(char));
	memcpy(message, csv, strlen(csv));
	message[slen] = '\n';
	int n = slen + 1;
	
	free(csv);
	for (; i < n_tests;i++){
		csv = result_to_csv(results[i]);
		slen = strlen(csv);
		memcpy(message + n, csv, strlen(csv));
		n+=slen;
		message[n] = '\n';
		n++;
		free(csv);
	}
	message[n] = '\0';
	
    printf("%s\n", message);
    if( sendall(r->sock , message , strlen(message)) < 0)
    {
		free(message);
       return 1;
    }
    free(message);
    
 
    close(r->sock);
    return 0;
}

void freeReporter(Reporter *r){
	int i = 0;
	for (; i < r->rsize; i++){
		if (r->results[i] != NULL){
			free(r->results[i]);
		}
	}
	free(r->results);
	free(r);
}

