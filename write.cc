#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include "debug.h"
typedef struct _TUPLE
{
  int key;
  int val;
} TUPLE;

#define FILE "R"
#define DATA_SIZE 10000000

void writeToStorage()
{
  int fd;
  TUPLE t;
  int key = 0;

  fd = open(FILE, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if (fd == -1)
    ERR;
  for (int i = 0; i < DATA_SIZE; i++)
  {
    t.key = key++;
    t.val = rand() % 100;
    write(fd, &t, sizeof(t));
  }
  close(fd);

  TUPLE t2;
  int fd2;
  int key2 = 0;
  fd2 = open("S", O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if (fd2 == -1) ERR;
  for (int i = 0; i < DATA_SIZE; i++)
  {
    t2.key = key2++;
    t2.val = rand() % 100;
    write(fd2, &t2, sizeof(t2));
  }
  close(fd2);
}

void readFromStorage()
{
  //READ "R"
  int fd;
  TUPLE bufR[DATA_SIZE];
  int byte;

  fd = open("R", O_RDONLY);
  if (fd == -1)
  {
    ERR;
  }
  while (1)
  {
    byte = read(fd, bufR, DATA_SIZE * sizeof(TUPLE));
    if (byte == 0)
      break;
    else if (byte == -1)
      ERR;
  }
  for (int i=0;i<DATA_SIZE;i++)
  {
    printf("R -> key: %d\tvalue: %d\n", bufR[i].key, bufR[i].val);
  }
  close(fd);
  //READ "S"
  int fd2;
  TUPLE bufS[DATA_SIZE];
  int byte2;

  fd2 = open("S", O_RDONLY);
  if (fd2 == -1) ERR;
  while (1)
  {
    byte2 = read(fd2, bufS, DATA_SIZE * sizeof(TUPLE));
    if (byte2 == 0) break;
    else if (byte2 == -1) ERR;
  }
  for (int i=0;i<DATA_SIZE;i++)
  {
    printf("S -> key: %d\tvalue: %d\n", bufS[i].key, bufS[i].val);
  }
  close(fd2);
}

int main(int argc, char *argv[])
{
  writeToStorage();
  //readFromStorage();

  return 0;
}
