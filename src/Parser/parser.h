
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <Lexer/lexer.h>

typedef enum
{
        PARSER_BINARY_EXPRESSION,
        PARSER_UNARY_EXPRESSION,
        PARSER_POSTUNARY_EXPRESSION,
        PARSER_TERNARY_EXPRESSION,
        PARSER_BLOCK_STATEMENT,
        PARSER_CALL,

        PARSER_LIT_INTEGER,
        PARSER_LIT_FLOAT,
        PARSER_LIT_STRING,
        PARSER_LIT_CHAR,
        PARSER_LIT_ARRAY,       // {a,b,c,d,...} as expr
        PARSER_SYMBOL,

        PARSER_TYPE,            // e.g. int
        PARSER_TYPECAST,        // e.g. (int)expr

        PARSER_STRUCT,
        PARSER_UNION,
        PARSER_ENUM,
        PARSER_EXTERNAL,        // e.g. extern void _start;

        PARSER_DECLARATION,     // e.g. int main;
        PARSER_FUNCTION,        // e.g. main() {}
        PARSER_INDEXED_ACCESS,  // e.g. a[b]
        PARSER_NAMED_ACCESS,    // e.g. a.b
        
        PARSER_IF,
        PARSER_WHILE,
        PARSER_FOR,
        PARSER_SWITCH,
        PARSER_CASE,
        PARSER_GOTO,
        PARSER_LABEL,
        PARSER_BREAK,
        PARSER_CONTINUE,
        PARSER_RETURN,
} ASTCLAS;

typedef struct _PARSAST
{
        TOKEN            Token;
        struct _PARSAST *Next;
        struct _PARSAST *Prev;
        struct _PARSAST *Parent;
        struct _PARSAST *Children;
        ASTCLAS          Class;
        // removing as to increase generality
} PARSAST;

PARSAST *Parser_ConstructAST(TOKEN *const Tokens, LEXFIL *File);
void     Parser_DestroyAST(PARSAST *);

#endif
