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

void writeToStorage(const int max)
{
  int fd;
  TUPLE t;
  int key = 0;

  fd = open(FILE, O_WRONLY | O_TRUNC | O_CREAT, 0644);
  if (fd == -1)
    ERR;
  for (int i = 0; i < max; i++)
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
  for (int i = 0; i < max; i++)
  {
    t2.key = key2++;
    t2.val = rand() % 100;
    write(fd2, &t2, sizeof(t2));
  }
  close(fd2);
}

void readFromStorage(int max)
{
  //READ "R"
  int fd;
  TUPLE bufR[max];
  int byte;

  fd = open("R", O_RDONLY);
  if (fd == -1)
  {
    ERR;
  }
  while (1)
  {
    byte = read(fd, bufR, max * sizeof(TUPLE));
    if (byte == 0)
      break;
    else if (byte == -1)
      ERR;
  }
  for (int i=0;i<max;i++)
  {
    printf("R -> key: %d\tvalue: %d\n", bufR[i].key, bufR[i].val);
  }
  close(fd);
  //READ "S"
  int fd2;
  TUPLE bufS[max];
  int byte2;

  fd2 = open("S", O_RDONLY);
  if (fd2 == -1) ERR;
  while (1)
  {
    byte2 = read(fd2, bufS, max * sizeof(TUPLE));
    if (byte2 == 0) break;
    else if (byte2 == -1) ERR;
  }
  for (int i=0;i<max;i++)
  {
    printf("S -> key: %d\tvalue: %d\n", bufS[i].key, bufS[i].val);
  }
  close(fd2);
}

int main(int argc, char *argv[])
{
  int max = 100000;

  writeToStorage(max);
  //readFromStorage(max);

  return 0;
}
