
#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>
#include <stdbool.h>
#include <Lexer/lexer.h>

typedef struct _PARSAST
{
        TOKEN            Token;
        struct _PARSAST *Next;
        struct _PARSAST *Prev;
        struct _PARSAST *Parent;
        union
        {
                struct
                {
                        struct _PARSAST *Expr;
                } Return;
                
                struct
                {
                        struct _PARSAST *Label;
                } GoTo;
                
                struct
                {
                        int RegisterIndex;
                } RegisterRef;

                struct
                {
                        // do then condition if present
                        // simply modifies usage of say while
                        struct _PARSAST *Expr;
                } Do;

                struct
                {
                        struct _PARSAST *ToMatch;
                        struct _PARSAST *Cases;
                } Switch;

                struct
                {
                        struct _PARSAST *Params;
                        struct _PARSAST *ReturnType;
                        struct _PARSAST *Body;
                        struct _PARSAST *Name; // nullable (IMM FUNCTION)
                } Function;

                struct
                {
                        struct _PARSAST *Cond;
                        struct _PARSAST *Then;
                        struct _PARSAST *Else;
                } Ternary;

                struct
                {
                        struct _PARSAST *Type;
                        struct _PARSAST *Init;
                } Declaration;

                struct
                {
                        struct _PARSAST *Callee;
                        struct _PARSAST *Arguments;
                } Call;

                struct
                {
                        struct _PARSAST *Base;
                        struct _PARSAST *Index;
                } Access;
                
                struct
                {
                        struct _PARSAST *Left;
                        struct _PARSAST *Right;
                } Binary;
                
                struct
                {
                        struct _PARSAST *Right;
                } Prefix;
                
                struct
                {
                        struct _PARSAST *Left;
                } Postfix;
                
                struct
                {
                        struct _PARSAST *Body;
                        bool IsSafe;
                } Block;

                struct
                {
                        struct _PARSAST *Expr;
                        struct _PARSAST *Then;
                        struct _PARSAST *Else;
                } If;
                
                struct
                {
                        // while is syntax sugar for
                        // for(;cond;)
                        // do {} for is what the AST actually does and is allowed (execute then check)
                        struct _PARSAST *Init;
                        struct _PARSAST *Cond;
                        struct _PARSAST *Post;
                        struct _PARSAST *Body;
                } While_For;

                struct
                {
                        struct _PARSAST *Args;
                } Preprocessor;
        } As;
} PARSAST;

#endif
