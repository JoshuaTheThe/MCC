
#include <stdio.h>
#include <stdlib.h>
#include <lexer/lexer.h>

int main(int argc, char **argv)
{
        for (int i = 1; i < argc; ++i)
        {
                LEXFIL LexFile     = Lexer_Open(argv[i]);
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
}

