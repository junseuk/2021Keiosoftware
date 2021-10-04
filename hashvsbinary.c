#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

int SIZE;
int numData;
//HashTable
struct bucket* hashTable = NULL;

struct node{
    int key;
    struct node* next;
};

struct bucket {
    struct node* head;
    int count;
};

struct node* createNode(int key) {
    struct node* newNode;
    newNode = (struct node*)malloc(sizeof(struct node));

    newNode -> key = key;
    newNode -> next = NULL;

    return newNode;
}

int hashFunction(int key) {
    return key%SIZE;
}

void insertHash(int key){
    int hashIndex = hashFunction(key);
    struct node* newNode = createNode(key);
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

int partition (int array[numData], int start, int end) {
    int pivot = array[(start + end)/2];
    while (start <= end) {
        while(array[start] < pivot) start++;
        while(array[end] > pivot) end--;
        if (start <= end) {
            int tmp = array[start];
            array[start] = array[end];
            array[end] = tmp;
            start++;
            end--;
        }
    }
    return start;
}

void quickSort(int array[numData], int start, int end) {
    int secondStart = partition(array, start, end);
    if ( start < secondStart - 1) {
        quickSort(array, start, secondStart - 1);
    }
    if ( secondStart < end ) {
        quickSort(array, secondStart, end );
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
            printf("[  Hash Search  ] FOUND! Key: %d\n", key);
            printf("[  Hash Search  ] Search Count: %d\n", searchCount);
            return;
        }
        node = node -> next;
    };
    return;
}

void searchBinary(int key, int array[SIZE]) {
    int start = 0;
    int end = numData - 1;
    int searchCount = 0;
    while(1) {
        if (start > end) {
            printf("[ binary Search ] NOT FOUND\n");
            return;
        }
        searchCount++;
        int mid = (start + end) / 2;
        if ( array[mid] == key ) {
            printf("[ Binary Search ] FOUND! Key: %d\n", key);
            printf("[ Binary Search ] Search Count: %d\n", searchCount);
            return;
        }
        else if ( key < array[mid] ) {
            end = mid;
            continue;
        }
        else {
            start = mid + 1;
            continue;
        }
    }
}

void search(int key, int array[SIZE]) {
    searchHash(key);
    searchBinary(key, array);
    return;
}


int main() {
    char buffer[100];
    srand(time(0));
    //SIZE of hash table
    printf("Please decide the size of hash table: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", &SIZE);
    assert(SIZE>0);
    hashTable = (struct bucket*) malloc(SIZE*sizeof(struct bucket));
    //Number of Data
    printf("Please input the number of data: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", &numData);
    //Create sorted array and hash table
    int array[numData];
    for (int i=0;i<numData;i++) {
        int randValue = rand();
        insertHash(randValue);
        array[i] = randValue;
        printf("Inserted Key: %d\n", randValue);
    }
    quickSort(array, 0, numData - 1);
    
    //Key to search
    int key = 0;
    printf("Input the key to search: ");
    fgets(buffer, sizeof(buffer), stdin);
    sscanf(buffer, "%d", &key);
    assert(key>=0);
    search(key, array);
}