#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct kvlistnode
{
    const char *key;
    const char *value;
    struct kvlistnode* next;
} kvlistnode;

typedef void (*callback)(kvlistnode* data);


/**
 *  create a new listnode
 *  initialize the data and next field
 *
 *  return the newly created listnode
*/
kvlistnode *KVLLcreate(const char *key, const char *value, kvlistnode* next)
{
    kvlistnode* new_kvlistnode = (kvlistnode*)malloc(sizeof(kvlistnode));
    new_kvlistnode->key = key;
    new_kvlistnode->value = value;
    new_kvlistnode->next = next;

    return new_kvlistnode;
}

/**
 *  add a new listnode at the beginning of the list
 */
kvlistnode *KVLLprepend(kvlistnode* head, const char *key, const char *value)
{
    kvlistnode* new_kvlistnode = KVLLcreate(key, value, head);
    head = new_kvlistnode;
    return head;
}

/**
 *  add a new listnode at the end of the list
 */
kvlistnode *KVLLappend(kvlistnode* head, const char * key, const char *value)
{
    if(head == NULL) return NULL;
    /* go to the last listnode */
    kvlistnode *cursor = head;
    while(cursor->next != NULL)
        cursor = cursor->next;

    /* create a new listnode */
    kvlistnode* new_kvlistnode =  KVLLcreate(key, value, NULL);
    cursor->next = new_kvlistnode;

    return head;
}

/*
 *  traverse the linked list
*/
void KVLLtraverse(kvlistnode* head, callback f)
{
    kvlistnode* cursor = head;
    while(cursor != NULL)
    {
        f(cursor);
        cursor = cursor->next;
    }
}

/**
 *  remove listnode from the front of list
 */
kvlistnode *KVLLremove_front(kvlistnode* head)
{
    if ( head == NULL) return NULL;
    kvlistnode *front = head;
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
kvlistnode *KVLLremove_back(kvlistnode* head)
{
    if(head == NULL)
        return NULL;

    kvlistnode *cursor = head;
    kvlistnode *back = NULL;
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
kvlistnode *KVLLremove_any(kvlistnode* head, kvlistnode* nd)
{
    if(nd == NULL) return NULL;
    /* if the listnode is the first listnode */
    if(nd == head) return KVLLremove_front(head);

    /* if the listnode is the last listnode */
    if(nd->next == NULL)
        return KVLLremove_back(head);

    /* if the listnode is in the middle */
    kvlistnode* cursor = head;
    while(cursor != NULL)
    {
        if(cursor->next == nd)
            break;
        cursor = cursor->next;
    }

    if(cursor != NULL)
    {
        kvlistnode* tmp = cursor->next;
        cursor->next = tmp->next;
        tmp->next = NULL;
        free(tmp);
    }
    return head;
}

/**
 *  display a listnode
 */
void KVLLdisplay(kvlistnode* cursor)
{
    while (cursor!=NULL)
    {
        printf("%s: %s\n", cursor->key, cursor->value);
        cursor = cursor->next;
    }
}

/**
 *  Search for a specific listnode with input data
 *
 *  return the first matched listnode that stores the input data,
 *  otherwise return NULL
 */
kvlistnode *KVLLsearch(kvlistnode* head, const char * key)
{

    kvlistnode *cursor = head;
    while (cursor!=NULL)
    {
        if (strcmp ( cursor->key, key) == 0) return cursor;

        cursor = cursor->next;
    }
    return NULL;
}

/**
 *  remove all element of the list
 */
void KVLLdispose(kvlistnode *head)
{
    kvlistnode *cursor, *tmp;

    if (head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while (cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}

/**
 *  return the number of elements in the list
 */
int KVLLcount(kvlistnode *head)
{
    kvlistnode *cursor = head;
    int c = 0;
    while (cursor != NULL)
    {
        c++;
        cursor = cursor->next;
    }
    return c;
}