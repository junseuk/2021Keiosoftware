TEMP* create_temp(NODE* leaf) {
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
  return tempNode;
}
  
  
void main() {
  if(1) {

  }
  else {
    //insert_in_temp
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
        tempNode->chi[j] = tempNode->chi[j-1];
        tempNode->key[j] = tempNode->key[j-1];
      } 
    }
    tempNode -> key[i] = key;
    tempNode -> chi[i] = (NODE *) data;
    tempNode -> nkey++;
    //insert_in_temp end
    split(tempNode, leaf);
	}

}


void split(TEMP *tempnode, NODE *original_node) {
  NODE *new_node;
  if (!(new_node = (NODE*) calloc(1, sizeof(NODE)))) ERR;
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
  original_node -> chi[N-1] = new_node;
  new_node -> chi[N-1] = tempnode -> chi[N];
  new_node -> parent = original_node -> parent;
  new_node -> isLeaf = true;
  free(tempnode);
  insert_to_parent(original_node -> parent, new_node -> key[0], original_node, new_node);
  }
}