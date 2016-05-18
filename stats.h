#ifndef STATS_H
#define STATS_H 1
#define RESULT_HEADERS "Addresses,Streams,PacketLoss,AverageBitrate/Stream(mbps),AggregateBitrate(mbps),RollingJitter(s),MinJit,Q1Jit,MedJit,Q3Jit,MaxJit,StddevJit,MeanJit"

typedef struct {
	int addresses;
	int streams;
	
	float min;
	float q1;
	float median;
	float q3;
	float max;
	float rollingJitter;
	
	float stddev;
	float mean;
	
	float loss;
	float bitrate;
	float aggrBitrate;
	
	
	
} McastResult;

typedef struct  {
	long used;
	long size;
	float *jitters;
	float rollingJitter;
	
	long rcvd;
	long lost;
	
	double ttime;
		
	long bytes;	
			
} McastStat;

int compare_floats (const void *a, const void *b);
McastStat *createMcastStat(int jitterStat);


void insertJitter(McastStat *j, float jitter);

float computeBitrate(McastStat *s);
float computeLoss(McastStat *s);

float computeMedian(float *arr, int s, int e);

McastResult* computeMcastResult(McastStat *j, int naddr, int streams);
void freeMcastStat(McastStat *j);







void print_results(McastResult **rs, int n_test, FILE *fd, int json);
char* result_to_json(McastResult *r);

char * result_to_csv(McastResult *r);

void disp_results(McastResult *j);


#endif
