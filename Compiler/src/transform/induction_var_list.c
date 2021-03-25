#include "induction_var_list.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef void (*callback)(list_node *data);

list_node *IVLcreate(const char *old_name, const char *new_name, list_node *next)
{
    list_node *new_list_node = (list_node *)malloc(sizeof(list_node));
    new_list_node->old_name = old_name;
    new_list_node->new_name = new_name;
    new_list_node->next = next;

    return new_list_node;
}

list_node *IVLadd(list_node *list, const char *old_name, const char *new_name)
{
    if (list == NULL)
    {
        list = IVLcreate(old_name, new_name, NULL);
        return list;
    }

    list_node *cursor = list;
    while (cursor->next != NULL)
    {
        cursor = cursor->next;
    }

    list_node *new_kvlistnode = IVLcreate(old_name, new_name, NULL);
    cursor->next = new_kvlistnode;

    return list;
}

list_node *IVLfind(list_node *list, const char *old_name)
{

    list_node *cursor = list;
    while (cursor != NULL)
    {
        if (strcmp(cursor->old_name, old_name) == 0)
        {
            return cursor;
        }

        cursor = cursor->next;
    }

    return NULL;
}

list_node *IVLremove(list_node *list, const char *old_name)
{
    if (list == NULL)
    {
        return NULL;
    }

    struct list_node *current = list;
    struct list_node *previous = NULL;

    while (current->old_name != old_name)
    {
        if (current->next == NULL)
        {
            return NULL;
        }
        else
        {
            previous = current;
            current = current->next;
        }
    }

    if (current == list)
    {
        list = list->next;
    }
    else
    {
        previous->next = current->next;
    }

    return current;
}

void IVLdispose(list_node *head)
{
    list_node *cursor, *tmp;

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
