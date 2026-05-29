
#include <Parser/parser.h>
#include <error.h>

void Parser_AST_Append(PARSAST **const A, PARSAST *const B)
{
        if (B)
                B->Prev = (*A);
        if (*A)
                (*A)->Next = B;
        *A = B;
}

PARSAST *Parser_AST_Create(ASTCLAS Class, LEXFIL *File)
{
        PARSAST *AST = calloc(1, sizeof(*AST));
        if (!AST)
        {
                Error(File, (TOKEN){0}, "Could not create AST node of class %ld", Class);
        }

        AST->Class = Class;
        return AST;
}

void Parser_AST_Destroy(PARSAST *AST)
{
        if (!AST)
                return;
        if (AST->Children)
        {
                PARSAST *End = AST->Children;
                for (;End->Next;End=End->Next);
                Parser_AST_Destroy(End);
        }
        
        Parser_AST_Destroy(AST->Prev);
        free(AST);
}

PARSAST *Parser_AST_Construct(TOKEN *Tokens, LEXFIL *File)
{
        if (!Tokens || !File)
                Error(File, *Tokens, "Provided corrupted state %p:%p", Tokens, File);
        PARSAST *Nodes = NULL, *Root = NULL;
        while (Tokens->Next)
        {
                if (File->Lang == LEXER_LANG_C)
                        Parser_AST_Append(&Nodes, Parser_C_ConstructStatement(&Tokens, File));
                else if (File->Lang == LEXER_LANG_P)
                        Tokens=Tokens->Next;
                if (!Root) Root = Nodes;
        }

        return Nodes;
}
