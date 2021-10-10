#include "bptree.h"
#include <vector>
#include <sys/time.h>

struct timeval
cur_time(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t;
}

void
print_tree_core(NODE *n)
{
	printf("["); 
	for (int i = 0; i < n->nkey; i++) {
		if (!n->isLeaf) print_tree_core(n->chi[i]); 
		printf("%d", n->key[i]); 
		if (i != n->nkey-1 && n->isLeaf) putchar(' ');
	}
	if (!n->isLeaf) print_tree_core(n->chi[n->nkey]);
	printf("]");
}

void
print_tree(NODE *node)
{
	print_tree_core(node);
	printf("\n"); fflush(stdout);
}


NODE *
find_leaf(NODE *node, int key)
{
	int kid;

	if (node->isLeaf) return node;
	for (kid = 0; kid < node->nkey; kid++) {
		if (key < node->key[kid]) break;
	}

	return find_leaf(node->chi[kid], key);
}

NODE *
insert_in_leaf(NODE *leaf, int key, DATA *data)
{
	int i;
  //If the key is the smallest
	if (key < leaf->key[0]) {
		for (i = leaf->nkey; i > 0; i--) {
			leaf->chi[i] = leaf->chi[i-1];
			leaf->key[i] = leaf->key[i-1];
		}
		leaf->key[0] = key;
		leaf->chi[0] = (NODE *)data;
	}
  //If the key should be in the middle or the end
	else {
		for (i = 0; i < leaf->nkey; i++) {
			if (key < leaf->key[i]) break;
		}
		for (int j = leaf->nkey; j > i; j--) {		
			leaf->chi[j] = leaf->chi[j-1] ;
			leaf->key[j] = leaf->key[j-1] ;
		} 
    //#1 Assignment
    leaf -> chi[i] = (NODE *) data;
    leaf -> key[i] = key;
	}
  leaf -> nkey++;
	return leaf;
}

NODE *
alloc_leaf(NODE *parent)
{
	NODE *node;
	if (!(node = (NODE *)calloc(1, sizeof(NODE)))) ERR;
	node->isLeaf = true;
	node->parent = parent;
	node->nkey = 0;

	return node;
}

void split(TEMP *tempnode, NODE *original_node) {
  printf("SPLIT!\n");
  NODE *new_node = if((NODE*) calloc(1, sizeof(NODE))) ERR;
  int i;
  int j = 0;
  for (i = N/2; i < N ; i++) {
      if (i == N - 1) {
        new_node -> key[j] = tempnode -> key[i];
        new_node -> chi[j] = tempnode ->chi[i];
        new_node -> nkey++;
      }
      else {
        //erase
        original_node -> key[i] = 0;
        original_node -> chi[i] = 0;
        original_node -> nkey--;
        //copy from tempnode
        new_node -> key[j] = tempnode -> key[i];
        new_node -> chi[j] = tempnode ->chi[i];
        new_node -> nkey++;
        j++;
      }
    }

  //change pointers in the end
  original_node -> chi[N-1] = new_node;
  new_node -> chi[N-1] = tempnode -> chi[N];
  new_node -> parent = orignal_node -> parent;
  free(tempnode);
  return;
}


void 
insert(int key, DATA *data)
{
	NODE *leaf;

	if (Root == NULL) {
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
	else {
	leaf = find_leaf(Root, key);
	}
	if (leaf->nkey < (N-1)) {
		insert_in_leaf(leaf, key, data);
	}
	else { // split
    TEMP *tempNode;
    if (!(tempNode = (TEMP *) calloc(1, sizeof(TEMP)))) ERR;
    //Copy leaf to tempnode
    for (int i = 0; i < N - 1; i++) {
      tempNode -> chi[i] = leaf -> chi[i];
      tempNode -> key[i] = leaf -> key[i];
    }
    tempNode -> isLeaf = leaf -> isLeaf;
    tempNode -> nkey = leaf -> nkey;
    tempNode -> chi[N] = leaf -> chi[N-1];
    printf("COPYING LEAF\n");
    printf("%d, %d, %d, %d\n", tempNode -> key[0], tempNode -> key[1], tempNode -> key[2], tempNode -> key[3]);
    int i;
    if (key < tempNode->key[0]) {
      for (i = tempNode->nkey; i > 0; i--) {
        tempNode->chi[i] = tempNode->chi[i-1];
        tempNode->key[i] = tempNode->key[i-1];
      }
      tempNode->key[0] = key;
      tempNode->chi[0] = (NODE *)data;
    }
    else {
      for (i = 0; i < tempNode->nkey; i++) {
        if (key < tempNode->key[i]) break;
      }
      for (int j = tempNode->nkey; j > i; j--) {	
        printf("j: %d i: %d\n", j, i);
        tempNode->chi[j] = tempNode->chi[j-1];
        tempNode->key[j] = tempNode->key[j-1];
      } 
    }
    tempNode -> key[i] = key;
    tempNode -> chi[i] = (NODE *) data;
    tempNode -> nkey++;
    split(tempNode);
	}
}

void
init_root(void)
{
	Root = NULL;
}

int 
interactive()
{
  int key;

  std::cout << "Key: ";
  std::cin >> key;

  return key;
}

int
main(int argc, char *argv[])
{
  printf("%d," 3/2);
  struct timeval begin, end;

	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
  while (true) {
		insert(interactive(), NULL);
    print_tree(Root);
  }
	end = cur_time();

	return 0;
}
