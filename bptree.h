#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <strings.h>
#include <math.h>
#include "debug.h"
#include <iostream>
#define MAX_OBJ (1000 * 1000)
#define N 3
using namespace std;

typedef struct _DATA
{
	int key;
	int val;
	struct _DATA *next;
} DATA;

typedef struct _NODE
{
	bool isLeaf;
	struct _NODE *chi[N];
	int key[N - 1];
	int nkey;
	struct _NODE *parent;
} NODE;

typedef struct _TEMP
{
	bool isLeaf;
	NODE *chi[N + 1]; // for internal split (for leaf, only N is enough)
	int key[N];		  // for leaf split
	int nkey;
} TEMP;

typedef struct _NEIGHBOR
{
	NODE *node;
	int parentIndex;
	bool isRight;
} NEIGHBOR;

DATA Head;
DATA *Tail;

NODE *Root = NULL;
