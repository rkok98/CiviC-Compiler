#include "type_checking.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "str.h"
#include "memory.h"

node *TCprogram(node *arg_node, info *arg_info) {
    DBUG_ENTER("TCprogram");

    DBUG_RETURN(arg_node);
}

node *TCdoTypeChecking(node *syntaxtree)
{
    DBUG_ENTER("TCdoTypeChecking");

    TRAVpush(TR_tc); 

    syntaxtree = TRAVdo(syntaxtree, NULL);

    TRAVpop();

    DBUG_RETURN(syntaxtree);
}