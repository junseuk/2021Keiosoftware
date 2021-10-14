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

TEMP* 
create_temp(NODE *leaf, int key, NODE *pointer) 
{
  TEMP *tempNode;
  if (!(tempNode = (TEMP *) calloc(1, sizeof(TEMP)))) ERR;
  //Copy leaf to tempnode
  tempNode -> isLeaf = leaf -> isLeaf;
  tempNode -> nkey = leaf -> nkey;
  for (int i = 0; i < N - 1; i++) {
    tempNode -> chi[i] = leaf -> chi[i];
    tempNode -> key[i] = leaf -> key[i];
  }
  tempNode -> chi[N-1] = leaf -> chi[N-1];
  //Search the position
  int i;
  if (key < tempNode->key[0]) {
    for (i = tempNode->nkey; i > 0; i--) {
      tempNode->chi[i] = tempNode->chi[i-1];
      tempNode->key[i] = tempNode->key[i-1];
    }
    tempNode->key[0] = key;
    tempNode->chi[0] = pointer;
  }
  else {
    for (i = 0; i < tempNode->nkey; i++) {
      if (key < tempNode->key[i]) break;
    }
    for (int j = tempNode->nkey; j > i; j--) {	
      tempNode->chi[j] = tempNode->chi[j-1];
      tempNode->key[j] = tempNode->key[j-1];
    } 
  }
  tempNode -> key[i] = key;
  tempNode -> chi[i] = pointer;
  tempNode -> nkey++;
  return tempNode;
}

void 
insert_in_parent(NODE* left_child, int key, NODE* right_child) 
{
  printf("inside insert_in_parent\n");
  if (Root == left_child) {
    printf("New root\n");
    NODE *new_root = alloc_leaf(NULL);
    new_root -> isLeaf = false;
    new_root -> chi[0] = left_child;
    new_root -> chi[1] = right_child;
    new_root -> key[0] = key;
    new_root -> nkey++;
    right_child -> parent = new_root;
    left_child -> parent = new_root;
    Root = new_root;
    return;
  }
  NODE *parent = left_child -> parent;
  printf("stop");
  //Insert new child and key
  if (parent -> nkey < N -1 ) {
    printf("inside insert_in_parent2\n");
    int i;
    //If the key is the smallest
    if (key < parent -> key[0]) {
      for (i = parent -> nkey + 1; i > 0; i--) {
        parent ->chi[i] = parent ->chi[i-1];
        parent ->key[i] = parent ->key[i-1];
      }
    }
    //If the key should be in the middle or the end
    else {
      for (i = 0; i < parent->nkey; i++) {
        if (key < parent->key[i]) break;
      }
      for (int j = parent->nkey; j > i; j--) {		
        parent->chi[j] = parent->chi[j-1] ;
        parent->key[j] = parent->key[j-1] ;
      } 
    }
    parent -> chi[i+1] = right_child;
    parent -> key[i] = key;
    parent -> nkey++; 
  }
  //If parent is full
  else {
    TEMP *tempNode = create_temp(parent, key, right_child);
    int i;
    //If the key is the smallest
    printf("stop");
    for (i = 0; i < N; i++) {
      if (left_child == parent -> chi[i]) break;
    }
    tempNode -> chi[i] = left_child;
    tempNode -> chi[i+1] = right_child;
    tempNode -> key[i] = key;
    printf("stop");
    
    return;
  }
  return;
}

void 
split(TEMP *tempnode, NODE *original_node) 
{
  printf("inside split\n");
  NODE *new_node;
  if (!(new_node = (NODE*) calloc(1, sizeof(NODE)))) ERR;
  int i;
  int j = 0;

  for (i = 0; i < N/2; i++) {
    original_node -> key[i] = tempnode -> key[i];
    original_node -> chi[i] = tempnode -> chi[i];
  }
  
  for (i = N/2 ; i < N ; i++) {
    //erase
    if (i < N - 1) {
      original_node -> key[i] = 0;
      original_node -> chi[i] = 0;
      original_node -> nkey--;
    }
    //copy from tempnode
    new_node -> key[j] = tempnode -> key[i];
    new_node -> chi[j] = tempnode ->chi[i];
    new_node -> nkey++;
    j++;
  }
  //change pointers in the end
  original_node -> chi[N-1] = new_node;
  new_node -> chi[N-1] = tempnode -> chi[N];
  new_node -> isLeaf = tempnode -> isLeaf;
  new_node -> parent = original_node -> parent;
  free(tempnode);
  //Insert_in_parent
  insert_in_parent(original_node, new_node -> key[0], new_node);
  return;
}


void 
insert(int key, DATA *data)
{
	NODE *leaf;
  //If tree is empty, create an empty leaf node which is root
	if (Root == NULL) {
		leaf = alloc_leaf(NULL);
		Root = leaf;
	}
  //Find the leaf node L
	else {
	leaf = find_leaf(Root, key);
	}
  //If L has less than n - 1 keys
	if (leaf->nkey < (N-1)) {
		insert_in_leaf(leaf, key, data);
	}
	else {
    printf("have to split\n");
    TEMP* tempNode = create_temp(leaf, key, (NODE*) data);
    split(tempNode, leaf);
    printf("split done\n");
	}
  return;
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
  struct timeval begin, end;

	init_root();

	printf("-----Insert-----\n");
	begin = cur_time();
  while (true) {
    // insert(interactive(), NULL);
    insert(1, NULL);
    insert(2, NULL);
    insert(5, NULL);
    insert(6, NULL);
    insert(4, NULL);
    insert(3, NULL);
    printf("after insert\n");
    print_tree(Root);
  }
	end = cur_time();

	return 0;
}
