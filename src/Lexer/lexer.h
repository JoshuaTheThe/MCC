
#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>

#define IDENTIFIER_SIZE 256

typedef enum
{
        LEXER_TOKEN_EOF,
        LEXER_TOKEN_IDENTIFIER,
        LEXER_TOKEN_INTEGER_LITERAL,
        LEXER_TOKEN_FLOAT_LITERAL,
        LEXER_TOKEN_CHAR_LITERAL,
        LEXER_TOKEN_STRING_LITERAL,
        LEXER_TOKEN_SINGLE_SYMBOL_START,
        LEXER_TOKEN_ADD=LEXER_TOKEN_SINGLE_SYMBOL_START,                // +
        LEXER_TOKEN_SUB,                // -
        LEXER_TOKEN_MUL_PTR,            // *
        LEXER_TOKEN_DIV,                // /
        LEXER_TOKEN_MOD,                // %
        LEXER_TOKEN_BXOR,               // ^
        LEXER_TOKEN_BAND,               // &
        LEXER_TOKEN_BOR,                // |
        LEXER_TOKEN_BNOT,               // ~
        LEXER_TOKEN_NOT,                // !
        LEXER_TOKEN_COMMA,              // ,
        LEXER_TOKEN_DOT,                // .
        LEXER_TOKEN_SEMICOLON,          // ;
        LEXER_TOKEN_COLON,              // :
        LEXER_TOKEN_TERNARY,            // ?
        LEXER_TOKEN_LPEXPR,             // (
        LEXER_TOKEN_RPEXPR,             // )
        LEXER_TOKEN_LBLOCK,             // {
        LEXER_TOKEN_RBLOCK,             // }
        LEXER_TOKEN_LARRAY,             // [
        LEXER_TOKEN_RARRAY,             // ]
        LEXER_TOKEN_GREATER,            // >
        LEXER_TOKEN_LESS,               // <
        LEXER_TOKEN_SET,                // =
        LEXER_TOKEN_PREPROC,            // # nop

        LEXER_TOKEN_DOUBLE_SYMBOL_START,
        LEXER_TOKEN_EQUAL=LEXER_TOKEN_DOUBLE_SYMBOL_START,                // ==
        LEXER_TOKEN_DECLARE_ASSIGN,     // :=
        LEXER_TOKEN_AND,                // &&
        LEXER_TOKEN_OR,                 // ||
        LEXER_TOKEN_XOR,                // ^^ (custom)
        LEXER_TOKEN_ADDSET,             // +=
        LEXER_TOKEN_SUBSET,             // -=
        LEXER_TOKEN_MULSET,             // *=
        LEXER_TOKEN_DIVSET,             // /=
        LEXER_TOKEN_ANDSET,             // &=
        LEXER_TOKEN_ORSET,              // |=
        LEXER_TOKEN_XORSET,             // ^=
        LEXER_TOKEN_MODSET,             // %=
        LEXER_TOKEN_NOTEQ,              // !=
        LEXER_TOKEN_LESSEQ,             // <=
        LEXER_TOKEN_GREATEREQ,          // >=
        LEXER_TOKEN_SHIFTLEFT,          // <<
        LEXER_TOKEN_SHIFTRIGHT,         // >>
        LEXER_TOKEN_ARROW,              // ->
        LEXER_TOKEN_INC,                // ++
        LEXER_TOKEN_DEC,                // --
        LEXER_TOKEN_RANGE,              // ..
        LEXER_TOKEN_COMMENT,            // //
        LEXER_TOKEN_LMULTICOMMENT,      // /*
        LEXER_TOKEN_RMULTICOMMENT,      // */

        LEXER_TOKEN_TRIPLE_SYMBOL_START,
        LEXER_TOKEN_SHIFTLEFTSET=LEXER_TOKEN_TRIPLE_SYMBOL_START,       // <<=
        LEXER_TOKEN_SHIFTRIGHTSET,      // >>=
        LEXER_TOKEN_ROLLLEFT,           // <<<
        LEXER_TOKEN_ROLLRIGHT,          // >>>
        LEXER_TOKEN_VARIADIC,           // ...

        LEXER_TOKEN_KEYWORD_START,
        LEXER_TOKEN_IF=LEXER_TOKEN_KEYWORD_START,
        LEXER_TOKEN_ELSE,
        LEXER_TOKEN_WHILE_FOR,
        LEXER_TOKEN_DO,
        LEXER_TOKEN_RETURN,
        LEXER_TOKEN_UNSIGNED,
        LEXER_TOKEN_SIGNED,
        LEXER_TOKEN_STATIC,
        LEXER_TOKEN_CONST,
        LEXER_TOKEN_AUTO,
        LEXER_TOKEN_CASE,
        LEXER_TOKEN_SWITCH,
        LEXER_TOKEN_BREAK,
        LEXER_TOKEN_CONTINUE,
        LEXER_TOKEN_REGISTER,
        LEXER_TOKEN_UNION,
        LEXER_TOKEN_VOID,
        LEXER_TOKEN_INT,
        LEXER_TOKEN_SHORT,
        LEXER_TOKEN_CHAR,
        LEXER_TOKEN_LONG,
        LEXER_TOKEN_EXTERN,
        LEXER_TOKEN_ENUM,
        LEXER_TOKEN_SIZEOF,
        LEXER_TOKEN_STRUCT,
        LEXER_TOKEN_TYPEDEF,
        LEXER_TOKEN_GOTO,
        LEXER_TOKEN_FLOAT,
        LEXER_TOKEN_DOUBLE,
        LEXER_TOKEN_DEFAULT,
        LEXER_TOKEN_VOLATILE,
        LEXER_TOKEN_OPTIONAL,
        LEXER_TOKEN_SOME,
        LEXER_TOKEN_UNWRAP,
        LEXER_TOKEN_SAFE,         // block specifier
        LEXER_TOKEN_UNSAFE,       // block specifier
        LEXER_TOKEN_PRE_INCLUDE,
        LEXER_TOKEN_PRE_DEFINE,
        LEXER_TOKEN_PRE_IFDEF,        // tell compiler to do some things conditionally
        LEXER_TOKEN_PRE_ELSEIF,       // tell compiler to do some things conditionally
        LEXER_TOKEN_PRE_ENDIF,
        LEXER_TOKEN_PRE_PRAGMA,       // tell compiler to do smth differently
} LEXCLAS;

typedef struct
{
        FILE   *fp;
        size_t  Line,Column,LineOffset;
        size_t  _Line,_Column,_LineOffset;
        char    Identifier[IDENTIFIER_SIZE];
        size_t *LineOffsets;
        size_t  LineCapacity;
        size_t  LineCount;
} LEXFIL;

typedef struct _TOKEN
{
        LEXFIL *File;
        LEXCLAS Class;
        char    Identifier[IDENTIFIER_SIZE];
        int     Number;
        size_t  Line,Column,LineOffset;
        struct _TOKEN *Next;
        struct _TOKEN *Prev;
} TOKEN;

// Caller owns, returns root
TOKEN *Lexer_LexFile(LEXFIL *fp);
void   Lexer_Destroy(TOKEN *Tokens);
TOKEN *Lexer_ConstructNext(TOKEN **Tokens, LEXFIL *fil);
TOKEN  Lexer_Next(LEXFIL *fil);
LEXFIL Lexer_Open(const char *path);
void   Lexer_Close(LEXFIL fil);
void Lexer_IndexLines(LEXFIL *fil);

#endif
