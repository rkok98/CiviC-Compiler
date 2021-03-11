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

static info *MakeInfo(node *parent)
{
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *)MEMmalloc(sizeof(info));

    node *table = TBmakeSymboltable(NULL, NULL);
    INFO_SYMBOL_TABLE(result) = table;

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

    PROGRAM_SYMBOLTABLE(arg_node) = INFO_SYMBOL_TABLE(arg_info);
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

node *CAfundecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAfundecl");

    // TODO: Do I need to create a symbol table for fundecl?

    node *table = INFO_SYMBOL_TABLE(arg_info);
    node *entry = TBmakeSymboltableentry(STRcpy(FUNDECL_NAME(arg_node)), FUNDECL_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    DBUG_RETURN(arg_node);
}

node *CAfundef(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAfundef");

    node *table = INFO_SYMBOL_TABLE(arg_info);

    info *fundef_info = MakeInfo(table);

    node *entry = TBmakeSymboltableentry(STRcpy(FUNDEF_NAME(arg_node)), FUNDEF_TYPE(arg_node), 0, 0, 0, NULL, INFO_SYMBOL_TABLE(fundef_info));

    STinsert(table, entry);

    FUNDEF_PARAMS(arg_node) = TRAVopt(FUNDEF_PARAMS(arg_node), fundef_info);
    FUNDEF_FUNBODY(arg_node) = TRAVopt(FUNDEF_FUNBODY(arg_node), fundef_info);

    fundef_info = FreeInfo(fundef_info);

    DBUG_RETURN(arg_node);
}

node *CAvardecl(node *arg_node, info *arg_info)
{
    DBUG_ENTER("CAvardecl");

    node *table = INFO_SYMBOL_TABLE ( arg_info);

    if (VARDECL_INIT ( arg_node)) {
        VARDECL_INIT ( arg_node) = TRAVopt(VARDECL_INIT ( arg_node), arg_info);
    }

    node *entry = TBmakeSymboltableentry(STRcpy(VARDECL_NAME(arg_node)), VARDECL_TYPE(arg_node), 0, 0, 0, NULL, NULL);

    STinsert(table, entry);

    VARDECL_NEXT( arg_node) = TRAVopt ( VARDECL_NEXT( arg_node), arg_info);

    DBUG_RETURN(arg_node);
}

extern node *CAdoContextAnalysis(node *syntaxtree)
{
    DBUG_ENTER("CAdoContextAnalysis");

    info *arg_info = MakeInfo(NULL);

    TRAVpush(TR_ca);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    STprint(INFO_SYMBOL_TABLE(arg_info), 0);

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}