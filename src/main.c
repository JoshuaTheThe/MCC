
#include <stdio.h>
#include <stdlib.h>
#include <lexer/lexer.h>

int main(void)
{
        LEXFIL LexFile     = {0};
        TOKEN  Tok         = {0};
        LexFile.fp         = fopen("test/0.c", "rb");
        LexFile.Column     = 0;
        LexFile.Line       = 1;
        LexFile.LineOffset = 0;
        if (!LexFile.fp)
        {
                abort();
        }

        do
        {
                Tok = Lexer_Next(&LexFile);
                printf(".class=%d; .identifier=%s; .number=%d; :%ld:%ld %ld\n",
                        Tok.Class, Tok.Identifier, Tok.Number, Tok.Line, Tok.Column, Tok.LineOffset);
        } while (Tok.Class != LEXER_TOKEN_EOF);
        fclose(LexFile.fp);
}

