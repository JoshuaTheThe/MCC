
#include <PreProc/preproc.h>

void PreProcess_Tokens(TOKEN **Tokens)
{
        TOKEN *Token = *Tokens;
        while (Token)
        {
                if (Token->Class == LEXER_TOKEN_EOL || Token->Class == LEXER_TOKEN_CNT)
                {
                        if (Token == *Tokens)
                                *Tokens = Token->Next;
                        Lexer_RemoveToken(Token);
                }
                
                Token = Token->Next;
        }
}
