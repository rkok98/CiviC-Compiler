#include "context_analysis.h"

#include "string.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

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

extern node *CAdoContextAnalysis(node *syntaxtree)
{
    DBUG_ENTER("CAdoContextAnalysis");

    info *info = MakeInfo(NULL);

    TRAVpush(TR_ca);
    syntaxtree = TRAVdo(syntaxtree, info);
    TRAVpop();

    STdisplay(INFO_SYMBOL_TABLE(info), 0);

    // free the pointer
    FreeInfo(info);

    DBUG_RETURN(syntaxtree);
}