
#include <stdio.h>
#include <stdlib.h>
#include <lexer/lexer.h>

int main(void)
{
        LEXFIL LexFile     = {0};
        TOKEN *Tokens      = NULL;
        LexFile.fp         = fopen("test/0.c", "rb");
        LexFile.Column     = 0;
        LexFile.Line       = 1;
        LexFile.LineOffset = 0;
        if (!LexFile.fp)
        {
                abort();
        }

        Tokens = Lexer_LexFile(&LexFile);
        for (TOKEN *Token = Tokens; Token; Token = Token->Next)
        {
                printf(" [info] .class=%d; .identifier=%s; .number=%d; :%ld:%ld %ld\n",
                        Token->Class, Token->Identifier, Token->Number, Token->Line, Token->Column, Token->LineOffset);
        }
        Lexer_Destroy(Tokens);
        fclose(LexFile.fp);
}

