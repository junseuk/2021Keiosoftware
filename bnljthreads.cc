#include "bnljthreads.h"

//Global variables
BUCKET *hashTable = NULL;
TUPLE bufR[DATA_SIZE];
TUPLE bufS[DATA_SIZE];
int successNum;
int failureNum;

int hashFunction(int key)
{
  return key % BUCKET_SIZE;
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
  if (pthread_mutex_lock(&(hashTable[hashIndex].lock)) == 0) {
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
  return;
}

TUPLE* searchHash(int key) {
    int hashIndex = hashFunction(key);
    TUPLE* tuple;
    NODE* node = hashTable[hashIndex].head;
    if (node == NULL) {
        // printf("[ Hash Search ] NOT FOUND\n");
        return NULL;
    }
    while(node != NULL) {
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

void joinOperation(TUPLE tuple) {
  RESULT *result;
  if (!(result = (RESULT *)calloc(1, sizeof(RESULT)))) ERR;
  TUPLE *r = searchHash(tuple.key);
  if (r != NULL) {
    result -> rkey = r -> key;
    result -> rval = r -> val;
    result -> skey = tuple.key;
    result -> sval = tuple.val;
    //printf("%d %d %d %d\n", result -> rkey, result -> rval, result -> skey, result -> sval);
    successNum++;
  }
  else failureNum++;
  free(result);
  return;
}

void insertWorker(ARG *arg) {
  printf("thread_id: %d start: %d, end: %d\n", arg -> thread_id, arg -> start, arg -> end);
  for (int i = arg -> start; i < arg -> end; i++) {
    insertHash(bufR[i]);
  }
}

void joinWorker(ARG *arg) {
  for (int i = arg -> start; i < arg -> end; i++) {
    joinOperation(bufS[i]);
  }
}

void printData() {
  int count = 0;
  for (int i = 0; i < DATA_SIZE; i++) {
    printf("key: %d, val: %d ", bufR[i].key, bufR[i].val);
    count++;
  }
}

void testHash() {
  int successes = 0;
  int failures = 0;
  for (int i = 0; i < DATA_SIZE; i++) {
    int hashIndex = hashFunction(i);
    NODE* node = hashTable[hashIndex].head;
    while(node != NULL) {
      if (node -> tuple.key == i) {
        successes++;  
        break;
      }
      else if (node == NULL) {
        failures++;
        break;
      }
      node = node -> next;
    }
  }
  printf("%d Successes, %d Failures\n", successes, failures);
  return;
}

void printHash() {	
	for (int i = 0; i < BUCKET_SIZE; i++)
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
  hashTable = (BUCKET*) malloc(BUCKET_SIZE * sizeof(BUCKET));
  for (int i = 0; i < BUCKET_SIZE; i++) {
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
  struct timeval begin, end;
  ARG args[THREAD_SIZE];
  std::vector<std::thread> writeThreads;
  std::vector<std::thread> readThreads;
  initHash();
  //Initialization of args for each thread
  for (int i = 0; i < THREAD_SIZE; i++) {
    args[i].start = DATA_SIZE / THREAD_SIZE * i;
    if (i != THREAD_SIZE - 1) args[i].end = DATA_SIZE / THREAD_SIZE *  (i + 1);
    else args[i].end = DATA_SIZE;
    args[i].thread_id = i;
    //printf("thread_id: %d start: %d, end: %d\n", args[i].thread_id, args[i].start, args[i].end);
  }

  rfd = open("R", O_RDONLY);
  if (rfd == -1) ERR;
  // sfd = open("S", O_RDONLY);
  // if (sfd == -1) ERR;
  gettimeofday(&begin, NULL);
  //Read "R"
  while (true)
  {
    nr = read(rfd, bufR, DATA_SIZE * sizeof(TUPLE));
    if (nr == -1) ERR;
    else if (nr == 0) break;
  }
  for (int i = 0; i < THREAD_SIZE; i++) {
    writeThreads.emplace_back(insertWorker, &args[i]);
  }
  for (auto &thread : writeThreads) {
    if (thread.joinable()) thread.join();
  }
  // printData();
  //printHash();
  testHash();
  // printf("----Join Operation----\n");
  close(rfd);
  //Read "S"
  // while (true)
  // {
  //   ns = read(sfd, bufS, DATA_SIZE * sizeof(TUPLE));
  //   if (ns == 0) break;
  //   else if (ns == -1) ERR;
  // }
  // for (int i = 0; i < THREAD_SIZE; i++) {
  //   readThreads.emplace_back(joinWorker, &args[i]);
  // }
  // for (auto &thread : readThreads) {
  //   if (thread.joinable()) thread.join();
  // }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  // printf("Result: %d Successes %d Failures\n", successNum, failureNum);
  return 0;
}
