/*****************************************************************************
 *
 * Module: count_ids
 *
 * Prefix: CID
 *
 * Description:
 *
 * This module implements a traversal of the abstract syntax tree that
 * counts the number of occurrences of each identifier.
 * After the traversal, the identifiers are printed with the number they occur.
 *
 * Author: René Kok (13671146)
 *         Doorstroomminor Software Engineering UvA
 *
 *****************************************************************************/

#include "count_ids.h"

#include "types.h"
#include "tree_basic.h"
#include "lookup_table.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"

#include "str.h"

struct INFO {
    lut_t *table;
};

struct COUNTER {
    char *id;
    int amount;
};

typedef struct COUNTER counter;

#define INFO_LUT(n) ((n)->table)
#define COUNTER_ID(n) ((n)->id)
#define COUNTER_AMOUNT(n) ((n)->amount)

static info *MakeInfo(void) {
    info *result;

    DBUG_ENTER("MakeInfo");

    result = (info *) MEMmalloc(sizeof(info));

    INFO_LUT(result) = LUTgenerateLut();

    DBUG_RETURN(result);
}

static info *FreeInfo(info *info) {
    DBUG_ENTER("FreeInfo");

    info = MEMfree(info);

    DBUG_RETURN(info);
}

static void *PrintLut(void *item) {
    DBUG_ENTER("printLut");

    counter *c = (counter *) item;
    CTInote("Identifier %s occurs %d time(s)", COUNTER_ID(c), COUNTER_AMOUNT(c));

    DBUG_RETURN(item);
}

static void *InsertCounter(lut_t *LUT, char *id) {
    DBUG_ENTER("Insert");

    void **S = LUTsearchInLutS(LUT, id);

    if (S != NULL) {
        counter *counter = *S;
        COUNTER_AMOUNT(counter) += 1;
    } else {
        counter *c = (counter *) MEMmalloc(sizeof(counter));

        COUNTER_AMOUNT(c) = 1;
        COUNTER_ID(c) = id;

        LUTinsertIntoLutS(LUT, id, c);
    }

    DBUG_RETURN(id);
}

node *CIDvar(node *arg_node, info *arg_info) {
    DBUG_ENTER("CIDvar");

    lut_t *LUT = INFO_LUT(arg_info);
    char *ID = STRcpy(VAR_NAME(arg_node));

    InsertCounter(LUT, ID);

    DBUG_RETURN(arg_node);
}

node *CIDvarlet(node *arg_node, info *arg_info) {
    DBUG_ENTER("CIDvarlet");

    lut_t *LUT = INFO_LUT(arg_info);
    char *ID = STRcpy(VARLET_NAME(arg_node));

    InsertCounter(LUT, ID);

    DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */
node *CIDdoCountIds(node *syntaxtree) {
    info *arg_info;

    DBUG_ENTER("CIDdoCountIds");

    arg_info = MakeInfo();

    TRAVpush(TR_cid);
    syntaxtree = TRAVdo(syntaxtree, arg_info);
    TRAVpop();

    lut_t *LUT = INFO_LUT(arg_info);
    LUTmapLutS(LUT, PrintLut);

    arg_info = FreeInfo(arg_info);

    DBUG_RETURN(syntaxtree);
}
