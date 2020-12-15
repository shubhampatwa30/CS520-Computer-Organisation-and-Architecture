
#include <stdio.h>
#include <stdlib.h>
#include "apex_cpu.h"

typedef struct prf_hashcode{
    int rf_code;// register value
    int prf_code;//physical value
}prf_hashcode;
  
typedef struct hasher
{
    prf_hashcode data;
    struct hasher* next;
} hasher;


prf_hashcode renametable;
hasher *rfprf;
typedef void (*callback2)(hasher* data);


hasher* create2(prf_hashcode data,hasher* next)
{
    hasher* new_preg = (hasher*)malloc(sizeof(hasher));
    if(new_preg == NULL)
    {
        printf("Error creating a new hasher.\n");
        exit(0);
    }
    new_preg->data = data;
    new_preg->next = next;

    return new_preg;
}

hasher* prependIntoRenameTable(hasher* head,prf_hashcode data)
{
    hasher* new_preg = create2(data,head);
    head = new_preg;
    return head;
}


hasher* append2(hasher* head, prf_hashcode data)
{
    if(head == NULL)
        return NULL;
    /* go to the last hasher */
    hasher *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;

    /* create1 a new hasher */
    hasher* new_preg =  create2(data,NULL);
    cursor->next = new_preg;

    return head;
}



hasher* enqueueprf(hasher* head,prf_hashcode data){
        if(head == NULL){
            head = prependIntoRenameTable(head,data);
            }
            else{
            head = append2(head,data);
            }
            return head;
}

void traverse2(hasher* head,callback2 f)
{
    hasher* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

hasher* dequeueprf(hasher* head)
{
    if(head == NULL)
        return NULL;
    hasher *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last hasher in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}

int searchprftop(hasher* head,int data)
{

    hasher *cursor = head;
    while(cursor!=NULL)
    {
        if(cursor->data.rf_code == data)
            return cursor->data.prf_code;
        cursor = cursor->next;
    }
    return PREGS_FILE_SIZE;
}
int searchprftop1(hasher* head,int data)
{

    hasher *cursor = head;
    while(cursor!=NULL)
    {
        if(cursor->data.rf_code == data)
            return cursor->data.prf_code;
        cursor = cursor->next;
    }
    return -1;
}

int countPrfList(hasher *head)
{
    hasher *cursor = head;
    int c = 0;
    while(cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

prf_hashcode searchAtIndexprf(hasher* head,int index)
{
    
    hasher *cursor = head;
    int i=0;
    while(i!=index)
    {
       
        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

void disposePrf(hasher *head)
{
    hasher *cursor, *tmp;

    if(head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while(cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}



