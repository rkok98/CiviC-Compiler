#include "gen_byte_code.h"

#include "helpers.h"
#include "symbol_table.h"

#include "dbug.h"
#include "globals.h"
#include "traverse.h"
#include "tree_basic.h"
#include "types.h"

#include "ctinfo.h"
#include "free.h"
#include "memory.h"
#include "str.h"

struct INFO
{
  FILE *fptr;
  node *code_gen_table;

  node *symbol_table;
  node *symbol_table_entry;

  int load_constants_counter;
  int branch_count;

  type current_type;
};

#define INFO_FILE(n) ((n)->fptr)
#define INFO_CODE_GEN_TABLE(n) ((n)->code_gen_table)

#define INFO_SYMBOL_TABLE(n) ((n)->symbol_table)
#define INFO_SYMBOL_TABLE_ENTRY(n) ((n)->symbol_table_entry)

#define INFO_LOAD_CONSTS_COUNTER(n) ((n)->load_constants_counter)
#define INFO_BRANCH_COUNTER(n) ((n)->branch_count)

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

  INFO_LOAD_CONSTS_COUNTER(result) = 0;
  INFO_BRANCH_COUNTER(result) = 0;

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
  INFO_BRANCH_COUNTER(info) += 1;
  return STRcatn(3, STRitoa(INFO_BRANCH_COUNTER(info)), "_", name);
}

node *addToCGTableEntries(node *entries, node *new_entry)
{
  if (!entries)
  {
    entries = new_entry;
  }
  else
  {
    CODEGENTABLEENTRY_NEXT(entries) = addToCGTableEntries(CODEGENTABLEENTRY_NEXT(entries), new_entry);
  }

  return entries;
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

  fprintf(INFO_FILE(arg_info), "\tisrg\n");

  FUNCALL_ARGS(arg_node) = TRAVopt(FUNCALL_ARGS(arg_node), arg_info);

  node *funcall = STdeepSearchFundef(INFO_SYMBOL_TABLE(arg_info), FUNCALL_NAME(arg_node));
  INFO_CURRENT_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(funcall);

  node *link = SYMBOLTABLEENTRY_DEFINITION(funcall);

  if (NODE_TYPE(link) == N_fundecl)
  {
    fprintf(INFO_FILE(arg_info), "\tjsre %d\n", SYMBOLTABLEENTRY_OFFSET(funcall));
  }
  else
  {
    node *symbol_table = SYMBOLTABLEENTRY_TABLE(funcall);
    fprintf(INFO_FILE(arg_info), "\tjsr %ld %s\n", STparams(symbol_table), FUNCALL_NAME(arg_node));
  }

  DBUG_RETURN(arg_node);
}

node *GBCfundefs(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundefs");

  TRAVdo(FUNDEFS_FUNDEF(arg_node), arg_info);
  TRAVopt(FUNDEFS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCfundecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundecl");

  node *symbol_table = INFO_SYMBOL_TABLE(arg_info);
  node *fundecl = STsearchFundef(symbol_table, FUNDECL_NAME(arg_node));
  node *fundecl_entry = SYMBOLTABLE_ENTRIES(SYMBOLTABLEENTRY_TABLE(fundecl));

  char *fundecl_params = NULL;

  while (fundecl_entry)
  {
    if (SYMBOLTABLEENTRY_ISPARAMETER(fundecl_entry))
    {
      fundecl_params = STRcatn(3, fundecl_params, " ", HprintType(SYMBOLTABLEENTRY_TYPE(fundecl_entry)));
    }

    fundecl_entry = SYMBOLTABLEENTRY_NEXT(fundecl_entry);
  }

  const char *instruction_value = STRcatn(6, "fun \"", FUNDECL_NAME(arg_node), "\" ", HprintType(FUNDECL_TYPE(arg_node)), " ", fundecl_params ? fundecl_params : "");

  node *cgtable_entry = TBmakeCodegentableentry(0, I_import, STRcpy(instruction_value), NULL);
  node *cgtable_imports = CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info));

  CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_imports, cgtable_entry);

  free(fundecl_params);

  DBUG_RETURN(arg_node);
}

node *GBCfundef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfundef");

  fprintf(INFO_FILE(arg_info), "%s:\n", FUNDEF_NAME(arg_node));

  node *symbol_table = INFO_SYMBOL_TABLE(arg_info);
  node *fundef = STsearchFundef(symbol_table, FUNDEF_NAME(arg_node));

  if (FUNDEF_ISEXPORT(arg_node))
  {
    node *fundef_entries = SYMBOLTABLE_ENTRIES(SYMBOLTABLEENTRY_TABLE(fundef));

    char *fundef_params = NULL;

    while (fundef_entries)
    {
      if (SYMBOLTABLEENTRY_ISPARAMETER(fundef_entries))
      {
        fundef_params = STRcatn(3, fundef_params, " ", HprintType(SYMBOLTABLEENTRY_TYPE(fundef_entries)));
      }

      fundef_entries = SYMBOLTABLEENTRY_NEXT(fundef_entries);
    }

    const char *instruction_value = STRcatn(8, "fun \"", FUNDEF_NAME(arg_node), "\" ", HprintType(FUNDEF_TYPE(arg_node)), " ", fundef_params ? fundef_params : "", " ", FUNDEF_NAME(arg_node));

    node *cgtable_entry = TBmakeCodegentableentry(0, I_export, STRcpy(instruction_value), NULL);
    node *cgtable_exports = CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info));

    CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_exports, cgtable_entry);
  }

  printf("%s %d", FUNDEF_NAME(arg_node), FUNDEF_ISEXPORT(arg_node));
  INFO_SYMBOL_TABLE(arg_info) = FUNDEF_SYMBOLTABLE(arg_node);

  unsigned int registers = STVardecls(INFO_SYMBOL_TABLE(arg_info));

  if (registers)
  {
    fprintf(INFO_FILE(arg_info), "\tesr %ld\n", registers);
  }

  TRAVopt(FUNDEF_PARAMS(arg_node), arg_info);
  TRAVopt(FUNDEF_FUNBODY(arg_node), arg_info);

  if (FUNDEF_TYPE(arg_node) == T_void)
  {
    fprintf(INFO_FILE(arg_info), "\t%s\n", "return");
  }

  INFO_SYMBOL_TABLE(arg_info) = symbol_table;

  fputc('\n', INFO_FILE(arg_info));

  DBUG_RETURN(arg_node);
}

node *GBCfunbody(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfunbody");
  DBUG_PRINT("GBC", ("GBCfunbody"));

  TRAVopt(FUNBODY_VARDECLS(arg_node), arg_info);
  TRAVopt(FUNBODY_STMTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCifelse(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCifelse");

  TRAVdo(IFELSE_COND(arg_node), arg_info);

  char *ifelse_branch = createBranch(IFELSE_ELSE(arg_node) ? "else" : "end", arg_info);
  char *ifelse_end = IFELSE_ELSE(arg_node) ? createBranch("end", arg_info) : ifelse_branch;

  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n\n", ifelse_branch);

  TRAVopt(IFELSE_THEN(arg_node), arg_info);

  if (IFELSE_ELSE(arg_node))
  {
    fprintf(INFO_FILE(arg_info), "\tjump %s\n\n", ifelse_end);
    fprintf(INFO_FILE(arg_info), "%s:\n", ifelse_branch);
    TRAVopt(IFELSE_ELSE(arg_node), arg_info);
    fputc('\n', INFO_FILE(arg_info));
  }

  fprintf(INFO_FILE(arg_info), "%s:\n", ifelse_end);

  free(ifelse_branch);

  if (IFELSE_ELSE(arg_node))
  {
    free(ifelse_end);
  }

  DBUG_RETURN(arg_node);
}

node *GBCwhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCwhile");

  char *while_branch = createBranch("while", arg_info);
  char *while_end = createBranch("end", arg_info);

  fprintf(INFO_FILE(arg_info), "\n%s:\n", while_branch);
  TRAVdo(WHILE_COND(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n", while_end);
  TRAVopt(WHILE_BLOCK(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "\tjump %s\n", while_branch);
  fprintf(INFO_FILE(arg_info), "%s:\n\n", while_end);

  free(while_branch);
  free(while_end);

  DBUG_RETURN(arg_node);
}

node *GBCdowhile(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCdowhile");

  char *dowhile_branch = createBranch("dowhile", arg_info);

  fprintf(INFO_FILE(arg_info), "\n%s:\n", dowhile_branch);

  TRAVopt(DOWHILE_BLOCK(arg_node), arg_info);
  TRAVdo(DOWHILE_COND(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "\tbranch_t %s\n", dowhile_branch);

  free(dowhile_branch);

  DBUG_RETURN(arg_node);
}

node *GBCfor(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfor");

  TRAVdo(FOR_START(arg_node), arg_info);
  TRAVdo(FOR_STOP(arg_node), arg_info);
  TRAVopt(FOR_STEP(arg_node), arg_info);
  TRAVopt(FOR_BLOCK(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCglobdecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCglobdecl");

  char *instructions_value = STRcatn(4, "var \"", GLOBDECL_NAME(arg_node), "\" ", HprintType(GLOBDECL_TYPE(arg_node)));

  node *cgtable_entry = TBmakeCodegentableentry(0, I_import, instructions_value, NULL);
  node *cgtable_imports = CODEGENTABLE_IMPORTS(INFO_CODE_GEN_TABLE(arg_info));

  cgtable_imports = addToCGTableEntries(cgtable_imports, cgtable_entry);

  TRAVopt(GLOBDECL_DIMS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCglobdef(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCglobdef");

  if (GLOBDEF_ISEXPORT(arg_node))
  {
    node *globdef = STdeepSearchVariableByName(INFO_SYMBOL_TABLE(arg_info), GLOBDEF_NAME(arg_node));

    char *globdef_offset = STRitoa(SYMBOLTABLEENTRY_OFFSET(globdef));
    char *instructions_value = STRcatn(4, "var \"", GLOBDEF_NAME(arg_node), "\" ", globdef_offset);

    node *cgtable_entry = TBmakeCodegentableentry(0, I_export, instructions_value, NULL);
    node *cgtable_exports = CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info));

    CODEGENTABLE_EXPORTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_exports, cgtable_entry);

    free(globdef_offset);
  }

  node *cg_table_globals = CODEGENTABLE_GLOBALS(INFO_CODE_GEN_TABLE(arg_info));
  node *cgtable_entry = TBmakeCodegentableentry(0, I_global, STRcpy(HprintType(GLOBDEF_TYPE(arg_node))), NULL);

  CODEGENTABLE_GLOBALS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cg_table_globals, cgtable_entry);

  TRAVopt(GLOBDEF_DIMS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCparam(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCparam");

  TRAVopt(PARAM_DIMS(arg_node), arg_info);
  TRAVopt(PARAM_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCvardecl(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvardecl");

  TRAVopt(VARDECL_DIMS(arg_node), arg_info);
  TRAVopt(VARDECL_INIT(arg_node), arg_info);
  TRAVopt(VARDECL_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCstmts(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCstmts");

  TRAVdo(STMTS_STMT(arg_node), arg_info);
  TRAVopt(STMTS_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCassign(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCassign");

  TRAVdo(ASSIGN_LET(arg_node), arg_info);
  TRAVdo(ASSIGN_EXPR(arg_node), arg_info);

  node *assign_entry = INFO_SYMBOL_TABLE_ENTRY(arg_info);

  if (SYMBOLTABLEENTRY_DEPTH(assign_entry) == 0)
  {
    fprintf(INFO_FILE(arg_info), "\t%sstoreg %d\n", typePrefix(SYMBOLTABLEENTRY_TYPE(assign_entry)), SYMBOLTABLEENTRY_OFFSET(assign_entry));
  }
  else
  {
    fprintf(INFO_FILE(arg_info), "\t%sstore %d\n", typePrefix(SYMBOLTABLEENTRY_TYPE(assign_entry)), SYMBOLTABLEENTRY_OFFSET(assign_entry));
  }

  DBUG_RETURN(arg_node);
}

node *GBCvarlet(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvarlet");

  node *symbol_table = INFO_SYMBOL_TABLE(arg_info);
  INFO_SYMBOL_TABLE_ENTRY(arg_info) = STdeepSearchVariableByName(symbol_table, VARLET_NAME(arg_node));

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

  if (typePrefix(INFO_CURRENT_TYPE(arg_info)))
  {
    fprintf(INFO_FILE(arg_info), "\t%s%s\n", typePrefix(INFO_CURRENT_TYPE(arg_info)), operation);
  }

  DBUG_RETURN(arg_node);
}

node *GBCmonop(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCmonop");

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

  if (typePrefix(INFO_CURRENT_TYPE(arg_info)))
  {
    fprintf(INFO_FILE(arg_info), "\t%s%s\n", typePrefix(INFO_CURRENT_TYPE(arg_info)), operation);
  }

  DBUG_RETURN(arg_node);
}

node *GBCcast(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcast");

  TRAVdo(CAST_EXPR(arg_node), arg_info);

  if (INFO_CURRENT_TYPE(arg_info) != CAST_TYPE(arg_node))
  {

    if (CAST_TYPE(arg_node) == T_int)
    {
      fprintf(INFO_FILE(arg_info), "\tf2i\n");
    }
    else if (CAST_TYPE(arg_node) == T_float)
    {
      fprintf(INFO_FILE(arg_info), "\ti2f\n");
    }
  }

  INFO_CURRENT_TYPE(arg_info) = CAST_TYPE(arg_node);

  DBUG_RETURN(arg_node);
}

node *GBCvar(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCvar");

  node *var_decl = VAR_DECL(arg_node);
  node *vardecl_entry = STdeepSearchByNode(INFO_SYMBOL_TABLE(arg_info), var_decl);

  INFO_CURRENT_TYPE(arg_info) = SYMBOLTABLEENTRY_TYPE(vardecl_entry);

  if (NODE_TYPE(var_decl) == N_globdef)
  {
    if (GLOBDEF_TYPE(var_decl) == T_int)
      fprintf(INFO_FILE(arg_info), "\tiloadg %d\n", SYMBOLTABLEENTRY_OFFSET(vardecl_entry));
    else if (GLOBDEF_TYPE(var_decl) == T_float)
      fprintf(INFO_FILE(arg_info), "\tfloadg %d\n", SYMBOLTABLEENTRY_OFFSET(vardecl_entry));
  }
  else if (NODE_TYPE(var_decl) == N_globdecl)
  {
    if (GLOBDECL_TYPE(var_decl) == T_int)
      fprintf(INFO_FILE(arg_info), "\tiloade %d\n", SYMBOLTABLEENTRY_OFFSET(vardecl_entry));
    else if (GLOBDECL_TYPE(var_decl) == T_float)
      fprintf(INFO_FILE(arg_info), "\tfloade %d\n", SYMBOLTABLEENTRY_OFFSET(vardecl_entry));
  }
  else
  {
    fprintf(INFO_FILE(arg_info), "\t%sload %d\n", typePrefix(SYMBOLTABLEENTRY_TYPE(vardecl_entry)), SYMBOLTABLEENTRY_OFFSET(vardecl_entry));
  }

  DBUG_RETURN(arg_node);
}

node *GBCnum(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCnum");

  char *instruction_value = STRcat("int ", STRitoa(NUM_VALUE(arg_node)));

  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_CONSTS_COUNTER(arg_info), I_constant, instruction_value, NULL);
  
  fprintf(INFO_FILE(arg_info), "\t%s %d\n", "iloadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

  CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_constants, cgtable_entry);
  INFO_LOAD_CONSTS_COUNTER(arg_info) += 1;

  INFO_CURRENT_TYPE(arg_info) = T_int;

  DBUG_RETURN(arg_node);
}

node *GBCfloat(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCfloat");

  char *instruction_value = STRcat("float ", STRitoa(FLOAT_VALUE(arg_node)));

  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_CONSTS_COUNTER(arg_info), I_constant, instruction_value, NULL);
  
  fprintf(INFO_FILE(arg_info), "\t%s %d\n", "floadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

  CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_constants, cgtable_entry);
  INFO_LOAD_CONSTS_COUNTER(arg_info) += 1;

  INFO_CURRENT_TYPE(arg_info) = T_float;

  DBUG_RETURN(arg_node);
}

node *GBCbool(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCbool");

  char *instruction_value = STRcat("bool ", BOOL_VALUE(arg_node) ? "true" : "false");

  node *cgtable_constants = CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info));
  node *cgtable_entry = TBmakeCodegentableentry(INFO_LOAD_CONSTS_COUNTER(arg_info), I_constant, instruction_value, NULL);
  
  fprintf(INFO_FILE(arg_info), "\t%s %d\n", "bloadc", CODEGENTABLEENTRY_INDEX(cgtable_entry));

  CODEGENTABLE_CONSTANTS(INFO_CODE_GEN_TABLE(arg_info)) = addToCGTableEntries(cgtable_constants, cgtable_entry);
  INFO_LOAD_CONSTS_COUNTER(arg_info) += 1;

  INFO_CURRENT_TYPE(arg_info) = T_bool;

  DBUG_RETURN(arg_node);
}

node *GBCerror(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCerror");

  ERROR_NEXT(arg_node) = TRAVopt(ERROR_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBCternary(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCternary");

  char *false_branch = createBranch("false_expr", arg_info);

  TERNARY_COND(arg_node) = TRAVopt(TERNARY_COND(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "\tbranch_f %s\n", false_branch);

  TERNARY_THEN(arg_node) = TRAVopt(TERNARY_THEN(arg_node), arg_info);
  char *end_branch = createBranch("end", arg_info);

  fprintf(INFO_FILE(arg_info), "\tjump %s\n", end_branch);
  fprintf(INFO_FILE(arg_info), "%s:\n", false_branch);

  TERNARY_ELSE(arg_node) = TRAVopt(TERNARY_ELSE(arg_node), arg_info);

  fprintf(INFO_FILE(arg_info), "%s:\n", end_branch);

  DBUG_RETURN(arg_node);
}

node *GBCcodegentable(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcodegentable");

  CODEGENTABLE_IMPORTS(arg_node) = TRAVopt(CODEGENTABLE_IMPORTS(arg_node), arg_info);
  CODEGENTABLE_CONSTANTS(arg_node) = TRAVopt(CODEGENTABLE_CONSTANTS(arg_node), arg_info);
  CODEGENTABLE_GLOBALS(arg_node) = TRAVopt(CODEGENTABLE_GLOBALS(arg_node), arg_info);
  CODEGENTABLE_EXPORTS(arg_node) = TRAVopt(CODEGENTABLE_EXPORTS(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

/**
 * Prints the imports, constants, globals and exports from the codegen table.
 */
node *GBCcodegentableentry(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBCcodegentableentry");

  FILE *fileptr = INFO_FILE(arg_info);

  const char *pseudo_instruction;

  switch (CODEGENTABLEENTRY_INSTRUCTION(arg_node))
  {
  case I_constant:
    pseudo_instruction = ".const ";
    break;
  case I_export:
    pseudo_instruction = ".export";
    break;
  case I_global:
    pseudo_instruction = ".global ";
    break;
  case I_import:
    pseudo_instruction = ".import";
    break;
  case I_unknown:
  default:
    DBUG_RETURN(arg_node);
  }

  fprintf(fileptr, "%s%s\n", pseudo_instruction, CODEGENTABLEENTRY_VALUE(arg_node));

  CODEGENTABLEENTRY_NEXT(arg_node) = TRAVopt(CODEGENTABLEENTRY_NEXT(arg_node), arg_info);

  DBUG_RETURN(arg_node);
}

node *GBClinkedvalue(node *arg_node, info *arg_info)
{
  DBUG_ENTER("GBClinkedvalue");
  DBUG_RETURN(arg_node);
}

node *GBCdoGenByteCode(node *syntaxtree)
{
  DBUG_ENTER("GBCdoGenByteCode");

  info *arg_info = MakeInfo();

  if (global.outfile)
  {
    INFO_FILE(arg_info) = fopen(global.outfile, "w");
  }
  else
  {
    INFO_FILE(arg_info) = stdout;
  }

  TRAVpush(TR_gbc);

  syntaxtree = TRAVdo(syntaxtree, arg_info);
  INFO_CODE_GEN_TABLE(arg_info) = TRAVopt(INFO_CODE_GEN_TABLE(arg_info), arg_info);

  TRAVpop();

  FreeInfo(arg_info);

  DBUG_RETURN(syntaxtree);
}
