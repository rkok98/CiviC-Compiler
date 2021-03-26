#include "normalize_for_loops.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"
#include "ctinfo.h"

#include <stdlib.h>
#include <string.h>

typedef struct list_node
{
    const char *old_name;
    const char *new_name;
    struct list_node *next;
} list_node;

struct INFO
{
    unsigned int for_loop_counter;
    list_node *induction_variables;
    
    node *variable_declarations;
    node *statements;
};

#define INFO_FOR_LOOP_COUNTER(n) ((n)->for_loop_counter)
#define INFO_INDUCTION_VARIABLES(n) ((n)->induction_variables)

#define INFO_VARDECLS(n) ((n)->variable_declarations)
#define INFO_STATEMENTS(n) ((n)->statements)

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

static info *MakeInfo()
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    INFO_VARDECLS(result) = NULL;
    INFO_STATEMENTS(result) = NULL;
    INFO_INDUCTION_VARIABLES(result) = NULL;
    INFO_FOR_LOOP_COUNTER(result) = 0;

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    IVLdispose(INFO_INDUCTION_VARIABLES(info));

    info = MEMfree(info);

    DBUG_RETURN(info);
}

void add_to_vardecls(node *decls, node *new_decl)
{
    node *current = decls;

    while (VARDECL_NEXT(current))
    {
        current = VARDECL_NEXT(current);
    }

    VARDECL_NEXT(current) = new_decl;
}

void add_to_stmts(node *stmts, node *new_stmt)
{
    node *current = stmts;

    while (STMTS_NEXT(current))
    {
        current = STMTS_NEXT(current);
    }

    STMTS_NEXT(current) = new_stmt;
}

node *NFLfunbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLfunbody");

    info *funbody_info = MakeInfo();

    FUNBODY_STMTS(arg_node) = TRAVopt(FUNBODY_STMTS(arg_node), funbody_info);

    if (INFO_VARDECLS(funbody_info))
    {
        if (!FUNBODY_VARDECLS(arg_node))
        {
            FUNBODY_VARDECLS(arg_node) = INFO_VARDECLS(funbody_info);
        }
        else
        {
            add_to_vardecls(FUNBODY_VARDECLS(arg_node), INFO_VARDECLS(funbody_info));
        }
    }

    funbody_info = FreeInfo(funbody_info);
    DBUG_RETURN(arg_node);
}

node *NFLstmts(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLstmts");

    nodetype type = NODE_TYPE(STMTS_STMT(arg_node));

    STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);

    if (type == N_for)
    {
        node *oldnode = arg_node;
        add_to_stmts(INFO_STATEMENTS(arg_info), arg_node);
        arg_node = INFO_STATEMENTS(arg_info);
        INFO_STATEMENTS(arg_info) = NULL;

        STMTS_NEXT(oldnode) = TRAVopt(STMTS_NEXT(oldnode), arg_info);
    }
    else
    {
        STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);
    }

    DBUG_RETURN(arg_node);
}

node *NFLfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLfor");

    // Generate a new induction variable base name
    char *induction_basename = STRcatn(4, "_for_", STRitoa(INFO_FOR_LOOP_COUNTER(arg_info)), "_", FOR_LOOPVAR(arg_node));
    INFO_FOR_LOOP_COUNTER(arg_info)++;

    INFO_INDUCTION_VARIABLES(arg_info) = IVLadd(INFO_INDUCTION_VARIABLES(arg_info), FOR_LOOPVAR(arg_node), induction_basename);

    // Create var decls for the for-loop
    node *vardecl_step = TBmakeVardecl(STRcat(induction_basename, "_step"), T_int, NULL, NULL, NULL);
    node *vardecl_stop = TBmakeVardecl(STRcat(induction_basename, "_stop"), T_int, NULL, NULL, vardecl_step);
    node *vardecl_start = TBmakeVardecl(STRcpy(induction_basename), T_int, NULL, NULL, vardecl_stop);

    if (INFO_VARDECLS(arg_info) == NULL)
    {
        INFO_VARDECLS(arg_info) = vardecl_start;
    }
    else
    {
        add_to_vardecls(INFO_VARDECLS(arg_info), vardecl_start);
    }

    // Create the for-loop's statements
    FOR_BLOCK(arg_node) = TRAVopt(FOR_BLOCK(arg_node), arg_info);

    node *induction_step = TBmakeNum(1); 
    if (FOR_STEP(arg_node))
    {
        induction_step = COPYdoCopy(FOR_STEP(arg_node));
    }

    node *induction_step_stmt = TBmakeStmts(TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_step)), vardecl_step, NULL), induction_step), NULL);
    node *stop_stmt = TBmakeStmts(TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_stop)), vardecl_stop, NULL), COPYdoCopy(FOR_STOP(arg_node))), induction_step_stmt);
    node *start_stmt = TBmakeStmts(TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), COPYdoCopy(FOR_START(arg_node))), stop_stmt);

    INFO_STATEMENTS(arg_info) = start_stmt;

    node *block = COPYdoCopy(FOR_BLOCK(arg_node));

    node *assign = TBmakeAssign(TBmakeVarlet(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), 
                                TBmakeBinop(BO_add, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), 
                                TBmakeVar(STRcpy(VARDECL_NAME(vardecl_step)), vardecl_step, NULL)));

    if (!block)
    {
        block = TBmakeStmts(assign, NULL);
    }
    else
    {
        add_to_stmts(block, TBmakeStmts(assign, NULL));
    }

    FREEdoFreeTree(arg_node);

    node *while_expr = TBmakeBinop(BO_lt, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), NULL, NULL), TBmakeVar(STRcpy(VARDECL_NAME(vardecl_stop)), NULL, NULL));
    DBUG_RETURN(TBmakeWhile(while_expr, block));
}

node *NFLvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvarlet");

    list_node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VARLET_NAME(arg_node));

    if (node)
    {
        VARLET_NAME(arg_node) = STRcpy(node->new_name);
    }

    DBUG_RETURN(arg_node);
}

node *NFLvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvar");

    list_node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VAR_NAME(arg_node));

    if (node)
    {
        VAR_NAME(arg_node) = STRcpy(node->new_name);
    }

    DBUG_RETURN(arg_node);
}

node *NFLdoNormalizeForLoops(node *syntaxtree)
{
    DBUG_ENTER("NFLdoNormalizeForLoops");

    TRAVpush(TR_nfl);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
