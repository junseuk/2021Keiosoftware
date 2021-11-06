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
#include <pthread.h>
#define SZ_PAGE 10
#define NB_BUFR (SZ_PAGE * 2 / sizeof(TUPLE))
#define NB_BUFS (SZ_PAGE * 16 / sizeof(TUPLE))
#define THREAD_SIZE 2
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
  pthread_mutex_t lock;
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
  if (pthread_mutex_trylock(&(hashTable[hashIndex].lock)) == 0) {
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
    pthread_mutex_unlock(&(hashTable[hashIndex].lock));
  }
  //No node at hashIndex
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

void insertWorker(int t_id) {
  printf("insert worker thread %d\n", t_id);
  if (t_id == 0) {
    for (int i = 0; i < 50; i++) {
      printf("%d from %d\n", i, t_id);
      insertHash(bufR[i]);
    }
  }
  else {
    for (int i = 50; i < 100; i++) {
      printf("%d from %d\n", i, t_id);
      insertHash(bufR[i]);
    }
  }
}

void joinWorker(int t_id) {
  RESULT result;
  printf("join worker thread %d\n", t_id);
  // if (t_id == 0) {
  //   for (int i = 0; i < 50; i++) {
  //     printf("%d from %d\n", i, t_id);
  //     TUPLE* r = searchHash(bufS[i].key);
  //     if (r != NULL) {
  //       result.rkey = r -> key;
  //       result.rval = r -> val;
  //       result.skey = bufS[i].key;
  //       result.sval = bufS[i].val;
  //     }
  //   }
  // }
  // else {
  //   for (int i = 50; i < 100; i++) {
  //     printf("%d from %d\n", i, t_id);
  //     TUPLE* r = searchHash(bufS[i].key);
  //     if (r != NULL) {
  //       result.rkey = r -> key;
  //       result.rval = r -> val;
  //       result.skey = bufS[i].key;
  //       result.sval = bufS[i].val;
  //     }
  //   }
  // }
}

void printHash() {	
	for (int i = 0; i < SZ_PAGE; i++)
	{
		if (hashTable[i].count != 0)
		{
      NODE *node = hashTable[i].head;
      printf("----HASHTABLE[%d]----\n", i);
      for (int j = 0; j < hashTable[i].count; j++) {
        printf("key: %d value: %d\n", node -> tuple.key, node -> tuple.val);
        node = node -> next;
      }
		}
	}
}

void initHash() {
  hashTable = (BUCKET*) malloc(SZ_PAGE * sizeof(BUCKET));
  for (int i = 0; i < SZ_PAGE; i++) {
    hashTable[i].head = NULL;
    hashTable[i].count = 0;
    pthread_mutex_init(&(hashTable[i].lock), NULL);
  }
}

int main(void)
{
  int rfd;
  int sfd;
  int nr;
  int ns;
  int resultVal;
  RESULT result;
  struct timeval begin, end;
  std::vector<std::thread> threads;
  initHash();

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
  for (int i = 0; i < 2; i++) {
    threads.emplace_back(insertWorker, i);
  }
  for (auto &thread : threads) {
    if (thread.joinable()) thread.join();
    printf("FIRST JOIN DONE\n");
  }
  //Read "S"
  // while (true)
  // {
  //   ns = read(sfd, bufS, 100 * sizeof(TUPLE));
  //   if (ns == 0) break;
  //   else if (ns == -1) ERR;
  // }
  // for (int i = 0; i < 2; i++) {
  //   threads.emplace_back(joinWorker, i);
  // }
  // for (auto &thread : threads) {
  //   if (thread.joinable()) thread.join();
  //   printf("SECOND JOIN DONE\n");
  // }
  // Manually join
  // for (int i = 0; i < 100; i++) {
  //   TUPLE* r = searchHash(bufS[i].key);
  //   if (r != NULL) {
  //     result.rkey = r -> key;
  //     result.rval = r -> val;
  //     result.skey = bufS[i].key;
  //     result.sval = bufS[i].val;
  //     resultVal += 1;
  //     printf("%d %d %d %d\n", result.rkey, result.rval, result.skey, result.sval);
  //   }
  //   else {
  //     //When not found
  //   }
  // }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  printf("Result: %d Success(es)\n", resultVal);
  printHash();
  return 0;
}
