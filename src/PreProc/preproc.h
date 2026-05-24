
#ifndef PREPROC_H
#define PREPROC_H

#include <Lexer/lexer.h>

typedef struct _MACRO
{
        char *Name;
        TOKEN *Body;
        char **Params;
        int ParamCount;
        struct _MACRO *Next;
} MACRO;

// + remove EOL,CNT (\n,\)
// + create+replace definitions using one line (extended via \)
// + conditional via lazy eval
void PreProcess_Tokens(TOKEN **Tokens);
void PreProcess_Define(MACRO Macro);

#endif

