#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

//HashTable
struct bucket* hashTable = NULL;
int SIZE;
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
            //printf("[ Binary Search ] Already existed value in the tree\n");
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


int searchHash(int key) {
    //printf("inside searchHash key: %d", key);
    int searchCount = 0;
    int hashIndex = hashFunction(key);
    struct node* node = hashTable[hashIndex].head;
    if (node == NULL) {
        printf("[ Hash Search ] NOT FOUND\n");
        return -1;
    }
    while(node != NULL) {
        searchCount++;
        if (node -> key == key) {
            printf("[  Hash Search  ] FOUND! Key: %d, Value: %d\n", key, node -> value);
            printf("[  Hash Search  ] Search Count: %d\n", searchCount);
            return node -> value;
        }
        node = node -> next;
    };
    return -1;
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

void search(int key) {
    int value = searchHash(key);
    searchBinary(value);
    return;
}


int main() {
    char buffer[1000];
    unsigned int numData;
    srand(time(0));
    //SIZE of hash table
    printf("Please decide the size of hash table: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%u", &SIZE);
    assert(SIZE>0);
    hashTable = (struct bucket*) malloc(SIZE*sizeof(struct bucket));

    //Number of Data
    printf("Please input the number of data: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%u", &numData);
    for (int i=0;i<numData;i++) {
        int randValue = rand();
        insert(i, randValue);
        printf("%d, %d\n", i, randValue);
    }
    
    //Key to search
    int key = 0;
    printf("Input the key to search: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", &key);
    assert(key<numData);
    assert(key>=0);
    search(key);
}