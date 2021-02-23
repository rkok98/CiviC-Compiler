/*****************************************************************************
 *
 * Module: sum_ints
 *
 * Prefix: SI
 *
 * Description:
 *
 * This module implements a demo traversal of the abstract syntax tree that 
 * sums up all integer constants and prints the result at the end of the traversal.
 *
 *****************************************************************************/


#include "sum_ints.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "memory.h"
#include "ctinfo.h"


/*
 * INFO structure
 */

struct INFO {
  int sum;
};


/*
 * INFO macros
 */

#define INFO_SUM(n)  ((n)->sum)


/*
 * INFO functions
 */

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER( "MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_SUM( result) = 0;

  DBUG_RETURN( result);
}

static info *FreeInfo( info *info)
{
  DBUG_ENTER ("FreeInfo");

  info = MEMfree( info);

  DBUG_RETURN( info);
}


/*
 * Traversal functions
 */

node *SInum (node *arg_node, info *arg_info)
{
  DBUG_ENTER("SInum");

  INFO_SUM( arg_info) += NUM_VALUE(arg_node);

  DBUG_RETURN( arg_node);
}


/*
 * Traversal start function
 */

node *SIdoSumInts( node *syntaxtree)
{
  info *arg_info;

  DBUG_ENTER("SIdoSumIns");

  arg_info = MakeInfo();

  TRAVpush( TR_si);
  syntaxtree = TRAVdo( syntaxtree, arg_info);
  TRAVpop();

  CTInote( "Sum of integer constants: %d", INFO_SUM( arg_info));

  arg_info = FreeInfo( arg_info);

  DBUG_RETURN( syntaxtree);
}
