%{


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <memory.h>

#include "types.h"
#include "tree_basic.h"
#include "str.h"
#include "dbug.h"
#include "ctinfo.h"
#include "free.h"
#include "globals.h"

static node *parseresult = NULL;
extern int yylex();
static int yyerror( char *errname);

%}

%union {
 nodetype            nodetype;
 char               *id;
 int                 cint;
 float               cflt;
 bool                cbool;
 binop               cbinop;
 monop               cmonop;
 type                ctype;
 node               *node;
}

%token PARENTHESIS_L PARENTHESIS_R CURLY_L CURLY_R BRACKET_L BRACKET_R COMMA SEMICOLON
%token MINUS PLUS STAR SLASH PERCENT LE LT GE GT EQ NE OR AND NEG
%token LET FLOAT INT BOOL
%token IF ELSE DO WHILE FOR
%token VOID RETURN EXTERN EXPORT
%token OTHER

%token <cint> INTVAL
%token <cflt> FLOATVAL
%token <id> ID
%token <cbool> BOOLVAL

%type <node> constant expr
%type <node> stmts stmt assign varlet program
%type <node> return exprstmt binop exprs monop
%type <node> vardecl fundecl fundef funbody block ifelse
%type <node> decl decls globdecl globdef for dowhile
%type <node> param while

%type <ctype> type

%start program

%right LET
%left OR
%left AND
%left EQ NE
%left LE LT GE GT
%left PLUS MINUS
%left STAR SLASH PERCENT
%right NOT CAST

%nonassoc ID
%nonassoc PARENTHESIS_L
%nonassoc PARENTHESIS_R
%nonassoc ELSE

%%

program: decls 
         {
           parseresult = TBmakeProgram($1, NULL);
         }
        ;

decls: decl decls
        {
            $$ = TBmakeDecls( $1, $2);
        }
    |   decl
        {
            $$ = TBmakeDecls( $1, NULL);
        }
    ;

decl:   fundecl
        {
            $$ = $1;
        }
    |
        fundef
        {
            $$ = $1;
        }
    |   globdef
        {
            $$ = $1;
        }
    |   globdecl
        {
            $$ = $1;
        }
      ;

globdecl: EXTERN type ID SEMICOLON
        {
            $$ = TBmakeGlobdecl($2, STRcpy( $3), NULL);
        }
      ;

globdef: type ID SEMICOLON
        {
            $$ = TBmakeGlobdef($1, STRcpy( $2), NULL, NULL);
        }
    |   type ID LET expr SEMICOLON
        {
            $$ = TBmakeGlobdef($1, STRcpy( $2), NULL, $4);
        }
    |   EXPORT type ID SEMICOLON
        {
            $$ = TBmakeGlobdef($2, STRcpy( $3), NULL, NULL);
            GLOBDEF_ISEXPORT($$) = 1;
        }
    |   EXPORT type ID LET expr SEMICOLON
        {
            $$ = TBmakeGlobdef($2, STRcpy( $3), NULL, $5);
            GLOBDEF_ISEXPORT($$) = 1;
        }
    ;

fundecl: EXTERN type ID PARENTHESIS_L PARENTHESIS_R SEMICOLON
        {
            $$ = TBmakeFundecl( $2, STRcpy( $3), NULL);
        }
    |   EXTERN type ID PARENTHESIS_L param PARENTHESIS_R SEMICOLON
        {
            $$ = TBmakeFundecl( $2, STRcpy( $3), $5);
        }
    ;

fundef: type ID PARENTHESIS_L PARENTHESIS_R  CURLY_L funbody CURLY_R
        {
            $$ = TBmakeFundef( $1, STRcpy( $2), $6, NULL);
        }
    |   type ID PARENTHESIS_L param PARENTHESIS_R CURLY_L funbody CURLY_R
        {
            $$ = TBmakeFundef( $1, STRcpy( $2), $7, $4);
        }
    |   EXPORT type ID PARENTHESIS_L PARENTHESIS_R CURLY_L funbody CURLY_R
        {
            $$ = TBmakeFundef( $2, STRcpy( $3), $7, NULL);
            FUNDEF_ISEXPORT($$) = 1;
        }
    |   EXPORT type ID PARENTHESIS_L param PARENTHESIS_R CURLY_L funbody CURLY_R
        {
            $$ = TBmakeFundef( $2, STRcpy( $3), $8, $5);
            FUNDEF_ISEXPORT($$) = 1;
        }
    ;

param: type ID COMMA param
        {
            $$ = TBmakeParam( STRcpy( $2), $1, NULL, $4);
        }
    |   type ID
        {
            $$ = TBmakeParam( STRcpy( $2), $1, NULL, NULL);
        }
    ;

funbody: 
        {
            $$ = TBmakeFunbody( NULL, NULL, NULL);
        }
    |   vardecl
        {
            $$ = TBmakeFunbody( $1, NULL, NULL);
        }
    |   stmts
        {
            $$ = TBmakeFunbody( NULL, NULL, $1);
        }
    |   vardecl stmts
        {
            $$ = TBmakeFunbody( $1, NULL, $2);
        }
    ;

vardecl: type ID SEMICOLON
        {
            $$ = TBmakeVardecl( STRcpy( $2), $1, NULL, NULL, NULL);
        }
    |   type ID LET expr SEMICOLON
        {
            $$ = TBmakeVardecl( STRcpy( $2), $1, NULL, $4, NULL);
        }
    |   type ID SEMICOLON vardecl
        {
            $$ = TBmakeVardecl( STRcpy( $2), $1, NULL, NULL, $4);
        }
    |   type ID LET expr SEMICOLON vardecl
        {
            $$ = TBmakeVardecl( STRcpy( $2), $1, NULL, $4, $6);
        }
    ;

stmts: stmt stmts
        {
          $$ = TBmakeStmts( $1, $2);
        }
      | stmt
        {
          $$ = TBmakeStmts( $1, NULL);
        }
        ;

stmt: assign
        {
            $$ = $1;
        }
    |   exprstmt
        {
            $$ = $1;
        }
    |   ifelse
        {
            $$ = $1;
        }
    |   while
        {
            $$ = $1;
        }
    |   dowhile
        {
            $$ = $1;
        }
    |   for
        {
            $$ = $1;
        }
    |   return
        {
            $$ = $1;
        }
    ;

ifelse: IF PARENTHESIS_L expr PARENTHESIS_R block
        {
            $$ = TBmakeIfelse($3, $5, NULL);
        }
    |   IF PARENTHESIS_L expr PARENTHESIS_R block ELSE block
        {
            $$ = TBmakeIfelse($3, $5, $7);
        }
    ;

while: WHILE PARENTHESIS_L expr PARENTHESIS_R block
        {
            $$ = TBmakeWhile( $3, $5);
        }
    ;

dowhile: DO block WHILE PARENTHESIS_L expr PARENTHESIS_R SEMICOLON
        {
            $$ = TBmakeDowhile( $5, $2);
        }
    ;

for: FOR PARENTHESIS_L INT ID LET expr COMMA expr PARENTHESIS_R block
        {
            $$ = TBmakeFor( STRcpy( $4), $6, $8, NULL, $10);
        }
    |   FOR PARENTHESIS_L INT ID LET expr COMMA expr COMMA expr PARENTHESIS_R block
        {
            $$ = TBmakeFor( STRcpy( $4), $6, $8, $10, $12);
        }
    ;

block: CURLY_L CURLY_R
        {
            $$ = NULL;
        }
    |   CURLY_L stmts CURLY_R
        {
            $$ = $2;
        }
    |   stmt
        {
            $$ = TBmakeStmts($1, NULL);
        }
    ;

return: RETURN SEMICOLON
        {
            $$ = TBmakeReturn( NULL);
        }
    |   RETURN expr SEMICOLON
        {
            $$ = TBmakeReturn( $2);
        }
    ;

exprstmt: expr SEMICOLON
        {
            $$ = TBmakeExprstmt( $1);
        }
    ;

assign: varlet LET expr SEMICOLON
        {
            $$ = TBmakeAssign( $1, $3);
        }
    ;

varlet: ID
        {
          $$ = TBmakeVarlet( STRcpy( $1), NULL, NULL);
        }
        ;

exprs:  expr COMMA exprs
        {
            $$ = TBmakeExprs($1, $3);
        }
    |   expr
        {
            $$ = TBmakeExprs($1, NULL);
        }
    ;

expr: 
    constant
        {
            $$ = $1;
        }
    |   ID
        {
            $$ = TBmakeVar( STRcpy( $1), NULL, NULL);
        }
    |   binop
        {
            $$ = $1;
        }
    |   monop
        {
            $$ = $1;
        }
    |   PARENTHESIS_L expr PARENTHESIS_R
        {
            $$ = $2;
        }
    |   PARENTHESIS_L type PARENTHESIS_R expr %prec CAST
        {
            $$ = TBmakeCast( $2, $4);
        }
    |   ID PARENTHESIS_L exprs PARENTHESIS_R
        {
            $$ = TBmakeFuncall( STRcpy( $1), NULL, $3);
        }
    |   ID PARENTHESIS_L PARENTHESIS_R
        {
            $$ = TBmakeFuncall( STRcpy( $1), NULL, NULL);
        }
    ;

constant: FLOATVAL
          {
            $$ = $$ = TBmakeFloat( $1);
          }
        | INTVAL
          {
            $$ = TBmakeNum( $1);
          }
        | BOOLVAL
          {
            $$ = TBmakeBool($1);
          }
        ;

monop:
      MINUS expr   { $$ = TBmakeMonop(MO_neg, $2); }
    | NOT expr     { $$ = TBmakeMonop(MO_not, $2); }
    ;

binop: 
       expr PLUS expr      { $$ = TBmakeBinop(BO_add, $1, $3); }
     | expr MINUS expr     { $$ = TBmakeBinop(BO_sub, $1, $3); }
     | expr STAR expr      { $$ = TBmakeBinop(BO_mul, $1, $3); }
     | expr SLASH expr     { $$ = TBmakeBinop(BO_div, $1, $3); }
     | expr PERCENT expr   { $$ = TBmakeBinop(BO_mod, $1, $3); }
     | expr LE expr        { $$ = TBmakeBinop(BO_le,  $1, $3); }
     | expr LT expr        { $$ = TBmakeBinop(BO_lt,  $1, $3); }
     | expr GE expr        { $$ = TBmakeBinop(BO_ge,  $1, $3); }
     | expr GT expr        { $$ = TBmakeBinop(BO_gt,  $1, $3); }
     | expr EQ expr        { $$ = TBmakeBinop(BO_eq,  $1, $3); }
     | expr NE expr        { $$ = TBmakeBinop(BO_ne,  $1, $3); }
     | expr OR expr        { $$ = TBmakeBinop(BO_or,  $1, $3); }
     | expr AND expr       { $$ = TBmakeBinop(BO_and, $1, $3); }
     ;

type: INT        { $$ = T_int;   }
     | FLOAT     { $$ = T_float; }
     | BOOL      { $$ = T_bool;  }
     | VOID      { $$ = T_void;  }
     ;

%%

static int yyerror( char *error)
{
  CTIabort( "line %d, col %d\nError parsing source code: %s\n", 
            global.line, global.col, error);

  return( 0);
}

node *YYparseTree( void)
{
  DBUG_ENTER("YYparseTree");

  yyparse();

  DBUG_RETURN( parseresult);
}

