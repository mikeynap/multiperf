#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include "stats.h"



int compare_floats (const void *x, const void *y)
{
    float xx = *(float*)x, yy = *(float*)y;
    if (xx < yy) return -1;
    if (xx > yy) return  1;
    return 0;
}

McastStat* createMcastStat (int jitterStat){
	McastStat *j = malloc(sizeof(McastStat));
	j->used = 0;
	j->size = 1000;
	if (jitterStat > 0){
		j->size = jitterStat;
	}
	j->jitters = malloc(j->size * sizeof(float));
	j->rollingJitter = 0;
	j->rcvd = 0; j->lost = 0; j->ttime = 0.0; j->bytes = 0;
	return j;
}


void insertJitter(McastStat *j, float jitter){
	if (j->rollingJitter && jitter > j->rollingJitter * 10000) return;
	if (j->used == j->size){
		j->size *= 2;
		j->jitters = realloc(j->jitters, j->size * sizeof(float));
	}
	j->jitters[j->used++] = jitter;
	j->rollingJitter += (jitter - j->rollingJitter)/16.0;
	
}


float computeMedian(float *arr, int s, int e){
	int n = e - s;
	arr = arr + s;
	if (n % 2 == 0){
		return (arr[n/2] + arr[n/2 - 1])/2.0;
	}
	
	return arr[n/2];
}



McastResult* computeMcastResult(McastStat *j, int naddr, int nstreams){
	if (j->used == 0) return NULL;
	McastResult *js = malloc(sizeof(McastResult));
    double sum = 0;
    double sq_sum = 0;
	int i;
    for(i = 0; i < j->used; ++i) {
       sum += j->jitters[i];
       sq_sum += j->jitters[i] * j->jitters[i];
    }
    js->mean = sum / j->used;
    double variance = sq_sum / j->used - js->mean * js->mean;
    js->stddev = sqrt(variance);
	if (js->stddev != js->stddev){ // isNan
		js->stddev = 0;
	}
	
	qsort(j->jitters, j->used, sizeof(float), compare_floats);
	
	js->min = j->jitters[0];
	js->max = j->jitters[j->used - 1];
	
	js->median = computeMedian(j->jitters, 0, j->used);
	js->q1 = computeMedian(j->jitters, 0, j->used/2);
	js->q3 = computeMedian(j->jitters, j->used/2, j->used);
	js->addresses = naddr;
	js->streams = nstreams;
	js->rollingJitter = j->rollingJitter;
	js->loss = computeLoss(j);
	js->bitrate = computeBitrate(j);
	js->aggrBitrate = js->bitrate * naddr * nstreams;
	return js;
}

float computeBitrate(McastStat *s){
	return s->bytes/s->ttime/1.0e6 * 8;
}

float computeLoss(McastStat *s){
	if (s->lost + s->rcvd == 0) return 1.0;
	return (float)s->lost/(s->lost + s->rcvd) * 100;
}


void freeMcastStat(McastStat *j){
	free(j->jitters);
	free(j);
}



void print_results(McastResult **rs, int n_test, FILE *fd, int json){
	char *headers = RESULT_HEADERS;
	int i = 0;
	if (json == 1){
		fprintf(fd, "{\n");		
	} 
	else {
		fprintf(fd, "%s\n", headers);
	}
	
	for (i = 0; i < n_test; i++){
		char *comma = "";
		if (json == 1){
			char *jout = result_to_json(rs[i]);
			if (i != n_test - 1){
				comma = ",";
			}
			fprintf(fd, "%s%s\n", jout, comma);
		}
		else {
			char *rez = result_to_csv(rs[i]);
			fprintf(fd, "%s\n", rez);
			free(rez);
		}
	}
	if (json == 1) {
		fprintf(fd, "}\n");
	}
}

char* result_to_json(McastResult *r){
	char *json = malloc(sizeof(char) * 450);
	sprintf(json, "\"%d\"{ addresses: %d,streams:%d,packet_loss:%f,bitrate_per_stream:%f,aggregate_bitrate:%f,jitter_rolling:%f," 
			"jitter_min:%f,jitter_q1:%f,jitter_median:%f,jitter_q3:%f,jitter_max:%f,jitter_stddev:%f,jitter_mean:%f}", 
			r->addresses, r->addresses, r->streams, r->loss, r->bitrate, r->aggrBitrate, 
			r->rollingJitter, r->min, r->q1, r->median, r->q3, r->max, r->stddev, r->mean);
    return json;
	
	
	
}

char * result_to_csv(McastResult *r){
	char *csv = malloc(sizeof(char) * 200);
	sprintf(csv, "%d,%d,%f%%,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", r->addresses, r->streams, r->loss, r->bitrate, r->aggrBitrate, 
													 r->rollingJitter, r->min, r->q1, r->median, r->q3, r->max, r->stddev, r->mean);
	return csv;
}

void disp_results(McastResult *j){
	printf("Packet Loss: %.3f%%, Average Bitrate/stream: %.3f mbps, Aggregate Bitrate: %.3f mbps\n", j->loss, j->bitrate,j->aggrBitrate);
	printf("Jitter Stats: Rolling: %f, Min: %f, Q1: %f, Med:%f, Q3: %f, Max: %f, Stddev: %f, Mean: %f\n\n",j->rollingJitter, j->min, j->q1, j->median, j->q3, j->max, j->stddev, j->mean);
}

