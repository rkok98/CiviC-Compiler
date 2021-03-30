#include "transform_boolean_cast.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "copy.h"
#include "dbug.h"

#include "free.h"
#include "memory.h"
#include "ctinfo.h"

#include "symbol_table.h"
#include "helpers.h"

struct INFO
{
  type type;
};

#define INFO_TYPE(n) ((n)->type)

static info *MakeInfo(void)
{
  info *result;

  DBUG_ENTER("MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_TYPE(result) = T_unknown;

  DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
  DBUG_ENTER("FreeInfo");

  info = MEMfree(info);

  DBUG_RETURN(info);
}

node *TBCbinop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCbinop");

  BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);
  BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

  if (HisBooleanOperator(BINOP_OP(arg_node)))
  {
    INFO_TYPE(arg_info) = T_bool;
  }

  DBUG_RETURN(arg_node);
}

node *TBCcast(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCcast");
  
  CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);
  
  if (CAST_TYPE(arg_node) == T_bool)
  {
    node *cast_expression = COPYdoCopy(CAST_EXPR(arg_node));
    FREEdoFreeTree(arg_node);

    if (INFO_TYPE(arg_info) == T_int)
    {
      arg_node = TBmakeBinop(BO_ne, cast_expression, TBmakeNum(FALSE));
    }
    else if (INFO_TYPE(arg_info) == T_float)
    {
      arg_node = TBmakeBinop(BO_ne, cast_expression, TBmakeFloat(0.0));
    }
  }
  else if (INFO_TYPE(arg_info) == T_bool)
  {
    node *cast_expression = COPYdoCopy(CAST_EXPR(arg_node));
    
    if (CAST_TYPE(arg_node) == T_int)
    {
      FREEdoFreeTree(arg_node);
      arg_node = TBmakeTernary(cast_expression, TBmakeNum(TRUE), TBmakeNum(FALSE));
    }
    else if (CAST_TYPE(arg_node) == T_float)
    {
      FREEdoFreeTree(arg_node);
      arg_node = TBmakeTernary(cast_expression, TBmakeFloat(1.0), TBmakeFloat(0.0));
    }
  }

  DBUG_RETURN(arg_node);
}

node *TBCvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCvar");

  node *node = STfindInParents(VAR_SYMBOLTABLE(arg_node), VAR_NAME(arg_node));

  INFO_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(node);

  DBUG_RETURN(arg_node);
}

node *TBCnum(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCint");
  INFO_TYPE(arg_info) = T_int;
  DBUG_RETURN(arg_node);
}

node *TBCfloat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCfloat");
  INFO_TYPE(arg_info) = T_float;
  DBUG_RETURN(arg_node);
}

node *TBCbool(node *arg_node, info *arg_info)
{
  DBUG_ENTER("TBCbool");
  INFO_TYPE(arg_info) = T_bool;
  DBUG_RETURN(arg_node);
}

node *TBCtransformBooleanCast(node *syntaxtree)
{
  DBUG_ENTER("TBCtransformBooleanCast");

  info *info = MakeInfo();

  TRAVpush(TR_tbc);
  syntaxtree = TRAVdo(syntaxtree, info);
  TRAVpop();

  FreeInfo(info);

  DBUG_RETURN(syntaxtree);
}
