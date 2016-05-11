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
	j->jstat = NULL;
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

void computeMcastStat(McastStat *j){
	if (j->used == 0) return;
	j->jstat = malloc(sizeof(McastJitterStat));
	McastJitterStat *js = j->jstat;
    double sum = 0;
    double sq_sum = 0;
	int i;
    for(i = 0; i < j->used; ++i) {
       sum += j->jitters[i];
       sq_sum += j->jitters[i] * j->jitters[i];
    }
    js->mean = sum / j->used;
    double variance = sq_sum / j->used - js->mean * js->mean;
    js->stddev = sqrtf(variance);
	
	qsort(j->jitters, j->used, sizeof(float), compare_floats);
	
	js->min = j->jitters[0];
	js->max = j->jitters[j->used - 1];
	
	js->median = computeMedian(j->jitters, 0, j->used);
	js->q1 = computeMedian(j->jitters, 0, j->used/2);
	js->q3 = computeMedian(j->jitters, j->used/2, j->used);
	
	
	
}

float computeBitrate(McastStat *s){
	return s->bytes/s->ttime/1.0e6 * 8;
}

float computeLoss(McastStat *s){
	return (float)s->lost/(s->lost + s->rcvd);
}


void freeMcastStat(McastStat *j){
	free(j->jitters);
	if (j->jstat) free (j->jstat);
}

