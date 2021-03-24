#include "context_analysis.h"

#include "string.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

#include "symbol_table.h"

struct INFO
{
    node *table;
};

#define INFO_SYMBOL_TABLE(n) ((n)->table)

static info *MakeInfo(void)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

extern node *CAprogram(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAprogram");

    node *table = TBmakeSymboltable(0, NULL, NULL);

    INFO_SYMBOL_TABLE(arg_info) = table;
    PROGRAM_SYMBOLTABLE(arg_node) = table;

    PROGRAM_DECLS(arg_node) = TRAVopt(PROGRAM_DECLS(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAglobdecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAglobdecl");

    node *table = INFO_SYMBOL_TABLE(arg_info);
    node *entry = TBmakeSymboltableentry(STRcpy(GLOBDECL_NAME(arg_node)), GLOBDECL_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    DBUG_RETURN(arg_node);
}

node *CAglobdef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAglobdef");

    node *table = INFO_SYMBOL_TABLE(arg_info);
    node *entry = TBmakeSymboltableentry(STRcpy(GLOBDEF_NAME(arg_node)), GLOBDEF_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    DBUG_RETURN(arg_node);
}

node *CAparam(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAparam");

    node *table = INFO_SYMBOL_TABLE(arg_info);
    node *entry = TBmakeSymboltableentry(STRcpy(PARAM_NAME(arg_node)), PARAM_TYPE(arg_node), 0, 0, 1, NULL, NULL);

    STinsert(table, entry);

    PARAM_NEXT(arg_node) = TRAVopt(PARAM_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAfundecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAfundecl");

    node *table = INFO_SYMBOL_TABLE(arg_info);
    node *entry = TBmakeSymboltableentry(STRcpy(FUNDECL_NAME(arg_node)), FUNDECL_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    DBUG_RETURN(arg_node);
}

node *CAfundef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAfundef");

    node *parent_table = INFO_SYMBOL_TABLE(arg_info);

    info *fundef_info = MakeInfo();
    node *fundef_table = TBmakeSymboltable(SYMBOLTABLE_NESTINGLEVEL(parent_table) + 1, parent_table, NULL);

    INFO_SYMBOL_TABLE(fundef_info) = fundef_table;
    FUNDEF_SYMBOLTABLE(arg_node) = fundef_table;

    node *entry = TBmakeSymboltableentry(STRcpy(FUNDEF_NAME(arg_node)), FUNDEF_TYPE(arg_node), 1, FUNDEF_ISEXPORT(arg_node), 0, NULL, INFO_SYMBOL_TABLE(fundef_info));

    STinsert(parent_table, entry);

    FUNDEF_PARAMS(arg_node) = TRAVopt(FUNDEF_PARAMS(arg_node), fundef_info);
    FUNDEF_FUNBODY(arg_node) = TRAVopt(FUNDEF_FUNBODY(arg_node), fundef_info);

    fundef_info = FreeInfo(fundef_info);

    DBUG_RETURN(arg_node);
}

node *CAvardecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAvardecl");

    node *table = INFO_SYMBOL_TABLE(arg_info);

    VARDECL_INIT(arg_node) = TRAVopt(VARDECL_INIT(arg_node), arg_info);

    node *entry = TBmakeSymboltableentry(STRcpy(VARDECL_NAME(arg_node)), VARDECL_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

node *CAvarlet(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAvarlet");

    node *varlet_entry = STfindInParents(INFO_SYMBOL_TABLE ( arg_info), VARLET_NAME ( arg_node));

    if (!varlet_entry)
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Undeclared var: %s\n", VAR_NAME(arg_node));
    }
    
    DBUG_RETURN(arg_node);
}

node *CAvar(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAvar");

    node *var_entry = STfindInParents(INFO_SYMBOL_TABLE(arg_info), VAR_NAME(arg_node));

    if (!var_entry)
    {
        CTIerrorLine(NODE_LINE(arg_node) + 1, "Undeclared var: %s\n", VAR_NAME(arg_node));
    }

    VAR_SYMBOLTABLE(arg_node) = INFO_SYMBOL_TABLE(arg_info);

    DBUG_RETURN(arg_node);
}

extern node *CAdoContextAnalysis(node *syntaxtree)
{
    DBUG_ENTER("CAdoContextAnalysis");

    info *arg_info = MakeInfo();

    TRAVpush(TR_ca);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
