
#include <stdio.h>
#include <stdlib.h>
#include <lexer/lexer.h>
#include <preproc/preproc.h>
#include <parser/parser.h>
#include <project/project.h>

int main(int argc, char **argv)
{
        for (int i = 1; i < argc; ++i)
        {
                // need to finsh compiler first
                if (!strncmp(argv[i], "init", 16))
                {
                        //InitProjectDir();
                }
                else if (!strncmp(argv[i], "build", 16))
                {
                        //BuildProject();
                }
                else if (!strncmp(argv[i], "--tokenise", 16) && i < argc - 1)
                {
                        LEXFIL LexFile     = Lexer_Open(argv[i+1]);
                        TOKEN *Tokens      = NULL;
                        Tokens = Lexer_LexFile(&LexFile);
                        for (TOKEN *Token = Tokens; Token; Token = Token->Next)
                        {
                                printf(" [info] .class='%d'; .identifier='%s'; .number='%d'; :%ld:%ld %ld\n",
                                        Token->Class, Token->Identifier, Token->Number, Token->Line, Token->Column, Token->LineOffset);
                        }
        
                        Lexer_Destroy(Tokens);
                        Lexer_Close(LexFile);
                        i += 1;
                }
                else if (!strncmp(argv[i], "--preproc", 16) && i < argc - 1)
                {
                        LEXFIL LexFile     = Lexer_Open(argv[i+1]);
                        TOKEN *Tokens      = NULL;
                        Tokens = Lexer_LexFile(&LexFile);
                        //PreProcess_Tokens(&Tokens);
                        for (TOKEN *Token = Tokens; Token; Token = Token->Next)
                        {
                                printf(" [info] .class='%d'; .identifier='%s'; .number='%d'; :%ld:%ld %ld\n",
                                        Token->Class, Token->Identifier, Token->Number, Token->Line, Token->Column, Token->LineOffset);
                        }
        
                        Lexer_Destroy(Tokens);
                        Lexer_Close(LexFile);
                        i += 1;
                }
                else if (!strncmp(argv[i], "--parse", 16) && i < argc - 1)
                {
                        LEXFIL LexFile     = Lexer_Open(argv[i+1]);
                        TOKEN *Tokens      = NULL;
                        Tokens = Lexer_LexFile(&LexFile);
                        PARSAST *AST       = Parser_ConstructAST(Tokens, &LexFile);
                        // preproc would go here
                        for (PARSAST *A = AST; A; A = A->Prev) // lol iterate backwards
                        {
                                printf(" [info] AST Object %p\n", A);
                        }
        
                        Parser_DestroyAST(AST);
                        Lexer_Destroy(Tokens);
                        Lexer_Close(LexFile);
                }
        }
}
