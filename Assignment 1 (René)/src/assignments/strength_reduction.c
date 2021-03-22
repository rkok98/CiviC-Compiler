/*****************************************************************************
 *
 * Module: strength_reduction
 *
 * Prefix: SR
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that
 * implements an optimisation called strength reduction.
 * The traversal converts an multiplier operation to a sequence of additions.
 *
 * Example:
 *      2*k -> k+k
 *      k*2 -> k+k
 *      3*k -> k+k+k
 *      k*3 -> k+k+k
 *
 * Author: René Kok (13671146)
 *         Doorstroomminor Software Engineering UvA
 *****************************************************************************/


#include "strength_reduction.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "free.h"
#include "str.h"

node *SRbinop(node *arg_node, info *arg_info) {
    DBUG_ENTER("SRbinop");

    if (BINOP_OP(arg_node) == BO_mul) {
        int MULTIPLIER;
        node *VAR;

        if (NODE_TYPE(BINOP_LEFT(arg_node)) == N_num
            && (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_var
                || NODE_TYPE(BINOP_RIGHT(arg_node)) == N_varlet)) {
            VAR = BINOP_RIGHT(arg_node);
            MULTIPLIER = NUM_VALUE(BINOP_LEFT(arg_node));
        } else if
           (NODE_TYPE(BINOP_RIGHT(arg_node)) == N_num
            && (NODE_TYPE(BINOP_LEFT(arg_node)) == N_var
                || NODE_TYPE(BINOP_LEFT(arg_node)) == N_varlet)) {
            MULTIPLIER = NUM_VALUE(BINOP_RIGHT(arg_node));
            VAR = BINOP_LEFT(arg_node);
        }
        else {
            // Returns if there's no num and var(let) node
            DBUG_RETURN(arg_node);
        }

        if (MULTIPLIER > 2) {
            BINOP_OP(arg_node) = BO_add;

            if (NODE_TYPE(VAR) == N_var) {
                char *V_NAME = VAR_NAME(VAR);

                BINOP_RIGHT(arg_node) = TBmakeVar(STRcpy(V_NAME));
                BINOP_LEFT(arg_node) = TBmakeBinop(BO_mul, TBmakeNum(MULTIPLIER - 1), TBmakeVar(STRcpy(V_NAME)));
            }

            if (NODE_TYPE(VAR) == N_varlet) {
                char *VL_NAME = VARLET_NAME(VAR);

                BINOP_RIGHT(arg_node) = TBmakeVarlet(STRcpy(VL_NAME));
                BINOP_LEFT(arg_node) = TBmakeBinop(BO_mul, TBmakeNum(MULTIPLIER - 1), TBmakeVarlet(STRcpy(VL_NAME)));
            }
        } else {
            if (NODE_TYPE(VAR) == N_var) {
                char *V_NAME = VAR_NAME(VAR);

                arg_node = TBmakeBinop(BO_add, TBmakeVar(STRcpy(V_NAME)), TBmakeVar(STRcpy(V_NAME)));
            }

            if (NODE_TYPE(VAR) == N_varlet) {
                char *VL_NAME = VARLET_NAME(VAR);

                arg_node = TBmakeBinop(BO_add, TBmakeVar(STRcpy(VL_NAME)), TBmakeVarlet(STRcpy(VL_NAME)));
            }
        }
    }

    BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
    DBUG_RETURN(arg_node);
}

node *SRdoStrengthReduction(node *syntaxtree) {
    DBUG_ENTER("SRdoStrengthReduction");

    TRAVpush(TR_sr);
    syntaxtree = TRAVdo(syntaxtree, NULL);
    TRAVpop();

    DBUG_RETURN(syntaxtree);
}
