#ifndef STATS_H
#define STATS_H 1

typedef struct {
	float min;
	float q1;
	float median;
	float q3;
	float max;
	
	float stddev;
	float mean;
	
} McastJitterStat;

typedef struct  {
	long used;
	long size;
	float *jitters;
	float rollingJitter;
	
	long rcvd;
	long lost;
	
	double ttime;
		
	long bytes;	
	McastJitterStat *jstat;
		
} McastStat;

int compare_floats (const void *a, const void *b);
McastStat *createMcastStat();


inline void insertJitter(McastStat *j, float jitter);

float computeBitrate(McastStat *s);
float computeLoss(McastStat *s);

float computeMedian(float *arr, int s, int e);

void computeMcastStat(McastStat *j);
void freeMcastStat(McastStat *j);
#endif
