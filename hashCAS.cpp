#include "bnljThreads.h"
#include <atomic>

typedef struct _BUCKET
{
  struct _NODE *head;
  int count;
  std::atomic<uint> lock;
} BUCKET;
//Global variables
BUCKET *hashTable = NULL;
TUPLE bufR[DATA_SIZE];
TUPLE bufS[DATA_SIZE];
int eachThreadSuccess[THREAD_SIZE];
static constexpr uint Lockbit = 0x01;
static constexpr uint Unlockbit = 0x00;

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

bool lockFunction(std::atomic<uint> &bucketLock) {
  auto lock = bucketLock.load();
    if (lock == Lockbit) {
    usleep(1);
    return false;
  }
  return bucketLock.compare_exchange_strong(lock, Lockbit);
}

void casLock(std::atomic<uint> &bucketLock) {
  while(true) {
    if (lockFunction(bucketLock)) break;
  }
}

void casUnlock(std::atomic<uint> &bucketLock) {
  bucketLock.store(Unlockbit);
}

void insertHash(TUPLE tuple)
{
  int hashIndex = hashFunction(tuple.key);
  NODE *newNode = createNode(tuple);
  casLock(std::ref(hashTable[hashIndex].lock));
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
  casUnlock(std::ref(hashTable[hashIndex].lock));
  return;
}

TUPLE* searchHash(int key) {
  int hashIndex = hashFunction(key);
  TUPLE *tuple;
  NODE *node = hashTable[hashIndex].head;
  if (node == NULL) {
      return NULL;
  }
  while(node != NULL) {
      if (node -> tuple.key == key) {
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

void insertWorker(ARG *arg) {
  //printf("thread_id: %d start: %d, end: %d\n", arg -> thread_id, arg -> start, arg -> end);
  for (int i = arg -> start; i < arg -> end; i++) {
    insertHash(bufR[i]);
  }
}

void joinWorker(ARG *arg) {
  RESULT result;
  for (int i = arg -> start; i < arg -> end; i++) {
    TUPLE *r = searchHash(bufS[i].key);
    result.rkey = r -> key;
    result.rval = r -> val;
    result.skey = bufS[i].key;
    result.sval = bufS[i].val;
    eachThreadSuccess[arg->thread_id]++;
  }
  return;
}

void testHash() {
  int successes = 0;
  for (int i = 0; i < DATA_SIZE; i++) {
    int hashIndex = hashFunction(i);
    NODE* node = hashTable[hashIndex].head;
    while(node != NULL) {
      if (node -> tuple.key == i) {
        successes++;  
        break;
      }
      else if (node == NULL) {
        break;
      }
      node = node -> next;
    }
  }
  printf("INSERTION: %d Successes\n", successes);
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
  }
}

int main(void)
{
  int rfd;
  int sfd;
  int nr;
  int ns;
  int totalSuccess = 0;
  struct timeval begin, end;
  ARG args[THREAD_SIZE];
  std::vector<std::thread> writeThreads;
  std::vector<std::thread> readThreads;
  initHash();
  printf("----Insert Operation----\n");
  //Initialization of args for each thread
  for (int i = 0; i < THREAD_SIZE; i++) {
    args[i].start = DATA_SIZE / THREAD_SIZE * i;
    if (i != THREAD_SIZE - 1) args[i].end = DATA_SIZE / THREAD_SIZE *  (i + 1);
    else args[i].end = DATA_SIZE;
    args[i].thread_id = i;
  }

  rfd = open("R", O_RDONLY);
  if (rfd == -1) ERR;
  sfd = open("S", O_RDONLY);
  if (sfd == -1) ERR;
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
  //printHash();
  testHash();
  close(rfd);
  printf("----Join Operation----\n");
  //Read "S"
  while (true)
  {
    ns = read(sfd, bufS, DATA_SIZE * sizeof(TUPLE));
    if (ns == 0) break;
    else if (ns == -1) ERR;
  }
  for (int i = 0; i < THREAD_SIZE; i++) {
    readThreads.emplace_back(joinWorker, &args[i]);
  }
  for (auto &thread : readThreads) {
    if (thread.joinable()) thread.join();
  }
  gettimeofday(&end, NULL);
  printDiff(begin, end);
  for (int i = 0; i < THREAD_SIZE; i++) {
    totalSuccess += eachThreadSuccess[i];
  }
  printf("JOINING: %d Successes\n", totalSuccess);
  return 0;
}
