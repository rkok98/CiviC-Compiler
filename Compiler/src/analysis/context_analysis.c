#include "context_analysis.h"

#include "string.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"
#include "ctinfo.h"

extern node *CAprogram (node * arg_node, info * arg_info) {
    DBUG_ENTER("CAprogram");
    DBUG_RETURN( arg_node);
}

extern node *CAdoContextAnalysis(node *syntaxtree) {
    DBUG_ENTER("CAdoContextAnalysis");
    DBUG_RETURN( syntaxtree);
}