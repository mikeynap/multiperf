#ifndef SENDER_H
#define SENDER_H 1

#include "receiver.h"

void reverse(char s[]);


void m_itoa(int n, char s[]);


int sender(int nAddr, int nPort, float bandwidth, int test_time, int test_inc, char *addr, int start_port);

#endif