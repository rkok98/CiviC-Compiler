#include "for_loop_variable_initialisation.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "copy.h"
#include "ctinfo.h"

#include <string.h>

struct INFO
{
    unsigned int for_loop_counter;
    node *induction_variables;

    node *variable_declarations;
    node *statements;
};

#define INFO_FOR_LOOP_COUNTER(n) ((n)->for_loop_counter)
#define INFO_INDUCTION_VARIABLES(n) ((n)->induction_variables)

#define INFO_VARDECLS(n) ((n)->variable_declarations)
#define INFO_STATEMENTS(n) ((n)->statements)

node *IVLadd(node *list, node *new_link)
{
    if (list == NULL)
    {
        list = new_link;
        return list;
    }

    node *cursor = list;
    while (LINKEDVALUE_NEXT(cursor))
    {
        cursor = LINKEDVALUE_NEXT(cursor);
    }

    LINKEDVALUE_NEXT(cursor) = new_link;

    return list;
}

node *IVLfind(node *list, const char *old_name)
{

    node *cursor = list;
    while (cursor)
    {
        if (strcmp(LINKEDVALUE_KEY(cursor), old_name) == 0)
        {
            return cursor;
        }

        cursor = LINKEDVALUE_NEXT(cursor);
    }

    return NULL;
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

    info = MEMfree(info);

    DBUG_RETURN(info);
}

node *add_to_vardecls(node *decls, node *new_decl)
{
    if (!decls)
    {
        return new_decl;
    }
    else
    {
        VARDECL_NEXT(decls) = add_to_vardecls(VARDECL_NEXT(decls), new_decl);
    }

    return decls;
}

node *add_to_stmts(node *stmts, node *new_stmt)
{
    if (!stmts)
    {
        return new_stmt;
    }
    else
    {
        STMTS_NEXT(stmts) = add_to_stmts(STMTS_NEXT(stmts), new_stmt);
    }

    return stmts;
}

node *FLVIfunbody(node *arg_node, info *arg_info)
{
    DBUG_ENTER("FLVIfunbody");

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

node *FLVIstmts(node *arg_node, info *arg_info)
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

node *FLVIfor(node *arg_node, info *arg_info)
{
    DBUG_ENTER("FLVIfor");

    // Generate a new induction variable base name
    char *induction_basename = STRcatn(4, "_for_", STRitoa(INFO_FOR_LOOP_COUNTER(arg_info)), "_", FOR_LOOPVAR(arg_node));
    INFO_FOR_LOOP_COUNTER(arg_info)
    ++;

    INFO_INDUCTION_VARIABLES(arg_info) = IVLadd(INFO_INDUCTION_VARIABLES(arg_info), TBmakeLinkedvalue(FOR_LOOPVAR(arg_node), induction_basename, NULL));

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

    // Create a new while loop and return it to replace the for-loop
    node *while_expr = TBmakeTernary(
        TBmakeBinop(BO_gt, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_step)), vardecl_step, NULL), TBmakeNum(0)),
        TBmakeBinop(BO_lt, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), TBmakeVar(STRcpy(VARDECL_NAME(vardecl_stop)), vardecl_stop, NULL)),
        TBmakeBinop(BO_gt, TBmakeVar(STRcpy(VARDECL_NAME(vardecl_start)), vardecl_start, NULL), TBmakeVar(STRcpy(VARDECL_NAME(vardecl_stop)), vardecl_stop, NULL)));

    DBUG_RETURN(TBmakeWhile(while_expr, block));
}

node *FLVIvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvarlet");

    node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VARLET_NAME(arg_node));

    if (node)
    {
        VARLET_NAME(arg_node) = STRcpy(LINKEDVALUE_VALUE(node));
    }

    DBUG_RETURN(arg_node);
}

node *FLVIvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("NFLvar");

    node *node = IVLfind(INFO_INDUCTION_VARIABLES(arg_info), VAR_NAME(arg_node));

    if (node)
    {
        VAR_NAME(arg_node) = STRcpy(LINKEDVALUE_VALUE(node));
    }

    DBUG_RETURN(arg_node);
}

node *FLVIinitializeForLoopsVariables(node *syntaxtree)
{
    DBUG_ENTER("FLVIinitializeForLoopsVariables");

    TRAVpush(TR_flvi);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
