#include "bool_disjunction.h"

#include "copy.h"
#include "dbug.h"
#include "free.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"

extern node *BDCbinop(node *arg_node, info *arg_info)
{
    DBUG_ENTER("BDCbinop");

    BINOP_LEFT(arg_node) = TRAVopt(BINOP_LEFT(arg_node), arg_info);
    BINOP_RIGHT(arg_node) = TRAVopt(BINOP_RIGHT(arg_node), arg_info);

    if (BINOP_OP(arg_node) == BO_or)
    {
        node *binop_right = COPYdoCopy(BINOP_RIGHT(arg_node));
        DBUG_RETURN(TBmakeTernary(COPYdoCopy(BINOP_LEFT(arg_node)), TBmakeBool(TRUE), binop_right));
    }

    if (BINOP_OP(arg_node) == BO_and)
    {
        node *binop_right = COPYdoCopy(BINOP_RIGHT(arg_node));
        DBUG_RETURN(TBmakeTernary(COPYdoCopy(BINOP_LEFT(arg_node)), binop_right, TBmakeBool(FALSE)));
    }

    DBUG_RETURN(arg_node);
}

extern node *BDCdoBoolDisjunction(node *syntaxtree)
{
    DBUG_ENTER("BDCdoBoolDisjunctionAndConjunction");

    TRAVpush(TR_bdc);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}