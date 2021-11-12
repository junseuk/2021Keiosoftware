#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include "debug.h"
#include <thread>
#include <iostream>
#include <vector>

#define SZ_PAGE 4052
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))
#define BUCKET_SIZE 10000
#define THREAD_SIZE 10
#define DATA_SIZE 1000000
typedef struct _TUPLE
{
  int key;
  int val;
} TUPLE;

typedef struct _RESULT
{
  int rkey;
  int rval;
  int skey;
  int sval;
} RESULT;

typedef struct _NODE
{
  TUPLE tuple;
  struct _NODE *next;
} NODE;

typedef struct _ARG
{
  int start;
  int end;
  int thread_id;
} ARG;