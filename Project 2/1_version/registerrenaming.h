#include "physicalRegisters.h"

typedef struct preg
{
    int data;
    struct preg* next;
} preg;


preg *phead;
preg *ptail;
typedef void (*callback1)(preg* data);

preg* create1(int data,preg* next)
{
    preg* new_preg = (preg*)malloc(sizeof(preg));
    if(new_preg == NULL)
    {
        printf("Error creating a new preg.\n");
        exit(0);
    }
    new_preg->data = data;
    new_preg->next = next;

    return new_preg;
}

preg* prependIntoPreg(preg* head,int data)
{
    preg* new_preg = create1(data,head);
    head = new_preg;
    return head;
}

preg* append1(preg* head, int data)
{
    if(head == NULL)
        return NULL;
    /* go to the last preg */
    preg *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;

    /* create1 a new preg */
    preg* new_preg =  create1(data,NULL);
    cursor->next = new_preg;

    return head;
}



preg* enqueueReg(preg* head,int data){
        if(head == NULL){
            head = prependIntoPreg(head,data);
            }
            else{
            head = append1(head,data);
            }
            return head;
}

void traverse1(preg* head,callback1 f)
{
    preg* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

preg* dequeueReg(preg* head)
{
    if(head == NULL)
        return NULL;
    preg *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last preg in the list */
    if(front == head)
        head = NULL;
    free(front);
    return head;
}

int countReg(preg *head)
{
    preg *cursor = head;
    int c = 0;
    while(cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

int searchAtIndexReg(preg* head,int index)
{
    
    preg *cursor = head;
    int i=0;
    while(i!=index)
    {
       
        cursor = cursor->next;
        i++;
    }
    return cursor->data;
}

void disposeReg(preg *head)
{
    preg *cursor, *tmp;

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



