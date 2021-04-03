#include "print.h"

#include "helpers.h"

#include <stdarg.h>

#include "dbug.h"
#include "globals.h"
#include "memory.h"
#include "traverse.h"
#include "tree_basic.h"

struct INFO
{
  bool firsterror;
  unsigned int indentation_level;
};

#define INFO_FIRSTERROR(n) ((n)->firsterror)
#define INFO_INDENTATION_LEVEL(n) ((n)->indentation_level)

static info *MakeInfo()
{
  info *result;

  result = MEMmalloc(sizeof(info));

  INFO_FIRSTERROR(result) = FALSE;
  INFO_INDENTATION_LEVEL(result) = 0;

  return result;
}

static info *FreeInfo(info *info)
{
  info = MEMfree(info);

  return info;
}

void printIndentations(info *info)
{
  if (info == NULL)
  {
    return;
  }

  for (size_t i = 0; i < INFO_INDENTATION_LEVEL(info); i++)
  {
    printf("\t");
  }
}

void print(info *info, char *fmt, ...)
{
  printIndentations(info);

  va_list args;
  va_start(args, fmt);
  vprintf(fmt, args);
  va_end(args);
}

node *PRTstmts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTstmts");

  STMTS_STMT(arg_node) = TRAVdo(STMTS_STMT(arg_node), arg_info);
  STMTS_NEXT(arg_node) = TRAVopt(STMTS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTassign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTassign");

  if (ASSIGN_LET(arg_node))
  {
    ASSIGN_LET(arg_node) = TRAVdo(ASSIGN_LET(arg_node), arg_info);
    printf(" = ");
  }

  ASSIGN_EXPR(arg_node) = TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

  printf(";\n");

  DBUG_RETURN(arg_node);
}

node *PRTvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvarlet");

  printIndentations(arg_info);
  printf("%s", VARLET_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTprogram(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTprogram");

  if (PROGRAM_SYMBOLTABLE(arg_node))
  {
    printf("/**\n");
    PROGRAM_SYMBOLTABLE(arg_node) = TRAVdo(PROGRAM_SYMBOLTABLE(arg_node), arg_info);
    printf("\n*/\n\n");
  }

  PROGRAM_DECLS(arg_node) = TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTreturn(node *arg_node, info *arg_info)
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

node *PRTexprstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTexprstmt");

  printIndentations(arg_info);

  EXPRSTMT_EXPR(arg_node) = TRAVdo(EXPRSTMT_EXPR(arg_node), arg_info);

  printf(";\n");

  DBUG_RETURN(arg_node);
}

node *PRTarrexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTarrexpr");

  ARREXPR_EXPRS(arg_node) = TRAVdo(ARREXPR_EXPRS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTbinop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTbinop");

  printf("( ");

  BINOP_LEFT(arg_node) = TRAVdo(BINOP_LEFT(arg_node), arg_info);

  printf(" %s ", HprintBinOp(BINOP_OP(arg_node)));

  BINOP_RIGHT(arg_node) = TRAVdo(BINOP_RIGHT(arg_node), arg_info);

  printf(" )");

  DBUG_RETURN(arg_node);
}

node *PRTexprs(node *arg_node, info *arg_info)
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

node *PRTmonop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTmonop");

  printf("%s", HprintMonOp(MONOP_OP(arg_node)));

  MONOP_OPERAND(arg_node) = TRAVdo(MONOP_OPERAND(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTvardecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvardecl");

  printIndentations(arg_info);

  printf("%s %s", HprintType(VARDECL_TYPE(arg_node)), VARDECL_NAME(arg_node));

  if (VARDECL_INIT(arg_node) != NULL)
  {
    printf(" = ");
    VARDECL_INIT(arg_node) = TRAVdo(VARDECL_INIT(arg_node), arg_info);
  }

  printf(";\n");

  VARDECL_NEXT(arg_node) = TRAVopt(VARDECL_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTfundecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundecl");

  printf("extern %s %s", HprintType(FUNDECL_TYPE(arg_node)), FUNDECL_NAME(arg_node));

  printf(" ( ");
  FUNDECL_PARAMS(arg_node) = TRAVopt(FUNDECL_PARAMS(arg_node), arg_info);
  printf(" );\n");

  DBUG_RETURN(arg_node);
}

node *PRTfundefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundefs");

  FUNDEFS_FUNDEF(arg_node) = TRAVdo(FUNDEFS_FUNDEF(arg_node), arg_info);
  FUNDEFS_NEXT(arg_node) = TRAVopt(FUNDEFS_FUNDEF(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTfundef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfundef");

  if (FUNDEF_SYMBOLTABLE(arg_node))
  {
    printf("\n/**\n");
    FUNDEF_SYMBOLTABLE(arg_node) = TRAVdo(FUNDEF_SYMBOLTABLE(arg_node), arg_info);
    printf("\n*/\n");
  }

  if (FUNDEF_ISEXPORT(arg_node))
  {
    printf("%s ", "export");
  }

  printf("%s %s", HprintType(FUNDEF_TYPE(arg_node)), FUNDEF_NAME(arg_node));

  printf(" ( ");
  FUNDEF_PARAMS(arg_node) = TRAVopt(FUNDEF_PARAMS(arg_node), arg_info);
  printf(" ) ");

  if (!FUNDEF_FUNBODY(arg_node))
  {
    printf(";\n");
  }
  else
  {
    print(arg_info, "\n{\n");

    INFO_INDENTATION_LEVEL(arg_info) += 1;

    FUNDEF_FUNBODY(arg_node) = TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

    INFO_INDENTATION_LEVEL(arg_info) -= 1;
    print(arg_info, "}\n");
  }

  DBUG_RETURN(arg_node);
}

node *PRTfunbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfunbody");

  FUNBODY_VARDECLS(arg_node) = TRAVopt(FUNBODY_VARDECLS(arg_node), arg_info);
  FUNBODY_LOCALFUNDEFS(arg_node) = TRAVopt(FUNBODY_LOCALFUNDEFS(arg_node), arg_info);
  FUNBODY_STMTS(arg_node) = TRAVopt(FUNBODY_STMTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTifelse(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTifelse");

  print(arg_info, "if ( ");
  IFELSE_COND(arg_node) = TRAVdo(IFELSE_COND(arg_node), arg_info);
  printf(" )\n");
  print(arg_info, "{\n");

  INFO_INDENTATION_LEVEL(arg_info) += 1;

  IFELSE_THEN(arg_node) = TRAVopt(IFELSE_THEN(arg_node), arg_info);

  INFO_INDENTATION_LEVEL(arg_info) -= 1;

  print(arg_info, "}\n");

  if (IFELSE_ELSE(arg_node))
  {
    print(arg_info, "else\n");
    print(arg_info, "{\n");
    INFO_INDENTATION_LEVEL(arg_info) += 1;

    IFELSE_ELSE(arg_node) = TRAVopt(IFELSE_ELSE(arg_node), arg_info);

    INFO_INDENTATION_LEVEL(arg_info) -= 1;
    print(arg_info, "}\n");
  }

  DBUG_RETURN(arg_node);
}

node *PRTternary(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTternary");

  printf("( ");
  TERNARY_COND(arg_node) = TRAVdo(TERNARY_COND(arg_node), arg_info);
  printf(" ? ");
  TERNARY_THEN(arg_node) = TRAVdo(TERNARY_THEN(arg_node), arg_info);
  printf(" : ");
  TERNARY_ELSE(arg_node) = TRAVdo(TERNARY_ELSE(arg_node), arg_info);
  printf(" )");

  DBUG_RETURN(arg_node);
}

node *PRTdecls(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTdecls");

  DECLS_DECL(arg_node) = TRAVdo(DECLS_DECL(arg_node), arg_info);
  DECLS_NEXT(arg_node) = TRAVopt(DECLS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTglobdecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTglobdecl");

  printf("extern %s %s;\n", HprintType(GLOBDECL_TYPE(arg_node)), GLOBDECL_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTglobdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTglobdef");

  if (GLOBDEF_ISEXPORT(arg_node))
  {
    printf("%s", "export ");
  }

  printf("%s %s", HprintType(GLOBDEF_TYPE(arg_node)), GLOBDEF_NAME(arg_node));

  GLOBDEF_DIMS(arg_node) = TRAVopt(GLOBDEF_DIMS(arg_node), arg_info);

  if (GLOBDEF_INIT(arg_node) != NULL)
  {
    printf(" = ");
    GLOBDEF_INIT(arg_node) = TRAVopt(GLOBDEF_INIT(arg_node), arg_info);
  }

  printf(";\n");

  DBUG_RETURN(arg_node);
}

node *PRTfor(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfor");

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

  INFO_INDENTATION_LEVEL(arg_info)
  ++;

  FOR_BLOCK(arg_node) = TRAVopt(FOR_BLOCK(arg_node), arg_info);

  INFO_INDENTATION_LEVEL(arg_info)
  --;

  print(arg_info, "}\n");

  DBUG_RETURN(arg_node);
}

node *PRTids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTids");

  printf("%s", IDS_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTvar");

  printf("%s", VAR_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTerror(node *arg_node, info *arg_info)
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

node *PRTfuncall(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfuncall");

  printf("%s(", FUNCALL_NAME(arg_node));

  FUNCALL_ARGS(arg_node) = TRAVopt(FUNCALL_ARGS(arg_node), arg_info);

  printf(")");

  DBUG_RETURN(arg_node);
}

node *PRTcast(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTcast");

  printf("(%s)", HprintType(CAST_TYPE(arg_node)));

  CAST_EXPR(arg_node) = TRAVdo(CAST_EXPR(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTwhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTwhile");

  print(arg_info, "while ( ");

  WHILE_COND(arg_node) = TRAVdo(WHILE_COND(arg_node), arg_info);

  printf(" )\n");
  print(arg_info, "{\n");

  INFO_INDENTATION_LEVEL(arg_info)
  ++;

  WHILE_BLOCK(arg_node) = TRAVopt(WHILE_BLOCK(arg_node), arg_info);

  INFO_INDENTATION_LEVEL(arg_info)
  --;

  print(arg_info, "}\n");

  DBUG_RETURN(arg_node);
}

node *PRTdowhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTdowhile");

  print(arg_info, "do\n");
  print(arg_info, "{\n");

  INFO_INDENTATION_LEVEL(arg_info)
  ++;

  DOWHILE_BLOCK(arg_node) = TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);

  INFO_INDENTATION_LEVEL(arg_info)
  --;

  print(arg_info, "}\n");
  print(arg_info, "while ( ");

  DOWHILE_COND(arg_node) = TRAVdo(DOWHILE_COND(arg_node), arg_info);

  printf(" );\n");

  DBUG_RETURN(arg_node);
}

node *PRTparam(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTparam");

  printf("%s %s", HprintType(PARAM_TYPE(arg_node)), PARAM_NAME(arg_node));

  if (PARAM_NEXT(arg_node))
  {
    printf(", ");
    PARAM_NEXT(arg_node) = TRAVopt(PARAM_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTsymboltable(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTsymboltable");

  printf("Symbol Table:\n\n");
  printf("\t%-15s %-10s %-15s %-15s %-15s\n", "Symbol:", "Type:", "Is Function:", "Is Export:", "Is Parameter:");
  SYMBOLTABLE_ENTRIES(arg_node) = TRAVopt(SYMBOLTABLE_ENTRIES(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *PRTsymboltableentry(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTsymboltableentry");

  printf("\t%-15s %-10s %-15s %-15s %-15s\n", SYMBOLTABLEENTRY_NAME(arg_node), HprintType(SYMBOLTABLEENTRY_TYPE(arg_node)), SYMBOLTABLEENTRY_ISFUNCTION(arg_node) ? "True" : "False", SYMBOLTABLEENTRY_ISEXPORT(arg_node) ? "True" : "False", SYMBOLTABLEENTRY_ISPARAMETER(arg_node) ? "True" : "False");

  if (SYMBOLTABLEENTRY_NEXT(arg_node) != NULL)
  {
    SYMBOLTABLEENTRY_NEXT(arg_node) = TRAVdo(SYMBOLTABLEENTRY_NEXT(arg_node), arg_info);
  }

  DBUG_RETURN(arg_node);
}

node *PRTnum(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTnum");

  printf("%i", NUM_VALUE(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTfloat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTfloat");

  printf("%f", FLOAT_VALUE(arg_node));

  DBUG_RETURN(arg_node);
}

node *PRTbool(node *arg_node, info *arg_info)
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

node *PRTlinkedvalue(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTlinkedvalue");
  DBUG_RETURN(arg_node);
}

node *PRTcodegentable(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTlinkedvalue");
  DBUG_RETURN(arg_node);
}

node *PRTcodegentableentry(node *arg_node, info *arg_info)
{
  DBUG_ENTER("PRTlinkedvalue");
  DBUG_RETURN(arg_node);
}

node *PRTdoPrint(node *syntaxtree)
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
