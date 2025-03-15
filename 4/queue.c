#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <threads.h>
#include "queue.h"
/*a struct in order to represenet the queue*/
typedef struct Node{
    void* data;
    struct Node *next;
} Node;
/*a struct inorder to consruct a waiting threads queue*/
 typedef struct threadNode{
    struct threadNode* next;
    bool noLock;
    cnd_t thrdCV;
    void* data;
} threadNode;
/*declaring variables*/
static threadNode* threadHead;
static threadNode* threadTail;

static int passingCounter;
static int qSize;
static int dqWaiting;
static Node* head;
static Node* tail;
static mtx_t queueLock;

/*addes a threadnode to a thread queue*/
void addThread(threadNode* newThread){
    if (threadHead == NULL){
        threadHead = newThread;
        threadTail = newThread;
    }
    else{
        threadTail->next = newThread;
        threadTail = newThread;
    }
    return;
}
/*Poping the head of the thread queue head, if no head reurns NULL*/
threadNode* popThread(){
    if (threadHead == NULL){
        return NULL;
    }
    else if(threadHead == threadTail){
        threadNode* tempNode = threadHead;
        tempNode->next = NULL; 
        threadHead = NULL;
        threadTail = NULL; 
        return tempNode;
    }
    else{
        threadNode* tempNode = threadHead;
        threadHead = threadHead->next;
        tempNode->next = NULL;
        return tempNode;
    }
    
}
/*init queue using lock*/
void initQueue(){
    mtx_init(&queueLock,mtx_plain);
    mtx_lock(&queueLock);
    threadHead = NULL;
    threadTail = threadHead;
    head = NULL;
    tail = head;
    passingCounter = 0;
    dqWaiting = 0;
    qSize = 0;
    mtx_unlock(&queueLock);
}
/*destorying queue locking as much as possible*/
void destroyQueue(){
    mtx_lock(&queueLock);
    while(head != NULL){
        Node* temp = head;
        head = head->next;
        free(temp);
        }
    head = NULL;
    tail = NULL;
    while(threadHead!= NULL){
        threadNode* tempThrd = threadHead;
        threadHead = threadHead->next;
        free(tempThrd);
    }
    threadHead = NULL;
    threadTail = NULL;
    mtx_unlock(&queueLock);
    mtx_destroy(&queueLock);
}

void enqueue(void* newData){
    mtx_lock(&queueLock);
    /*Checking if there are waiting blocked threads in dequeue*/
    threadNode* thrdQNode = popThread();
    if(thrdQNode != NULL){
        /*signaling to the the thread from thread queue dequeue with the new element added */
        thrdQNode->data = newData;
        thrdQNode->noLock = true;
        cnd_signal(&(thrdQNode->thrdCV)); 
    } 
    else{ /*else adding*/
        Node* newNode = (Node*)malloc(sizeof(Node));
        newNode->data = newData;
        newNode->next = NULL;
    if(head == NULL)
    {
        head = newNode;
        tail = newNode;
    }
    else{
        tail->next = newNode;
        tail = tail->next;
    }
    }
    qSize++;
    mtx_unlock(&queueLock);
    return;
}
void* dequeue(){
    mtx_lock(&queueLock);
    void* data;
    bool noLock = true; /* no lock is for preventing threads that shouldn't do anything*/
    /*We are blocking while head is empty*/
    if(head == NULL){
        threadNode* tempThread = (threadNode*)malloc(sizeof(threadNode));
        tempThread->next = NULL;
        addThread(tempThread);
        dqWaiting++;
        /*adding to thread queue */
        cnd_init(&(tempThread->thrdCV));
        cnd_wait(&(tempThread->thrdCV),&queueLock);
        /*back from thread queue*/
        data = tempThread->data;
        noLock = tempThread->noLock;
        if(noLock){
            dqWaiting--;
        }
        cnd_destroy(&tempThread->thrdCV);
        free(tempThread);
    }
    else{
        data = head->data;
        Node* tempNode = head;
        head = head->next;
        free(tempNode);
    }
    if(noLock){
        passingCounter++;
        qSize--;
    }   
    mtx_unlock(&queueLock);
    return data;
}
bool tryDequeue(void** returnData){
    /*Trying to add the data if can't return false*/
    mtx_lock(&queueLock);
    if (head == NULL)
    {
        mtx_unlock(&queueLock);
        return false;
    }
    Node* temp = head;
    *returnData = head->data;
    head = head->next;
    free(temp);
    passingCounter++;
    qSize--;
    mtx_unlock(&queueLock);
    return true;
}
size_t size(){
    return qSize;
}
size_t waiting(){
    return dqWaiting;
}
size_t visited(){
    return passingCounter;
}