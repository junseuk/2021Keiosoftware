#include <assert.h>
#include <strings.h>
#include <math.h>
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
#include <time.h>

#define THREAD_SIZE 1000
#define DATA_SIZE 10000000
#define N 4

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

NODE *Root = NULL;

struct timeval
cur_time(void)
{
  struct timeval t;
  gettimeofday(&t, NULL);
  return t;
}

typedef struct _ARG
{
  int start;
  int end;
  int thread_id;
} ARG;

//Prototype
void print_tree_core(NODE *n);
void print_tree(NODE *node);
NODE *find_leaf(NODE *node, int key);
NODE *insert_in_leaf(NODE *leaf, int key, DATA *data);
NODE *alloc_leaf(NODE *parent);
void temp_split(TEMP *tempNode, NODE *leaf);
TEMP *create_temp(NODE *leaf, int key, NODE *pointer);
void insert_in_parent(NODE *left_child, int key, NODE *right_child);
void split(TEMP *tempnode, NODE *original_node);
void insert(int key, DATA *data);
void init_root(void);
int interactive();
void delete_key(int key, DATA *data);
void delete_entry(NODE *node, int key, DATA *data);
void delete_operation(NODE *node, int key);
int howManyChild(NODE *node);
void printDiff(struct timeval begin, struct timeval end);
NEIGHBOR findLeftOrRightNode(NODE *node, int key);
pthread_mutex_t giantlock;
/* 
** test type 1: keys in order
** test type 2: keys in reverse order
** test type 3: random keys
*/
void test(int test_type, int num_data);
int search(int key);

void print_tree_core(NODE *n)
{
  printf("[");
  for (int i = 0; i < n->nkey; i++)
  {
    if (!n->isLeaf)
      print_tree_core(n->chi[i]);
    printf("%d", n->key[i]);
    if (i != n->nkey - 1 && n->isLeaf)
      putchar(' ');
  }
  if (!n->isLeaf)
    print_tree_core(n->chi[n->nkey]);
  printf("]");
}

void print_tree(NODE *node)
{
  print_tree_core(node);
  printf("\n");
  fflush(stdout);
}

NODE *
find_leaf(NODE *node, int key)
{
  int kid;

  if (node->isLeaf)
    return node;
  for (kid = 0; kid < node->nkey; kid++)
  {
    if (key < node->key[kid])
      break;
  }

  return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
  int i;
  //If the key is the smallest
  if (key < leaf->key[0])
  {
    for (i = leaf->nkey; i > 0; i--)
    {
      leaf->chi[i] = leaf->chi[i - 1];
      leaf->key[i] = leaf->key[i - 1];
    }
    leaf->key[0] = key;
    leaf->chi[0] = (NODE *)data;
  }
  //If the key should be in the middle or the end
  else
  {
    for (i = 0; i < leaf->nkey; i++)
    {
      if (key < leaf->key[i])
        break;
    }
    for (int j = leaf->nkey; j > i; j--)
    {
      leaf->chi[j] = leaf->chi[j - 1];
      leaf->key[j] = leaf->key[j - 1];
    }
    //#1 Assignment
    leaf->chi[i] = (NODE *)data;
    leaf->key[i] = key;
  }
  leaf->nkey++;
  return leaf;
}

NODE *
alloc_leaf(NODE *parent)
{
  NODE *node;
  if (!(node = (NODE *)calloc(1, sizeof(NODE))))
    ERR;
  node->isLeaf = true;
  node->parent = parent;
  node->nkey = 0;

  return node;
}

void temp_split(TEMP *tempNode, NODE *leaf)
{
  NODE *new_node;
  if (!(new_node = (NODE *)calloc(1, sizeof(NODE))))
    ERR;
  //Erase all entries first
  int i;
  for (i = 0; i < N; i++)
  {
    if (i != N - 1)
    {
      leaf->key[i] = 0;
      leaf->nkey--;
    }
    leaf->chi[i] = 0;
  }
  //Create left_child
  for (i = 0; i < (N / 2) + 1; i++)
  {
    if (i != N / 2)
    {
      leaf->key[i] = tempNode->key[i];
      leaf->nkey++;
    }
    leaf->chi[i] = tempNode->chi[i];
  }
  //Create right_child
  int j = 0;
  for (i = (N / 2) + 1; i < N + 1; i++)
  {
    if (i != N)
    {
      new_node->key[j] = tempNode->key[i];
      new_node->nkey++;
    }
    new_node->chi[j] = tempNode->chi[i];
    tempNode->chi[i]->parent = new_node;
    j++;
  }
  int key = tempNode->key[N / 2];
  new_node->isLeaf = tempNode->isLeaf;
  new_node->parent = leaf->parent;
  free(tempNode);
  insert_in_parent(leaf, key, new_node);
  return;
}

TEMP *
create_temp(NODE *leaf, int key, NODE *pointer)
{
  TEMP *tempNode;
  if (!(tempNode = (TEMP *)calloc(1, sizeof(TEMP))))
    ERR;
  //Copy leaf to tempnode
  tempNode->isLeaf = leaf->isLeaf;
  tempNode->nkey = leaf->nkey;
  for (int i = 0; i < N - 1; i++)
  {
    tempNode->chi[i] = leaf->chi[i];
    tempNode->key[i] = leaf->key[i];
  }
  //Search the position
  int i;
  if (key < tempNode->key[0])
  {
    for (i = tempNode->nkey; i > 0; i--)
    {
      tempNode->chi[i] = tempNode->chi[i - 1];
      tempNode->key[i] = tempNode->key[i - 1];
    }
    tempNode->key[0] = key;
    tempNode->chi[0] = pointer;
  }
  else
  {
    for (i = 0; i < tempNode->nkey; i++)
    {
      if (key < tempNode->key[i])
        break;
    }
    for (int j = tempNode->nkey; j > i; j--)
    {
      tempNode->chi[j] = tempNode->chi[j - 1];
      tempNode->key[j] = tempNode->key[j - 1];
    }
  }
  tempNode->key[i] = key;
  tempNode->chi[i] = pointer;
  tempNode->chi[N] = leaf->chi[N - 1];
  tempNode->nkey++;
  return tempNode;
}

void insert_in_parent(NODE *left_child, int key, NODE *right_child)
{
  if (Root == left_child)
  {
    NODE *new_root = alloc_leaf(NULL);
    new_root->isLeaf = false;
    new_root->chi[0] = left_child;
    new_root->chi[1] = right_child;
    new_root->key[0] = key;
    new_root->nkey++;
    right_child->parent = new_root;
    left_child->parent = new_root;
    Root = new_root;
    return;
  }
  NODE *parent = left_child->parent;
  //Insert new child and key
  //If there is a space in parent
  if (parent->nkey < N - 1)
  {
    int i;
    //If the key is the smallest
    if (key < parent->key[0])
    {
      for (i = parent->nkey + 1; i > 0; i--)
      {
        if (i != parent->nkey + 1)
          parent->key[i] = parent->key[i - 1];
        parent->chi[i] = parent->chi[i - 1];
      }
      //Connect left_child
      parent->chi[0] = left_child;
    }
    //If the key should be in the middle or the end
    else
    {
      for (i = 0; i < parent->nkey; i++)
      {
        if (key < parent->key[i])
          break;
      }
      for (int j = parent->nkey; j > i; j--)
      {
        parent->key[j] = parent->key[j - 1];
      }
      for (int k = parent->nkey + 1; k > i + 1; k--)
      {
        parent->chi[k] = parent->chi[k - 1];
      }
    }
    //Connect right_child and insert key and increase nkey of parent
    parent->chi[i + 1] = right_child;
    parent->key[i] = key;
    parent->nkey++;
    return;
  }
  //If parent is full
  else
  {
    TEMP *tempNode = create_temp(parent, key, left_child);
    int i;
    //Find the position to insert right_child
    for (i = 0; i < N; i++)
    {
      if (left_child == parent->chi[i])
        break;
    }
    tempNode->chi[i + 1] = right_child;
    temp_split(tempNode, parent);
    return;
  }
  return;
}

void split(TEMP *tempnode, NODE *original_node)
{
  NODE *new_node;
  if (!(new_node = (NODE *)calloc(1, sizeof(NODE))))
    ERR;
  int i;
  int j = 0;

  for (i = 0; i < N / 2; i++)
  {
    original_node->key[i] = tempnode->key[i];
    original_node->chi[i] = tempnode->chi[i];
  }

  for (i = N / 2; i < N; i++)
  {
    //erase
    if (i < N - 1)
    {
      original_node->key[i] = 0;
      original_node->chi[i] = 0;
      original_node->nkey--;
    }
    //copy from tempnode
    new_node->key[j] = tempnode->key[i];
    new_node->chi[j] = tempnode->chi[i];
    new_node->nkey++;
    j++;
  }
  //change pointers in the end
  original_node->chi[N - 1] = new_node;
  new_node->chi[N - 1] = tempnode->chi[N];
  new_node->isLeaf = tempnode->isLeaf;
  new_node->parent = original_node->parent;
  free(tempnode);
  //Insert_in_parent
  insert_in_parent(original_node, new_node->key[0], new_node);
  return;
}

void insert(int key, DATA *data)
{
  NODE *leaf;
  //If tree is empty, create an empty leaf node which is root
  if (Root == NULL)
  {
    leaf = alloc_leaf(NULL);
    Root = leaf;
  }
  //Find the leaf node L
  else
  {
    leaf = find_leaf(Root, key);
  }
  //If L has less than n - 1 keys
  if (leaf->nkey < (N - 1))
  {
    insert_in_leaf(leaf, key, data);
  }
  else
  {
    TEMP *tempNode = create_temp(leaf, key, (NODE *)data);
    split(tempNode, leaf);
  }
  // printf("num of Child: %d\n", howManyChild(leaf));
  // printf("num of Child of root: %d\n", howManyChild(Root));
  return;
}

void init_root(void)
{
  Root = NULL;
}

int interactive()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

int search(int key)
{
  NODE *leaf;
  leaf = find_leaf(Root, key);
  for (int i = 0; i < N - 1; i++)
  {
    if (key == leaf->key[i])
      return 1;
  }
  return 0;
}

void test(int test_type, int num_data)
{
  int result = 0;
  switch (test_type)
  {
  case 1:
    printf("IN ORDER\n");
    for (int i = 1; i < num_data + 1; i++)
    {
      insert(i, NULL);
    }
    printf("INSERTION DONE\n");
    for (int i = 1; i < num_data + 1; i++)
    {
      if (!search(i))
        printf("FAIL TO FIND: %d\n", i);
      else
        result++;
    }
    break;

  case 2:
    printf("REVERSE ORDER\n");
    for (int i = num_data; i > 0; i--)
    {
      insert(i, NULL);
    }
    printf("INSERTION DONE\n");
    for (int i = num_data; i > 0; i--)
    {
      if (!search(i))
        printf("FAIL TO FIND: %d\n", i);
      else
        result++;
    }
    break;

  case 3:
  {
    printf("RANDOM ORDER\n");
    //Creating vector
    int len = num_data, i, r, temp;
    std::vector<int> num;
    for (i = 1; i < len + 1; i++)
    {
      num.push_back(i);
    }
    random_shuffle(num.begin(), num.end());
    for (i = 0; i < len; i++)
    {
      insert(num[i], NULL);
    }
    printf("INSERTION DONE\n");
    for (int i = 1; i < num_data + 1; i++)
    {
      if (!search(i))
        printf("FAIL TO FIND: %d\n", i);
      else
        result++;
    }
    break;
  }

  default:
    std::cout << "Wrong test type" << std::endl;
    break;
  }
  printf("%d SUCCESS\n", result);
  return;
}

void delete_operation(NODE *node, int key) {
  int pos;
  if (node->isLeaf) {
    for (int i = 0; i < N; i++) {
      if (node -> key[i] == key) {
        pos = i;
        node -> key[i] = 0;
        node -> chi[i] = 0;
        node -> nkey--;
        break;
      }
    }
    for (int j = pos; j < node->nkey + 1; j++) {
      if (j == node->nkey) {
        node->key[j] = 0;
        node->chi[j] = 0;
        break;
      }
      node->key[j] = node->key[j+1];
      node->chi[j] = node->chi[j+1];
    }
  }
}

NEIGHBOR findLeftOrRightNode(NODE *node, int key) {
  NEIGHBOR neighbor;
  if (node == Root) return neighbor;
  NODE *parent = node->parent;
  for(int i = 0; i < parent->nkey+1; i++) {
    if (parent->chi[i] == node && i != parent->nkey) {
      neighbor.parentIndex = i;
      neighbor.node = parent -> chi[i+1];
      neighbor.isRight = true;
      return neighbor;
    }
    else if(parent->chi[i] == node && i == parent->nkey) {
      neighbor.parentIndex = i - 1;
      neighbor.node = parent -> chi[i-1];
      neighbor.isRight = false;
      return neighbor;
    }
  }
  return neighbor;
}

void delete_entry(NODE *node, int key, DATA *data) {
  delete_operation(node, key);
  if (node == Root && node->nkey == 0) {
    printf("only child gonna be the new root\n");
    free(node);
    return;
  }
  else if (node->nkey < (N-1)/2) {
    printf("%p : lower than minimum keys\n", node);
    NEIGHBOR neighbor = findLeftOrRightNode(node, key);
    printf("Neighbor node: %p, parentIndex: %d, isRight: %d\n", neighbor.node, neighbor.parentIndex, neighbor.isRight);
    //Redistribution
    if (neighbor.node->nkey > (N-1)/2) {
      if (neighbor.isRight) {
        if(!node->isLeaf) {
          printf("INTERNAL RIGHT REDISTRIBUTION\n");
        }
        else {
          printf("RIGHT NEIGHBOR REDISTRIBUTION\n");
          // printf("parent Index: %d\n", node->parent->key[neighbor.parentIndex]);
          // printf("neighbor Index: %d\n", neighbor.node->key[0]);
          insert_in_leaf(node, neighbor.node->key[0], (DATA*) neighbor.node->chi[0]);
          printf("RIGHT NEIGHBOR DELETION\n");
          delete_operation(neighbor.node, neighbor.node->key[0]);
          node->parent->key[neighbor.parentIndex] = neighbor.node->key[0];
          printf("RIGHT REDISTRIBUTION DONE\n");
        }
      }
      else {
        if(!node->isLeaf) {
          printf("INTERNAL LEFT REDISTRIBUTION\n");
        }
        else {
          printf("LEFT NEIGHBOR REDISTRIBUTION\n");
          int i = neighbor.node->nkey - 1;
          printf("Index: %d\n", i);
          insert_in_leaf(node, neighbor.node->key[i], (DATA*) neighbor.node->chi[i]);
          delete_operation(neighbor.node, neighbor.node->key[i]);
          node->parent->key[neighbor.parentIndex] = node->key[0];
          printf("LEFT REDISTRIBUTION DONE\n");
        }
      }
    }
    //Merge
    else {
      printf("MERGE\n");
      if(!neighbor.isRight) {
        for(int i = 0; i < neighbor.node->nkey; i++) {
          insert_in_leaf(node, neighbor.node->key[i], (DATA*) neighbor.node->chi[i]);
        }
      }
      delete_entry(node->parent, node->parent->key[neighbor.parentIndex], (DATA*) node);
    }
  }
  return;
}

void delete_key(int key, DATA *data) {
  NODE *leaf;
  leaf = find_leaf(Root, key);
  delete_entry(leaf, key, data);
}

void insertWorker(ARG *arg) {
  //printf("thread_id: %d start: %d, end: %d\n", arg -> thread_id, arg -> start, arg -> end);
  pthread_mutex_lock(&giantlock);
  for (int i = arg -> start; i < arg -> end; i++) {
    //printf("%d",i);
    insert(i,NULL);
  }
  pthread_mutex_unlock(&giantlock);
}

void printDiff(struct timeval begin, struct timeval end)
{
  long diff;

  diff = (end.tv_sec - begin.tv_sec) * 1000 * 1000 + (end.tv_usec - begin.tv_usec);
  printf("Thread size: %d Data size: %d Diff: %ld us (%ld ms)\n",THREAD_SIZE, DATA_SIZE ,diff, diff / 1000);
}

int main(int argc, char *argv[])
{
  struct timeval begin, end;
  init_root();
  begin = cur_time();
  int result = 0;
  ARG args[THREAD_SIZE];
  std::vector<std::thread> insertThreads;
  pthread_mutex_init(&giantlock, NULL);

  for (int i = 0; i < THREAD_SIZE; i++) {
    args[i].start = DATA_SIZE / THREAD_SIZE * i;
    if (i != THREAD_SIZE - 1) args[i].end = DATA_SIZE / THREAD_SIZE *  (i + 1);
    else args[i].end = DATA_SIZE;
    args[i].thread_id = i;
  }

  for (int i = 0; i < THREAD_SIZE; i++) {
    insertThreads.emplace_back(insertWorker, &args[i]);
  }
  for (auto &thread : insertThreads) {
    if (thread.joinable()) thread.join();
  }
  end = cur_time();
  printDiff(begin, end);
  //print_tree(Root);

  for (int i = 0; i < DATA_SIZE; i++)
  {
    if (!search(i))
      printf("FAIL TO FIND: %d\n", i);
    else
      result++;
  }
  printf("%d success\n", result);
  //interactive mode
  // while(1) {
    // insert(interactive(), NULL);
    // print_tree(Root);
    // delete_key(interactive(), NULL);
    // print_tree(Root);
  // }
  

  return 0;
}