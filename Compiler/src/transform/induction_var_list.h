#ifndef _INDUCTION_VAR_LIST_H_
#define _INDUCTION_VAR_LIST_H_

typedef struct list_node
{
    const char *old_name;
    const char *new_name;
    struct list_node *next;
} list_node;

extern list_node *IVLadd(list_node *list, const char *old_name, const char *new_name);
extern list_node *IVLfind(list_node *list, const char *old_name);
extern list_node *IVLremove(list_node *list, const char *old_name);
extern void IVLdispose(list_node *head);

#endif