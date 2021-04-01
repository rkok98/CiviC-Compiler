#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct listnode
{
    char *value;
    int counter;
    struct listnode* next;
} listnode;

typedef void (*callback)(listnode* data);


/**
 *  create a new listnode
 *  initialize the data and next field
 *
 *  return the newly created listnode
*/
listnode *LLcreate(char *value, int counter, listnode* next)
{
    listnode* new_listnode = (listnode*)malloc(sizeof(listnode));
    new_listnode->value = value;
    new_listnode->counter = counter;
    new_listnode->next = next;

    return new_listnode;
}

/**
 *  add a new listnode at the beginning of the list
 */
listnode *LLprepend(listnode* head, char *value, int counter)
{
    listnode* new_listnode = LLcreate(value, counter, head);
    head = new_listnode;
    return head;
}

/**
 *  add a new listnode at the end of the list
 */
listnode *LLappend(listnode* head, char *value, int counter)
{
    // do we have a head?
    if (head == NULL) return NULL;

    // go to the last listnode
    listnode *cursor = head;

    // get the last entry
    while(cursor->next != NULL) cursor = cursor->next;

    // create a new listnode
    listnode* new_listnode =  LLcreate(value, counter, NULL);

    // set the next node
    cursor->next = new_listnode;

    // return the heads
    return head;
}

/*
 *  traverse the linked list
*/
void LLtraverse(listnode* head, callback f)
{
    listnode* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

/**
 *  remove listnode from the front of list
 */
listnode *LLremove_front(listnode* head)
{
    if ( head == NULL) return NULL;
    listnode *front = head;
    head = head->next;
    front->next = NULL;
    /* is this the last listnode in the list */
    if (front == head) head = NULL;
    free(front);

    return head;
}

/**
 *  remove listnode from the back of the list
 */
listnode *LLremove_back(listnode* head)
{
    if(head == NULL)
        return NULL;

    listnode *cursor = head;
    listnode *back = NULL;
    while(cursor->next != NULL)
    {
        back = cursor;
        cursor = cursor->next;
    }

    if(back != NULL)
        back->next = NULL;

    /* if this is the last listnode in the list*/
    if(cursor == head)
        head = NULL;

    free(cursor);

    return head;
}

/**
 *  remove a listnode from the list
 */
listnode *LLremove_any(listnode* head, listnode* nd)
{
    if(nd == NULL) return NULL;
    /* if the listnode is the first listnode */
    if(nd == head) return LLremove_front(head);

    /* if the listnode is the last listnode */
    if(nd->next == NULL)
        return LLremove_back(head);

    /* if the listnode is in the middle */
    listnode* cursor = head;
    while(cursor != NULL)
    {
        if(cursor->next == nd) break;
        cursor = cursor->next;
    }

    if(cursor != NULL)
    {
        listnode* tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

/**
 *  display a listnode
 */
void LLprint(listnode* cursor)
{
    while (cursor!=NULL)
    {
        printf("%s\n", cursor->value);
        cursor = cursor->next;
    }
}

/**
 *  Search for a specific listnode with input data
 *
 *  return the first matched listnode that stores the input data,
 *  otherwise return NULL
 */
listnode *LLsearch(listnode* head, const char *value)
{

    listnode *cursor = head;
    while (cursor!=NULL)
    {
        if (strcmp ( cursor->value, value) == 0) return cursor;

        cursor = cursor->next;
    }
    return NULL;
}

/**
 *  remove all element of the list
 */
void LLdispose(listnode *cursor)
{
    if (cursor == NULL) return;

    listnode *tmp;

    while (cursor != NULL)
    {
        tmp = cursor->next;
        free(cursor->value);
        free(cursor);

        cursor = tmp;
    }
    
}
/**
 *  return the number of elements in the list
 */
int LLcount(listnode *head)
{
    listnode *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}

/**
 *  reverse the linked list
 */
listnode *LLreverse(listnode* head)
{
    listnode* prev    = NULL;
    listnode* current = head;
    listnode* next;
    while (current != NULL)
    {
        next  = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    head = prev;
    return head;
}
