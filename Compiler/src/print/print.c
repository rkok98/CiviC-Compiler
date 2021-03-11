
/**
 * @file print.c
 *
 * Functions needed by print traversal.
 *
 */

/**
 * @defgroup print Print Functions.
 *
 * Functions needed by print traversal.
 *
 * @{
 */

#include <stdarg.h>
#include "print.h"
#include "traverse.h"
#include "tree_basic.h"
#include "dbug.h"
#include "memory.h"
#include "globals.h"

/*
 * INFO structure
 */
struct INFO
{
  bool firsterror;
  size_t indentations;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENTATIONS(n) ((n)->indentations)

static info *MakeInfo()
{
  info *result;

  result = MEMmalloc(sizeof(info));

  INFO_FIRSTERROR(result) = FALSE;
  INFO_INDENTATIONS(result) = 0;

  return result;
}

static info *FreeInfo(info *info)
{
  info = MEMfree(info);

  return info;
}

/**
 * 
 */
void printIndentations(info *info)
{
  if (info == NULL)
  {
    return;
  }

  for (size_t i = 0; i < INFO_INDENTATIONS(info); i++)
  {
    printf("\t");
  }
}

/**
 * 
 */
void print(info *info, char *fmt, ...)
{
  printIndentations(info);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

/** <!--******************************************************************-->
 *
 * @fn stype
 *
 * @brief returns the type specified as a string.
 *
 * @param type the type of the arg_node
 *
 * @return void
 *
 ***************************************************************************/
const char *stype(type type)
{
  switch (type)
  {
  case T_void:
    return "void";
  case T_bool:
    return "bool";
  case T_int:
    return "int";
  case T_float:
    return "float";
  case T_unknown:
    return "unknown";
  }

  return "unknown";
}

/** <!--******************************************************************-->
 *
 * @fn PRTstmts
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTstmts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTstmts");

  STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);

  STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTassign
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTassign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTassign");

  if (ASSIGN_LET(arg_node) != NULL)
  {
    ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
    printf(" = ");
  }

  ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

  printf(";\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTbinop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node BinOp node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTbinop(node *arg_node, info *arg_info)
{
  char *tmp;

  DBUG_ENTER("PRTbinop");

  printf("( ");

  BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);

  switch (BINOP_OP(arg_node))
  {
  case BO_add:
    tmp = "+";
    break;
  case BO_sub:
    tmp = "-";
    break;
  case BO_mul:
    tmp = "*";
    break;
  case BO_div:
    tmp = "/";
    break;
  case BO_mod:
    tmp = "%";
    break;
  case BO_lt:
    tmp = "<";
    break;
  case BO_le:
    tmp = "<=";
    break;
  case BO_gt:
    tmp = ">";
    break;
  case BO_ge:
    tmp = ">=";
    break;
  case BO_eq:
    tmp = "==";
    break;
  case BO_ne:
    tmp = "!=";
    break;
  case BO_or:
    tmp = "||";
    break;
  case BO_and:
    tmp = "&&";
    break;
  case BO_unknown:
    DBUG_ASSERT(0, "unknown binop detected!");
  }

  printf(" %s ", tmp);

  BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

  printf(")");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfloat
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Float node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfloat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfloat");

  printf("%f", FLOAT_VALUE(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTnum
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Num node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTnum(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTnum");

  printf("%i", NUM_VALUE(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTboolean
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node Boolean node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTbool(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTbool");

  if (BOOL_VALUE(arg_node))
  {
    printf("true");
  }
  else
  {
    printf("false");
  }

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTvar
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvar");

  printf("%s", VAR_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTvarlet
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvarlet");

  printf("%s", VARLET_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTerror
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTerror(node *arg_node, info *arg_info)
{
  bool first_error;

  DBUG_ENTER("PRTerror");

  if (NODE_ERROR(arg_node) != NULL)
  {
    NODE_ERROR(arg_node) = TRAVdo(NODE_ERROR(arg_node), arg_info);
  }

  first_error = INFO_FIRSTERROR(arg_info);

  if ((global.outfile != NULL) && (ERROR_ANYPHASE(arg_node) == global.compiler_anyphase))
  {

    if (first_error)
    {
      printf("\n/******* BEGIN TREE CORRUPTION ********\n");
      INFO_FIRSTERROR(arg_info) = FALSE;
    }

    printf("%s\n", ERROR_MESSAGE(arg_node));

    if (ERROR_NEXT(arg_node) != NULL)
    {
      TRAVopt(ERROR_NEXT(arg_node), arg_info);
    }

    if (first_error)
    {
      printf("********  END TREE CORRUPTION  *******/\n");
      INFO_FIRSTERROR(arg_info) = TRUE;
    }
  }

  DBUG_RETURN(arg_node);
}

/**
 * @}
 */
/** <!--******************************************************************-->
 *
 * @fn PRTprogram
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *
PRTprogram(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTprogram");

  PROGRAM_DECLS(arg_node) = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTdecls
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTdecls(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTdecls");

  DECLS_DECL(arg_node) = TRAVdo(DECLS_DECL(arg_node), arg_info);
  DECLS_NEXT(arg_node) = TRAVopt(DECLS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTexprs
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTexprs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTexprs");

  EXPRS_EXPR(arg_node) = TRAVdo(EXPRS_EXPR(arg_node), arg_info);

  if (EXPRS_NEXT(arg_node))
  {
    printf(", ");
    EXPRS_NEXT(arg_node) = TRAVopt(EXPRS_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTarrexpr
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTarrexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTarrexpr");

  ARREXPR_EXPRS(arg_node) = TRAVdo(ARREXPR_EXPRS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTids
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTids");

  printf("%s", IDS_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTexprstmt
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTexprstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTexprstmt");

  printIndentations(arg_info);

  EXPRSTMT_EXPR(arg_node) = TRAVdo(EXPRSTMT_EXPR(arg_node), arg_info);

  printf(";\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTreturn
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTreturn(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTreturn");

  print(arg_info, "return");

  if (RETURN_EXPR(arg_node))
  {
    printf(" ");
    RETURN_EXPR(arg_node) = TRAVopt(RETURN_EXPR(arg_node), arg_info);
  }

  printf(";\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfuncall
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfuncall(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfuncall");

  printf("%s(", FUNCALL_NAME(arg_node));

  FUNCALL_ARGS(arg_node) = TRAVopt(FUNCALL_ARGS(arg_node), arg_info);

  printf(")");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTcast
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTcast(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTcast");

  printf("( %s )", stype(CAST_TYPE(arg_node)));

  CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfundefs
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfundefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundefs");
  DBUG_PRINT("PRT", ("PRTfundefs"));

  FUNDEFS_FUNDEF(arg_node) = TRAVdo(FUNDEFS_FUNDEF(arg_node), arg_info);
  FUNDEFS_NEXT(arg_node) = TRAVopt(FUNDEFS_FUNDEF(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfundef
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfundecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundecl");

  printf("extern %s %s", stype(FUNDECL_TYPE(arg_node)), FUNDECL_NAME(arg_node));

  printf(" ( ");
  FUNDECL_PARAMS(arg_node) = TRAVopt(FUNDECL_PARAMS(arg_node), arg_info);
  printf(" );\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfundef
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfundef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundef");

  if (FUNDEF_ISEXPORT(arg_node))
  {
    printf("%s ", "export");
  }

  printf("%s %s", stype(FUNDEF_TYPE(arg_node)), FUNDEF_NAME(arg_node));

  printf(" ( ");
  FUNDEF_PARAMS(arg_node) = TRAVopt(FUNDEF_PARAMS(arg_node), arg_info);
  printf(" ) ");

  if (FUNDEF_FUNBODY(arg_node) == NULL)
  {
    printf(";\n");
  }
  else
  {
    print(arg_info, "\n{\n");

    INFO_INDENTATIONS(arg_info) += 1;

    FUNDEF_FUNBODY(arg_node) = TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_INDENTATIONS(arg_info) -= 1;
    print(arg_info, "}\n");
  }

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfunbody
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfunbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfunbody");

  FUNBODY_VARDECLS(arg_node) = TRAVopt(FUNBODY_VARDECLS(arg_node), arg_info);
  FUNBODY_LOCALFUNDEFS(arg_node) = TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
  FUNBODY_STMTS(arg_node) = TRAVopt(FUNBODY_STMTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTifelse
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTifelse(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTifelse");

  print(arg_info, "if ( ");
  IFELSE_COND(arg_node) = TRAVdo(IFELSE_COND(arg_node), arg_info);
  printf(" )\n");
  print(arg_info, "{\n");

  INFO_INDENTATIONS(arg_info) += 1;

  IFELSE_THEN(arg_node) = TRAVopt(IFELSE_THEN(arg_node), arg_info);

  INFO_INDENTATIONS(arg_info) -= 1;

  print(arg_info, "}\n");

  if (IFELSE_ELSE(arg_node))
  {
    print(arg_info, "else\n");
    print(arg_info, "{\n");
    INFO_INDENTATIONS(arg_info) += 1;

    IFELSE_ELSE(arg_node) = TRAVopt(IFELSE_ELSE(arg_node), arg_info);

    INFO_INDENTATIONS(arg_info) -= 1;
    print(arg_info, "}\n");
  }

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTwhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTwhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTwhile");

  print(arg_info, "while ( ");

  WHILE_COND(arg_node) = TRAVdo(WHILE_COND(arg_node), arg_info);

  printf(" )\n");
  print(arg_info, "{\n");

  INFO_INDENTATIONS(arg_info)
  ++;

  WHILE_BLOCK(arg_node) = TRAVopt(WHILE_BLOCK(arg_node), arg_info);

  INFO_INDENTATIONS(arg_info)
  --;

  print(arg_info, "}\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTdowhile
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTdowhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTdowhile");

  print(arg_info, "do\n");
  print(arg_info, "{\n");

  INFO_INDENTATIONS(arg_info)
  ++;

  DOWHILE_BLOCK(arg_node) = TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);

  INFO_INDENTATIONS(arg_info)
  --;

  print(arg_info, "}\n");
  print(arg_info, "while ( ");

  DOWHILE_COND(arg_node) = TRAVdo(DOWHILE_COND(arg_node), arg_info);

  printf(" );\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTfor
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTfor(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfor");
  DBUG_PRINT("PRT", ("PRTfor"));

  print(arg_info, "for ( int %s = ", FOR_LOOPVAR(arg_node));

  FOR_START(arg_node) = TRAVdo(FOR_START(arg_node), arg_info);

  printf(", ");
  FOR_STOP(arg_node) = TRAVdo(FOR_STOP(arg_node), arg_info);

  if (FOR_STEP(arg_node) != NULL)
  {
    printf(", ");
    FOR_STEP(arg_node) = TRAVopt(FOR_STEP(arg_node), arg_info);
  }

  printf(")\n");

  print(arg_info, "{\n");

  INFO_INDENTATIONS(arg_info)
  ++;

  FOR_BLOCK(arg_node) = TRAVopt(FOR_BLOCK(arg_node), arg_info);

  INFO_INDENTATIONS(arg_info)
  --;

  print(arg_info, "}\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTglobdecl
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTglobdecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTglobdecl");

  printf("extern %s %s;\n", stype(GLOBDECL_TYPE(arg_node)), GLOBDECL_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTglobdef
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTglobdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTglobdef");
  DBUG_PRINT("PRT", ("PRTglobdef"));

  if (GLOBDEF_ISEXPORT(arg_node) == 1)
  {
    printf("%s", "export ");
  }

  int isExtern = GLOBDEF_ISEXPORT(arg_node) == 1;

  if (isExtern)
  {
    printf("%s", "extern ");
  }

  printf("%s %s", stype(GLOBDEF_TYPE(arg_node)), GLOBDEF_NAME(arg_node));

  GLOBDEF_DIMS(arg_node) = TRAVopt(GLOBDEF_DIMS(arg_node), arg_info);

  if (GLOBDEF_INIT(arg_node) != NULL)
  {
    printf(" = ");
    GLOBDEF_INIT(arg_node) = TRAVopt(GLOBDEF_INIT(arg_node), arg_info);
  }

  printf(";\n");

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTparam
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTparam(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTparam");

  printf("%s %s", stype(PARAM_TYPE(arg_node)), PARAM_NAME(arg_node));

  if (PARAM_NEXT(arg_node))
  {
    printf(", ");
    PARAM_NEXT(arg_node) = TRAVopt(PARAM_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTvardecl
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTvardecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvardecl");

  printIndentations(arg_info);

  printf("%s %s", stype(VARDECL_TYPE(arg_node)), VARDECL_NAME(arg_node));

  if (VARDECL_INIT(arg_node) != NULL)
  {
    printf(" = ");
    VARDECL_INIT(arg_node) = TRAVdo(VARDECL_INIT(arg_node), arg_info);
  }

  printf(";\n");

  VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTmonop
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/

node *
PRTmonop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmonop");
  char *tmp;

  switch (MONOP_OP(arg_node))
  {
  case MO_not:
    tmp = "-";
    break;
  case MO_neg:
    tmp = "!";
    break;
  case MO_unknown:
    DBUG_ASSERT(0, "unknown monop detected!");
  }

  printf("%s", tmp);

  MONOP_OPERAND(arg_node) = TRAVdo(MONOP_OPERAND(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTsymboltable
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *PRTsymboltable(node *arg_node, info *arg_info) {
  DBUG_ENTER("PRTsymboltable");
  DBUG_RETURN(arg_node);
}

/** <!--******************************************************************-->
 *
 * @fn PRTsymboltableentry
 *
 * @brief Prints the node and its sons/attributes
 *
 * @param arg_node letrec node to process
 * @param arg_info pointer to info structure
 *
 * @return processed node
 *
 ***************************************************************************/
node *PRTsymboltableentry(node *arg_node, info *arg_info) {
  DBUG_ENTER("PRTsymboltableentry");
  DBUG_RETURN(arg_node);
}

/** <!-- ****************************************************************** -->
 * @brief Prints the given syntaxtree
 * 
 * @param syntaxtree a node structure
 * 
 * @return the unchanged nodestructure
 ******************************************************************************/

node *
PRTdoPrint(node *syntaxtree)
{
  info *info;

  DBUG_ENTER("PRTdoPrint");

  DBUG_ASSERT((syntaxtree != NULL), "PRTdoPrint called with empty syntaxtree");

  printf("\n\n------------------------------\n\n");

  info = MakeInfo();

  TRAVpush(TR_prt);

  syntaxtree = TRAVdo(syntaxtree, info);

  TRAVpop();

  info = FreeInfo(info);

  printf("\n------------------------------\n\n");

  DBUG_RETURN(syntaxtree);
}
