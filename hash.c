#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

//HashTable
struct bucket* hashTable = NULL;
int SIZE = 10;
//Binary Tree
struct node* root = NULL;

struct node{
    int key;
    int value;
    struct node* next;
    struct node* left;
    struct node* right;
};

struct bucket {
    struct node* head;
    int count;
};

struct node* createNode(int key, int value) {
    struct node* newNode;
    newNode = (struct node*)malloc(sizeof(struct node));

    newNode -> key = key;
    newNode -> value = value;

    newNode -> next = NULL;
    newNode -> left = NULL;
    newNode -> right = NULL;

    return newNode;
}

int hashFunction(int key) {
    return key%SIZE;
}

void insertHash(int key, int value){
    int hashIndex = hashFunction(key);
    struct node* newNode = createNode(key, value);
    //No node at hashIndex
    if (hashTable[hashIndex].count == 0){
        hashTable[hashIndex].head = newNode;
        hashTable[hashIndex].count = 1;
    }
    //If there are nodes at hashIndex
    else {
        newNode -> next = hashTable[hashIndex].head;
        hashTable[hashIndex].head = newNode;
        hashTable[hashIndex].count += 1;
    }
    return;
}

void insertBinary(int key, int value) {
    struct node* p = root;
    struct node* parent = NULL;
    if (p == NULL) {
        root = createNode(key, value);
        return;
    }
    while (p!=NULL) {
        parent = p;
        if (p -> value == value) {
            printf("[ Binary Search ] Already existed value in the tree\n");
            return;
        }
        else if (value < p -> value) {
            p = p -> left;
        }
        else {
            p = p -> right;
        }
    }
    if (parent != NULL) {
        if (value < parent -> value) {
            parent -> left = createNode(key, value);
        }
        else {
            parent -> right = createNode(key, value);
        }
    }

}


void searchHash(int key) {
    int searchCount = 0;
    int hashIndex = hashFunction(key);
    struct node* node = hashTable[hashIndex].head;
    if (node == NULL) {
        printf("[ Hash Search ] NOT FOUND\n");
        return;
    }
    while(node != NULL) {
        searchCount++;
        if (node -> key == key) {
            printf("[  Hash Search  ] FOUND! Key: %d, Value: %d\n", key, node -> value);
            printf("[  Hash Search  ] Search Count: %d\n", searchCount);
            break;
        }
        node = node -> next;
    }
}

void searchBinary(int value) {
    int searchCount = 0;
    struct node* p = root;
    while (p!=NULL) {
        searchCount++;
        if (p -> value == value) {
            printf("[ Binary Search ] FOUND! Key: %d, Value: %d\n", p -> key, value);
            printf("[ Binary Search ] Search Count: %d\n", searchCount);
            return;
        }
        else if (value < p -> value) {
            p = p -> left;
        }
        else {
            p = p -> right;
        }
    }
    printf("[ Binary Search ] NOT FOUND!\n");
    printf("[ Binary Search ] Search Count: %d\n", searchCount);
    return;
}

void insert(int key, int value) {
    insertHash(key, value);
    insertBinary(key, value);
    return;
}

void search(int key, int value) {
    searchBinary(value);
    searchHash(key);
}


int main() {
    hashTable = (struct bucket*) malloc(SIZE*sizeof(struct bucket));

    insert(0,1);
    insert(1, 10);
    insert(11, 12);
    insert(21, 13);
    insert(3, 5);
    insert(35, 4);
    insert(51, 14);

    search(0, 1);
}