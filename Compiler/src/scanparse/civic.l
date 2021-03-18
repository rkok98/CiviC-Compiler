%{

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "types.h"
#include "node_basic.h"
#include "str.h"
#include "free.h"
#include "civic.tab.h"
#include "globals.h"
#include "ctinfo.h"
#include "limits.h"


#define FILTER(token) \
  global.col += yyleng;  \
  return( token);

#define LINEBUF_SIZE 256
#define YY_NO_UNPUT
#define YY_NO_INPUT


%}

%p 6000 /* set positions to 6000 */
%e 2000 /* set nodes to 1100 */
%option noyywrap
%option nounput


%%

"("                        { FILTER( PARENTHESIS_L); }
")"                        { FILTER( PARENTHESIS_R); }
"{"                        { FILTER( CURLY_L); }
"}"                        { FILTER( CURLY_R); }
"["                        { FILTER( BRACKET_L); }
"]"                        { FILTER( BRACKET_R); }
","                        { FILTER( COMMA); }
";"                        { FILTER( SEMICOLON); }

"-"                        { FILTER( MINUS); }
"+"                        { FILTER( PLUS);  }
"*"                        { FILTER( STAR);  } 
"/"                        { FILTER( SLASH);  } 
"%"                        { FILTER( PERCENT);  } 

"<="                       { FILTER( LE);    }
"<"                        { FILTER( LT);    } 
">="                       { FILTER( GE);    }
">"                        { FILTER( GT);    }
"=="                       { FILTER( EQ);    }
"!="                       { FILTER( NE);    }
"&&"                       { FILTER( AND);   }
"||"                       { FILTER( OR);    }

"="                        { FILTER( LET);   }

"!"                        { FILTER( NOT);   }

"void"                     { FILTER( VOID); }

"true"                     { yylval.cbool = TRUE;
                             FILTER(BOOLVAL);
                           }
                           
"false"                    { yylval.cbool = FALSE;
                             FILTER(BOOLVAL);
                           }

"int"                      { FILTER( INT); }
"float"                    { FILTER( FLOAT); }
"bool"                     { FILTER( BOOL); }

"extern"                   { FILTER( EXTERN); }
"export"                   { FILTER( EXPORT); }
"return"                   { FILTER( RETURN); }

"if"                       { FILTER( IF); }
"else"                     { FILTER( ELSE); }
"do"                       { FILTER( DO);}
"while"                    { FILTER( WHILE); }
"for"                      { FILTER( FOR); }


[A-Za-z][A-Za-z0-9_]*      { yylval.id = STRcpy(yytext);
                             FILTER( ID);
                           }

[0-9]+                     { long integer = strtol(yytext, NULL, 10);
                             if (integer > INT_MAX || integer < INT_MIN) {
                                CTIerror("Integer %s out of range", yytext);
                             }
                             else
                             {
                                yylval.cint=atoi(yytext);
                                FILTER( INTVAL);
                             }
                           }

[0-9]*[.][0-9]+            { yylval.cflt = atof(yytext);  
                              FILTER( FLOATVAL);
                              printf( "A float: %s (%g)\n", yytext,
                              atof( yytext ) );
                           }                           

\n.*                       { global.line += 1;
                             global.col = 0;
                             yyless(1);
                           }

[ \t]                      { global.col += yyleng;
                           } 

[ \t\n]+                                  // White space
"//".*                                    // Comments
[/][*][^*]*[*]+([^*/][^*]*[*]+)*[/]       { /* DO NOTHING */ }
[/][*]                                    { yy_fatal_error("Unterminated comment"); } 
