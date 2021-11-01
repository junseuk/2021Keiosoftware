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
using namespace std;
#define SZ_PAGE 4096
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))

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

typedef struct _BUCKET
{
  struct _NODE *head;
  int count;
} BUCKET;

typedef struct _NODE
{
  TUPLE tuple;
  struct _NODE *next;
} NODE;

//Global variables
BUCKET *hashTable = NULL;
TUPLE bufR[100];
TUPLE bufS[100];

int hashFunction(int key)
{
  return key % SZ_PAGE;
}

NODE *createNode(TUPLE tuple)
{
  NODE *newNode;
  newNode = (NODE *)malloc(sizeof(NODE));
  newNode -> tuple = tuple;
  newNode -> next = NULL;

  return newNode;
}

void insertHash(TUPLE tuple)
{
  int hashIndex = hashFunction(tuple.key);
  NODE *newNode = createNode(tuple);
  //No node at hashIndex
  if (hashTable[hashIndex].count == 0)
  {
    hashTable[hashIndex].head = newNode;
    hashTable[hashIndex].count = 1;
  }
  //If there are nodes at hashIndex
  else
  {
    newNode->next = hashTable[hashIndex].head;
    hashTable[hashIndex].head = newNode;
    hashTable[hashIndex].count += 1;
  }
  return;
}

TUPLE* searchHash(int key) {
    int searchCount = 0;
    int hashIndex = hashFunction(key);
    TUPLE* tuple;
    NODE* node = hashTable[hashIndex].head;
    if (node == NULL) {
        printf("[ Hash Search ] NOT FOUND\n");
        return NULL;
    }
    while(node != NULL) {
        searchCount++;
        if (node -> tuple.key == key) {
            // printf("[  Hash Search  ] FOUND! Key: %d Value: %d\n", node -> tuple.key, node -> tuple.val);
            // printf("[  Hash Search  ] Search Count: %d\n", searchCount);
            tuple = &(node -> tuple);
        }
        node = node -> next;
    }
    return tuple;
}

void printDiff(struct timeval begin, struct timeval end)
{
  long diff;

  diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  printf("Diff: %ld us (%ld ms)\n", diff, diff / 1000);
}

void joinOperation(int key) {
  RESULT result;
  TUPLE* r = searchHash(bufS[key].key);
  if (r != NULL) {
    result.rkey = r -> key;
    result.rval = r -> val;
    result.skey = bufS[key].key;
    result.sval = bufS[key].val;
    //printf("%d %d %d %d\n", result.rkey, result.rval, result.skey, result.sval);
  }
  return;
}

int main(void)
{
  int rfd;
  int sfd;
  int nr;
  int ns;
  int resultVal = 0;
  struct timeval begin, end;
  thread t[100];
  hashTable = (BUCKET*) malloc(SZ_PAGE*sizeof(BUCKET));

  rfd = open("R", O_RDONLY);
  if (rfd == -1) ERR;
  sfd = open("S", O_RDONLY);
  if (sfd == -1) ERR;

  gettimeofday(&begin, NULL);
  //Read "R"
  while (true)
  {
    nr = read(rfd, bufR, 100 * sizeof(TUPLE));
    if (nr == -1) ERR;
    else if (nr == 0) break;
  }
  //Create Hash table
  for (int i = 0; i < 100; i++)
  {
    insertHash(bufR[i]);
  }
  //Read "S"
  while (true)
  {
    ns = read(sfd, bufS, 100 * sizeof(TUPLE));
    if (ns == 0) break;
    else if (ns == -1) ERR;
  }
  //Joining operation using threads, C++ 98 doesn't support std::thread
  for (int i = 0; i < 100; i++) {
    t[i] = thread(joinOperation, bufS[i]);
  }
  for (int i = 0; i < 100; i++) {
    t[i].join();
  }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  printf("Result: %d Success(es)\n", resultVal);

  return 0;
}
