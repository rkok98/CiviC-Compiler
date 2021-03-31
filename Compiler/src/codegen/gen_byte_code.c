#include "globals.h"

#include "gen_byte_code.h"
#include "symbol_table.h"

#include "types.h"
#include "tree_basic.h"
#include "traverse.h"
#include "dbug.h"

#include "ctinfo.h"
#include <stdio.h>
#include "memory.h"
#include "free.h"
#include "str.h"
#include "print.h"
#include <unistd.h>

#include "helpers.h"

struct INFO
{
  FILE *fptr;
  node *code_gen_table;

  node *symbol_table;
  node *symbol_table_entry;

  int constants_counter;
  int branch_count;

  type current_type;
};

#define INFO_FILE(n) ((n)->fptr)
#define INFO_CODE_GEN_TABLE(n) ((n)->code_gen_table)

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_SYMBOL_TABLE_ENTRY(n) ((n)->symbol_table_entry)

#define INFO_LOAD_COUNTER(n) ((n)->constants_counter)
#define INFO_BRANCH_COUNT(n) ((n)->branch_count)

#define INFO_CURRENT_TYPE(n) ((n)->current_type)

static info *MakeInfo()
{
  info *result;

  DBUG_ENTER("MakeInfo");

  result = (info *)MEMmalloc(sizeof(info));

  INFO_FILE(result) = NULL;
  INFO_CODE_GEN_TABLE(result) = TBmakeCodegentable(NULL, NULL, NULL, NULL);

  INFO_SYMBOL_TABLE(result) = NULL;
  INFO_SYMBOL_TABLE_ENTRY(result) = NULL;

  INFO_LOAD_COUNTER(result) = 0;
  INFO_BRANCH_COUNT(result) = 0;

  INFO_CURRENT_TYPE(result) = T_unknown;

  DBUG_RETURN(result);
}

static info *FreeInfo(info *info)
{
  DBUG_ENTER("FreeInfo");

  info = MEMfree(info);

  DBUG_RETURN(info);
}

char *createBranch(const char *name, info *info)
{
  INFO_BRANCH_COUNT(info) += 1;
  char *index = STRitoa(INFO_BRANCH_COUNT(info));
  char *branch = STRcatn(3, index, "_", name);

  free(index);

  return branch;
}

node *SearchPool(node *pool, const char *value)
{
  if (!pool)
  {
    return NULL;
  }

  if (STReq(CODEGENTABLEENTRY_INSTRUCTION(pool), value))
  {
    return pool;
  }

  return SearchPool(CODEGENTABLEENTRY_NEXT(pool), value);
}

node *addToPool(node *pool, node *value)
{
  if (!pool)
  {
    pool = value;
  }
  else
  {
    CODEGENTABLEENTRY_NEXT(pool) = addToPool(CODEGENTABLEENTRY_NEXT(pool), value);
  }

  return pool;
}

const char *typePrefix(type t)
{
  switch (t)
  {
  case T_int:
    return "i";
  case T_float:
    return "f";
  case T_bool:
    return "b";
  case T_void:
    return NULL;
  case T_unknown:
    CTIabort("Unknown type found in file: %s, line: %s", __FILE__, __LINE__);
  }

  return NULL;
} 

node *GBCprogram(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCprogram");

  INFO_SYMBOL_TABLE(arg_info) = PROGRAM_SYMBOLTABLE(arg_node);

  TRAVdo(PROGRAM_DECLS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCsymboltable(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCsymboltable");
  DBUG_RETURN(arg_node);
}

node *GBCsymboltableentry(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCsymboltableentry");
  DBUG_RETURN(arg_node);
}

node *GBCdecls(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCdecls");

  TRAVdo(DECLS_DECL(arg_node), arg_info);
  TRAVopt(DECLS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCexprs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCexprs");

  TRAVdo(EXPRS_EXPR(arg_node), arg_info);
  TRAVopt(EXPRS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCarrexpr(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCarrexpr");
  DBUG_RETURN(arg_node);
}

node *GBCids(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCids");

  TRAVopt(IDS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCexprstmt(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCexprstmt");

  node *expr = EXPRSTMT_EXPR(arg_node);
  TRAVdo(expr, arg_info);

  if (NODE_TYPE(expr) != N_funcall)
  {
    DBUG_RETURN(arg_node);
  }

  node *entry = STdeepSearchFundef(INFO_SYMBOL_TABLE(arg_info), FUNCALL_NAME(expr));
  node *link = SYMBOLTABLEENTRY_DEFINITION(entry);

  if (NODE_TYPE(link) == N_fundecl)
  {
    DBUG_RETURN(arg_node);
  }

  if (typePrefix(SYMBOLTABLEENTRY_TYPE(entry)))
  {
    fprintf(INFO_FILE(arg_info), "\t%spop\n", typePrefix(SYMBOLTABLEENTRY_TYPE(entry)));
  }

  DBUG_RETURN(arg_node);
}

node *GBCreturn(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCreturn");

  TRAVopt(RETURN_EXPR(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "\t%sreturn\n", typePrefix(INFO_CURRENT_TYPE(arg_info)));

  DBUG_RETURN(arg_node);
}

node *GBCfuncall(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfuncall");
  DBUG_PRINT("GBC", ("GBCfuncall"));

  node *entry = STdeepSearchFundef(INFO_SYMBOL_TABLE(arg_info), FUNCALL_NAME(arg_node));

  fprintf(INFO_FILE(arg_info), "\tisrg\n");

  TRAVopt(FUNCALL_ARGS(arg_node), arg_info);

  INFO_CURRENT_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(entry);

  // print
  node *table = SYMBOLTABLEENTRY_TABLE(entry);
  node *link = SYMBOLTABLEENTRY_DEFINITION(entry);

  if (NODE_TYPE(link) == N_fundecl)
  {
    fprintf(INFO_FILE(arg_info), "\tjsre %d\n", SYMBOLTABLEENTRY_OFFSET(entry));
  }
  else
    fprintf(INFO_FILE(arg_info), "\tjsr %ld %s\n", STparams(table), FUNCALL_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *GBCfundefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundefs");
  DBUG_PRINT("GBC", ("GBCfundefs"));

  TRAVdo(FUNDEFS_FUNDEF(arg_node), arg_info);
  TRAVopt(FUNDEFS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCfundecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundecl");

  node *table = INFO_SYMBOL_TABLE(arg_info);

  node *entry = STsearchFundef(table, FUNDECL_NAME(arg_node));

  printf("%d", entry == NULL);

  node *fentry = SYMBOLTABLE_ENTRIES(SYMBOLTABLEENTRY_TABLE(entry));

  char *params = NULL;

  // loop over the entries
  for (; fentry != NULL; fentry = SYMBOLTABLEENTRY_NEXT(fentry))
  {
    // do we have a param entry
    if (!SYMBOLTABLEENTRY_ISPARAMETER(fentry))
    {
      continue;
    }

    // what is this?
    char *temp = STRcatn(3, params, " ", HprintType(SYMBOLTABLEENTRY_TYPE(fentry)));

    params = temp;
  }

  // Create import pool string
  int length = snprintf(
      NULL,
      0,
      "fun \"%s\" %s %s",
      FUNDECL_NAME(arg_node),
      HprintType(FUNDECL_TYPE(arg_node)),
      params == NULL ? "" : params);

  char *str = (char *)malloc(length + 1);

  snprintf(
      str,
      length + 1,
      "fun \"%s\" %s %s",
      FUNDECL_NAME(arg_node),
      HprintType(FUNDECL_TYPE(arg_node)),
      params == NULL ? "" : params);

  node *cgtable_entry = TBmakeCodegentableentry(0, ".import", str, NULL);
  node *cgtable_imports = CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info));

  CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_imports, cgtable_entry);
  free(params);

  DBUG_RETURN(arg_node);
}

node *GBCfundef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundef");
  DBUG_PRINT("GBC", ("GBCfundef"));

  // store the symbol table
  node *table = INFO_SYMBOL_TABLE(arg_info);

  // get the entry from the symbol table
  node *entry = STsearchFundef(table, FUNDEF_NAME(arg_node));

  // print to the file
  fprintf(INFO_FILE(arg_info), "%s:\n", FUNDEF_NAME(arg_node));

  // is this an exported fundef?
  if (FUNDEF_ISEXPORT(arg_node))
  {
    // get the entry
    node *fentry = SYMBOLTABLE_ENTRIES(SYMBOLTABLEENTRY_TABLE(entry));

    char *params = NULL;

    // loop over the entries
    for (; fentry != NULL; fentry = SYMBOLTABLEENTRY_NEXT(fentry))
    {
      // do we have a param entry
      if (!SYMBOLTABLEENTRY_ISPARAMETER(fentry))
        continue;

      char *temp = STRcatn(3, params, " ", HprintType(SYMBOLTABLEENTRY_TYPE(fentry)));
      free(params);

      params = temp;
    }

    // Create export pool string
    int length = snprintf(
        NULL,
        0,
        "fun \"%s\" %s %s %s",
        FUNDEF_NAME(arg_node),
        HprintType(FUNDEF_TYPE(arg_node)),
        params == NULL ? "" : params,
        FUNDEF_NAME(arg_node));

    char *str = (char *)malloc(length + 1);

    snprintf(
        str,
        length + 1,
        "fun \"%s\" %s %s %s",
        FUNDEF_NAME(arg_node),
        HprintType(FUNDEF_TYPE(arg_node)),
        params == NULL ? "" : params,
        FUNDEF_NAME(arg_node));

    node *cgtable_entry = TBmakeCodegentableentry(0, ".export", str, NULL);
    node *cgtable_exports = CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info));

    CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_exports, cgtable_entry);
  }

  // set the symbol table for the upcoming scope
  INFO_SYMBOL_TABLE(arg_info) = FUNDEF_SYMBOLTABLE(arg_node); // nested symbol table

  printf("%s %d", FUNDEF_NAME(arg_node), FUNDEF_ISEXPORT(arg_node));

  // number of registers to use
  size_t registers = STVardecls(INFO_SYMBOL_TABLE(arg_info));

  if (registers > 0)
  {
    // Print amount of function vardecls
    fprintf(
        INFO_FILE(arg_info),
        "\tesr %ld\n",
        registers);
  }

  // traverse over the params and body
  TRAVopt(FUNDEF_PARAMS(arg_node), arg_info);
  TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

  // If return type is void, add 'return' as last instruction to function.
  if (FUNDEF_TYPE(arg_node) == T_void)
    fprintf(INFO_FILE(arg_info), "\t%s\n", "return");

  // reset the symbol table
  INFO_SYMBOL_TABLE(arg_info) = table;

  // print end of line
  fputc('\n', INFO_FILE(arg_info));

  DBUG_RETURN(arg_node);
}

node *GBCfunbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfunbody");
  DBUG_PRINT("GBC", ("GBCfunbody"));

  // iterate over the nodes
  TRAVopt(FUNBODY_VARDECLS(arg_node), arg_info);
  TRAVopt(FUNBODY_STMTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCifelse(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCifelse");
  DBUG_PRINT("GBC", ("GBCifelse"));

  TRAVdo(IFELSE_COND(arg_node), arg_info);

  // the end branch
  char *branch = createBranch(IFELSE_ELSE(arg_node) == NULL ? "end" : "else", arg_info);
  char *end = IFELSE_ELSE(arg_node) != NULL ? createBranch("end", arg_info) : branch;

  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n\n", branch);

  TRAVopt(IFELSE_THEN(arg_node), arg_info);

  if (IFELSE_ELSE(arg_node) != NULL)
  {
    fprintf(INFO_FILE(arg_info), "\tjump %s\n\n", end);
    fprintf(INFO_FILE(arg_info), "%s:\n", branch);
    TRAVopt(IFELSE_ELSE(arg_node), arg_info);
    fputc('\n', INFO_FILE(arg_info));
  }

  fprintf(INFO_FILE(arg_info), "%s:\n", end);

  // free the string
  free(branch);
  if (IFELSE_ELSE(arg_node) != NULL)
    free(end);

  DBUG_RETURN(arg_node);
}

node *GBCwhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCwhile");
  DBUG_PRINT("GBC", ("GBCwhile"));

  // creat the branch name
  char *branch = createBranch("while", arg_info);

  // creat the end branch
  fprintf(INFO_FILE(arg_info), "\n%s:\n", branch);

  // traverse over the conditions
  TRAVdo(WHILE_COND(arg_node), arg_info);

  // creat brach name
  char *end = createBranch("end", arg_info);

  // print to the file
  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n", end);

  // traverse over the block
  TRAVopt(WHILE_BLOCK(arg_node), arg_info);

  // jump back to the beginning of the while loop
  fprintf(INFO_FILE(arg_info), "\tjump %s\n", branch);

  // creat the end branch
  fprintf(INFO_FILE(arg_info), "%s:\n\n", end);

  // free the resources
  free(branch);
  free(end);

  // leap out
  DBUG_RETURN(arg_node);
}

node *GBCdowhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCdowhile");
  DBUG_PRINT("GBC", ("GBCdowhile"));

  // creat the branch name
  char *branch = createBranch("dowhile", arg_info);

  // creat the end branch
  fprintf(INFO_FILE(arg_info), "\n%s:\n", branch);

  TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);
  TRAVdo(DOWHILE_COND(arg_node), arg_info);

  // print to the file
  fprintf(INFO_FILE(arg_info), "\tbranch_t %s\n", branch);

  // free the resources
  free(branch);

  // done
  DBUG_RETURN(arg_node);
}

node *GBCfor(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfor");
  DBUG_PRINT("GBC", ("GBCfor"));

  TRAVdo(FOR_START(arg_node), arg_info);
  TRAVdo(FOR_STOP(arg_node), arg_info);
  TRAVopt(FOR_STEP(arg_node), arg_info);
  TRAVopt(FOR_BLOCK(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCglobdecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCglobdecl");

  char *str = STRcatn(4, "var \"", GLOBDECL_NAME(arg_node), "\" ", HprintType(GLOBDECL_TYPE(arg_node)));

  node *cgtable_entry = TBmakeCodegentableentry(0, ".import", str, NULL);
  node *cgtable_imports = CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info));

  cgtable_imports = addToPool(cgtable_imports, cgtable_entry);

  TRAVopt(GLOBDECL_DIMS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCglobdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCglobdef");
  DBUG_PRINT("GBC", ("GBCglobdef"));

  if (GLOBDEF_ISEXPORT(arg_node))
  {
    node *entry = STdeepSearchVariableByName(INFO_SYMBOL_TABLE(arg_info), GLOBDEF_NAME(arg_node));

    char *offset = STRitoa(SYMBOLTABLEENTRY_OFFSET(entry));
    char *str = STRcatn(4, "var \"", GLOBDEF_NAME(arg_node), "\" ", offset);
    free(offset);

    node *cgtable_entry = TBmakeCodegentableentry(0, ".export", str, NULL);
    node *cgtable_exports = CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info));

    CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_exports, cgtable_entry);
  }

  node *cg_table_globals = CODEGENTABLE_GLOBALS(INFO_CODE_GEN_TABLE(arg_info));
  node *cgtable_entry = TBmakeCodegentableentry(0, ".global ", STRcpy(HprintType(GLOBDEF_TYPE(arg_node))), NULL);

  CODEGENTABLE_GLOBALS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cg_table_globals, cgtable_entry);

  TRAVopt(GLOBDEF_DIMS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCparam(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCparam");
  DBUG_PRINT("GBC", ("GBCparam"));

  TRAVopt(PARAM_DIMS(arg_node), arg_info);
  TRAVopt(PARAM_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCvardecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvardecl");
  DBUG_PRINT("GBC", ("GBCvardecl"));

  TRAVopt(VARDECL_DIMS(arg_node), arg_info);
  TRAVopt(VARDECL_INIT(arg_node), arg_info);
  TRAVopt(VARDECL_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCstmts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCstmts");
  DBUG_PRINT("GBC", ("GBCstmts"));

  TRAVdo(STMTS_STMT(arg_node), arg_info);
  TRAVopt(STMTS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCassign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCassign");
  DBUG_PRINT("GBC", ("GBCassign"));

  TRAVdo(ASSIGN_LET(arg_node), arg_info);
  TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

  node *entry = INFO_SYMBOL_TABLE_ENTRY(arg_info);

  // store count
  char type;
  switch (SYMBOLTABLEENTRY_TYPE(entry))
  {
  case T_int:
    type = 'i';
    break;
  case T_float:
    type = 'f';
    break;
  case T_bool:
    type = 'b';
    break;
  case T_void:
  case T_unknown:
    break;
  }

  if (SYMBOLTABLEENTRY_DEPTH(entry) == 0)
  {
    printf("_-=");
    fprintf(INFO_FILE(arg_info), "\t%cstoreg %d\n", type, SYMBOLTABLEENTRY_OFFSET(entry));
  }
  else
    fprintf(INFO_FILE(arg_info), "\t%cstore %d\n", type, SYMBOLTABLEENTRY_OFFSET(entry));

  // increment the store
  INFO_SYMBOL_TABLE_ENTRY(arg_info) = NULL;

  DBUG_RETURN(arg_node);
}

node *GBCvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvarlet");
  DBUG_PRINT("GBC", ("GBCvarlet"));

  // set the current
  node *table = INFO_SYMBOL_TABLE(arg_info);
  INFO_SYMBOL_TABLE_ENTRY(arg_info) = STdeepSearchVariableByName(table, VARLET_NAME(arg_node));

  DBUG_RETURN(arg_node);
}

node *GBCbinop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCbinop");
  DBUG_PRINT("GBC", ("GBCbinop"));

  TRAVdo(BINOP_LEFT(arg_node), arg_info);
  TRAVdo(BINOP_RIGHT(arg_node), arg_info);

  const char *operation;
  switch (BINOP_OP(arg_node))
  {
  case BO_add:
    operation = "add";
    break;
  case BO_sub:
    operation = "sub";
    break;
  case BO_mul:
    operation = "mul";
    break;
  case BO_div:
    operation = "div";
    break;
  case BO_mod:
    operation = "rem";
    break;
  case BO_lt:
    operation = "lt";
    break;
  case BO_le:
    operation = "le";
    break;
  case BO_gt:
    operation = "gt";
    break;
  case BO_ge:
    operation = "ge";
    break;
  case BO_eq:
    operation = "eq";
    break;
  case BO_ne:
    operation = "ne";
    break;

  case BO_and:
  case BO_or:
  case BO_unknown:
    CTIabortLine(NODE_LINE(arg_node), "Unknown operator type found");
    break;
  }

  switch (INFO_CURRENT_TYPE(arg_info))
  {
  case T_int:
    fprintf(INFO_FILE(arg_info), "\ti%s\n", operation);
    break;
  case T_float:
    fprintf(INFO_FILE(arg_info), "\tf%s\n", operation);
    break;
  case T_bool:
    fprintf(INFO_FILE(arg_info), "\tb%s\n", operation);
    break;
  case T_void:
  case T_unknown:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCmonop");
  DBUG_PRINT("GBC", ("GBCmonop"));

  TRAVdo(MONOP_OPERAND(arg_node), arg_info);

  const char *operation;
  switch (MONOP_OP(arg_node))
  {
  case MO_neg:
    operation = "neg";
    break;
  case MO_not:
    operation = "not";
    break;
  case MO_unknown:
    CTIabort("Unknown operator type found in file: %s, line: %s", __FILE__, __LINE__);
    break;
  }

  switch (INFO_CURRENT_TYPE(arg_info))
  {
  case T_int:
    fprintf(INFO_FILE(arg_info), "\ti%s\n", operation);
    break;
  case T_float:
    fprintf(INFO_FILE(arg_info), "\tf%s\n", operation);
    break;
  case T_bool:
    fprintf(INFO_FILE(arg_info), "\tb%s\n", operation);
    break;
  case T_void:
  case T_unknown:
    break;
  }

  DBUG_RETURN(arg_node);
}

node *GBCcast(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcast");
  DBUG_PRINT("GBC", ("GBCcast"));

  TRAVdo(CAST_EXPR(arg_node), arg_info);

  switch (CAST_TYPE(arg_node))
  {
  case T_int:
    fprintf(INFO_FILE(arg_info), "\tf2i\n");
    break;
  case T_float:
    fprintf(INFO_FILE(arg_info), "\ti2f\n");
    break;
  case T_bool:
    fprintf(INFO_FILE(arg_info), "\tbi2f\n");
    break;
  case T_void:
  case T_unknown:
    break;
  }

  // Set current type to int
  INFO_CURRENT_TYPE(arg_info) = CAST_TYPE(arg_node);

  DBUG_RETURN(arg_node);
}

node *GBCvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvar");
  DBUG_PRINT("GBC", ("GBCvar"));

  DBUG_PRINT("GBC", ("GBCvar 1"));
  node *decl = VAR_DECL(arg_node);

  if (decl == NULL)
  {
    printf("%s", VAR_NAME(arg_node));
  }

  DBUG_PRINT("GBC", ("GBCvar 1.1"));

  node *entry = STdeepSearchByNode(INFO_SYMBOL_TABLE(arg_info), decl);

  // Set current type to int
  INFO_CURRENT_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(entry);

  DBUG_PRINT("GBC", ("GBCvar 2"));

  // is this the global scope?
  if (NODE_TYPE(decl) == N_globdef)
  {
    DBUG_PRINT("GBC", ("GBCvar 2a"));
    // change scope to extern if flag is set.
    char scope = 'g';

    if (GLOBDEF_TYPE(decl) == T_int)
      fprintf(INFO_FILE(arg_info), "\tiload%c %d\n", scope, SYMBOLTABLEENTRY_OFFSET(entry));
    else if (GLOBDEF_TYPE(decl) == T_float)
      fprintf(INFO_FILE(arg_info), "\tfload%c %d\n", scope, SYMBOLTABLEENTRY_OFFSET(entry));
  }
  else if (NODE_TYPE(decl) == N_globdecl)
  {
    DBUG_PRINT("GBC", ("GBCvar 2b"));
    char scope = 'e';

    if (GLOBDECL_TYPE(decl) == T_int)
      fprintf(INFO_FILE(arg_info), "\tiload%c %d\n", scope, SYMBOLTABLEENTRY_OFFSET(entry));
    else if (GLOBDECL_TYPE(decl) == T_float)
      fprintf(INFO_FILE(arg_info), "\tfload%c %d\n", scope, SYMBOLTABLEENTRY_OFFSET(entry));
  }
  else
  {
    DBUG_PRINT("GBC", ("GBCvar 2c"));
    if (SYMBOLTABLEENTRY_TYPE(entry) == T_int)
      fprintf(
          INFO_FILE(arg_info),
          SYMBOLTABLEENTRY_OFFSET(entry) < 4 ? "\tiload_%d\n" : "\tiload %d\n",
          SYMBOLTABLEENTRY_OFFSET(entry));
    else if (SYMBOLTABLEENTRY_TYPE(entry) == T_float)
      fprintf(
          INFO_FILE(arg_info),
          SYMBOLTABLEENTRY_OFFSET(entry) < 4 ? "\tfload_%d\n" : "\tfload %d\n",
          SYMBOLTABLEENTRY_OFFSET(entry));
    else if (SYMBOLTABLEENTRY_TYPE(entry) == T_bool)
      fprintf(
          INFO_FILE(arg_info),
          SYMBOLTABLEENTRY_OFFSET(entry) < 4 ? "\tbload_%d\n" : "\tbload %d\n",
          SYMBOLTABLEENTRY_OFFSET(entry));
  }

  DBUG_PRINT("GBC", ("GBCvar 3"));

  DBUG_RETURN(arg_node);
}

node *GBCnum(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCnum");
  DBUG_PRINT("GBC", ("GBCnum"));

  // Create const pool byte code string
  char *str = STRcat("int ", STRitoa(NUM_VALUE(arg_node)));

  // Search linked list for const value
  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *const_pool = SearchPool(cgtable_constants, str);

  // Add to const pool if it doesn't exist yet.
  // Else extract values from linked list and print to file.
  if (const_pool == NULL)
  {
    node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_COUNTER(arg_info), ".const ", str, NULL);
    fprintf(INFO_FILE(arg_info), "\t%s %d\n", "iloadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

    CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_constants, cgtable_entry);
    INFO_LOAD_COUNTER(arg_info) += 1;
  }
  else
  {
    fprintf(INFO_FILE(arg_info), "\t%s %u\n", "iloadc", CODEGENTABLEENTRY_INDEX(const_pool));
    free(str);
  }

  // Set current type to int
  INFO_CURRENT_TYPE(arg_info) = T_int;

  DBUG_RETURN(arg_node);
}

node *GBCfloat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfloat");
  DBUG_PRINT("GBC", ("GBCfloat"));

  // Create const pool byte code string
  int length = snprintf(NULL, 0, "float %f", FLOAT_VALUE(arg_node));
  char *str = malloc(length + 1);
  snprintf(str, length + 1, "float %f", FLOAT_VALUE(arg_node));

  // Search linked list for const value
  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *const_pool = SearchPool(cgtable_constants, str);

  // Add to const pool if it doesn't exist yet.
  // Else extract values from linked list and print to file.
  if (const_pool == NULL)
  {
    node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_COUNTER(arg_info), ".const ", str, NULL);
    fprintf(INFO_FILE(arg_info), "\t%s %d\n", "floadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

    CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_constants, cgtable_entry);
    INFO_LOAD_COUNTER(arg_info) += 1;
  }
  else
  {
    fprintf(INFO_FILE(arg_info), "\t%s %u\n", "floadc", CODEGENTABLEENTRY_INDEX(const_pool));
    free(str);
  }

  // Set current type to float
  INFO_CURRENT_TYPE(arg_info) = T_float;

  DBUG_RETURN(arg_node);
}

node *GBCbool(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCbool");
  DBUG_PRINT("GBC", ("GBCbool"));

  // Create const pool byte code string
  char *str = STRcat("bool ", BOOL_VALUE(arg_node) ? "true" : "false");

  // Search linked list for const value
  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *const_pool = SearchPool(cgtable_constants, str);

  // Add to const pool if it doesn't exist yet.
  // Else extract values from linked list and print to file.
  if (const_pool == NULL)
  {
    node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_COUNTER(arg_info), ".const ", str, NULL);
    fprintf(INFO_FILE(arg_info), "\t%s %d\n", "bloadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

    CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToPool(cgtable_constants, cgtable_entry);
    INFO_LOAD_COUNTER(arg_info) += 1;
  }
  else
  {
    fprintf(INFO_FILE(arg_info), "\t%s %u\n", "bloadc", CODEGENTABLEENTRY_INDEX(const_pool));
    free(str);
  }

  // Set current type to bool
  INFO_CURRENT_TYPE(arg_info) = T_bool;

  DBUG_RETURN(arg_node);
}

node *GBCerror(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCerror");
  DBUG_PRINT("GBC", ("GBCerror"));

  TRAVopt(ERROR_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCternary(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCternary");
  DBUG_PRINT("GBC", ("GBCternary"));

  // creat the branch name
  char *branch = createBranch("false_expr", arg_info);

  // traverse over the expresion
  TRAVopt(TERNARY_COND(arg_node), arg_info);

  // creat the end branch
  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n", branch);

  // traverse over true branch
  TRAVopt(TERNARY_THEN(arg_node), arg_info);

  // creat the branch name
  char *end = createBranch("end", arg_info);

  // write the jump
  fprintf(INFO_FILE(arg_info), "\tjump %s\n", end);

  // write the end false branch
  fprintf(INFO_FILE(arg_info), "%s:\n", branch);

  // traverse over false branch
  TRAVopt(TERNARY_ELSE(arg_node), arg_info);

  // write the end branch
  fprintf(INFO_FILE(arg_info), "%s:\n", end);

  // done
  DBUG_RETURN(arg_node);
}

/*
 * Traversal start function
 */

node *GBCdoGenByteCode(node *syntaxtree)
{
  DBUG_ENTER("GBCdoGenByteCode");
  DBUG_PRINT("GBC", ("GBCdoGenByteCode"));

  // the output file
  global.outfile = global.outfile ? global.outfile : STRcpy("a.out");

  info *info = MakeInfo();

  INFO_FILE(info) = fopen(global.outfile, "w");

  if (INFO_FILE(info) == NULL)
    CTIabort("Could not open file: %s", global.outfile);

  TRAVpush(TR_gbc);
  syntaxtree = TRAVdo(syntaxtree, info);
  INFO_CODE_GEN_TABLE(info) = TRAVopt(INFO_CODE_GEN_TABLE(info), info);
  TRAVpop();

  // close the file
  fclose(INFO_FILE(info));

  // free the pointer
  FreeInfo(info);

  DBUG_RETURN(syntaxtree);
}

/**
 * TODO: Print to out file
 */
node *GBCcodegentable(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcodegentable");

  CODEGENTABLE_IMPORTS(arg_node) = TRAVopt(CODEGENTABLE_IMPORTS(arg_node), arg_info);
  CODEGENTABLE_CONSTANTS(arg_node) = TRAVopt(CODEGENTABLE_CONSTANTS(arg_node), arg_info);
  CODEGENTABLE_GLOBALS(arg_node) = TRAVopt(CODEGENTABLE_GLOBALS(arg_node), arg_info);
  CODEGENTABLE_EXPORTS(arg_node) = TRAVopt(CODEGENTABLE_EXPORTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCcodegentableentry(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcodegentableentry");

  FILE *fileptr = INFO_FILE(arg_info);

  fprintf(fileptr, "%s%s\n", CODEGENTABLEENTRY_INSTRUCTION(arg_node), CODEGENTABLEENTRY_VALUE(arg_node));

  CODEGENTABLEENTRY_NEXT(arg_node) = TRAVopt(CODEGENTABLEENTRY_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/**
 * TODO: Remove the functions below
 */
node *GBClinkedvalue(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBClinkedvalue");
  DBUG_RETURN(arg_node);
}
