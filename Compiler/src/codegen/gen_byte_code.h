#ifndef _GEN_BYTE_CODE_H_
#define _GEN_BYTE_CODE_H_
#include "types.h"

extern node *GBCprogram (node *arg_node, info *arg_info);
extern node *GBCsymboltable (node *arg_node, info *arg_info);
extern node *GBCsymboltableentry (node *arg_node, info *arg_info);
extern node *GBCdecls (node *arg_node, info *arg_info);
extern node *GBCexprs (node *arg_node, info *arg_info);
extern node *GBCarrexpr (node *arg_node, info *arg_info);
extern node *GBCids (node *arg_node, info *arg_info);
extern node *GBCexprstmt (node *arg_node, info *arg_info);
extern node *GBCreturn (node *arg_node, info *arg_info);
extern node *GBCfuncall (node *arg_node, info *arg_info);
extern node *GBCcast (node *arg_node, info *arg_info);
extern node *GBCfundefs (node *arg_node, info *arg_info);
extern node *GBCfundef (node *arg_node, info *arg_info);
extern node *GBCfundecl (node *arg_node, info *arg_info);
extern node *GBCfunbody (node *arg_node, info *arg_info);
extern node *GBCifelse (node *arg_node, info *arg_info);
extern node *GBCwhile (node *arg_node, info *arg_info);
extern node *GBCdowhile (node *arg_node, info *arg_info);
extern node *GBCfor (node *arg_node, info *arg_info);
extern node *GBCglobdef (node *arg_node, info *arg_info);
extern node *GBCglobdecl (node *arg_node, info *arg_info);
extern node *GBCparam (node *arg_node, info *arg_info);
extern node *GBCvardecl (node *arg_node, info *arg_info);
extern node *GBCstmts (node *arg_node, info *arg_info);
extern node *GBCassign (node *arg_node, info *arg_info);
extern node *GBCbinop (node *arg_node, info *arg_info);
extern node *GBCmonop (node *arg_node, info *arg_info);
extern node *GBCvarlet (node *arg_node, info *arg_info);
extern node *GBCvar (node *arg_node, info *arg_info);
extern node *GBCnum (node *arg_node, info *arg_info);
extern node *GBCfloat (node *arg_node, info *arg_info);
extern node *GBCbool (node *arg_node, info *arg_info);
extern node *GBCerror (node *arg_node, info *arg_info);
extern node *GBCternary (node *arg_node, info *arg_info);

extern node *GBCdoGenByteCode( node *syntaxtree);

/** 
 * TODO: remove functions below
 */
extern node *GBClinkedvalue(node *arg_node, info *arg_info);
extern node *GBCcodegentable(node *arg_node, info *arg_info);
extern node *GBCcodegentableentry(node *arg_node, info *arg_info);


#endif
