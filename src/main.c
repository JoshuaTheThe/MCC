
#include <stdio.h>
#include <stdlib.h>
#include <lexer/lexer.h>

int main(void)
{
        LEXFIL LexFile     = Lexer_Open("test/0.c");
        TOKEN *Tokens      = NULL;
        Tokens = Lexer_LexFile(&LexFile);
        for (TOKEN *Token = Tokens; Token; Token = Token->Next)
        {
                printf(" [info] .class=%d; .identifier=%s; .number=%d; :%ld:%ld %ld\n",
                        Token->Class, Token->Identifier, Token->Number, Token->Line, Token->Column, Token->LineOffset);
        }
        
        Lexer_Destroy(Tokens);
        Lexer_Close(LexFile);
}

