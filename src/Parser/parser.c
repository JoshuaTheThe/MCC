
#include <Parser/Parser.h>
#include <stdarg.h>

static PARSAST *Parser_ConstructExpressions(TOKEN **const Tokens, LEXFIL *File);

void Parser_Error(LEXFIL *File, TOKEN ReferenceToken, const char *const fmt, ...)
{
        char ErrorBuffer[1024*2] = {0};
        char FormattedMessage[1024] = {0};
        va_list args;
        va_start(args, fmt);
        vsnprintf(FormattedMessage, sizeof(FormattedMessage), fmt, args);
        va_end(args);
        if (ReferenceToken.Class != LEXER_TOKEN_EOF)
                snprintf(ErrorBuffer, sizeof(ErrorBuffer), "%s:%ld:%ld: error: %s\n", 
                         File->Identifier, ReferenceToken.Line, ReferenceToken.Column, 
                         FormattedMessage);
        else
                snprintf(ErrorBuffer, sizeof(ErrorBuffer), "error: %s\n", FormattedMessage);
        printf("%s", ErrorBuffer);
        if (ReferenceToken.Class != LEXER_TOKEN_EOF && 
                ReferenceToken.Line < File->LineCount)
        {
                char Character = 0;
                fseek(File->fp, File->LineOffsets[ReferenceToken.Line-1], SEEK_SET);
                while (Character != '\n' && Character != EOF)
                {
                        if (Character != 0)
                                printf("%c", Character);
                        Character = fgetc(File->fp);
                }
        
                printf("\n%*s^\n", ReferenceToken.Column-1, "");
        }
    
        exit(1);
}

static void ASTAppend(PARSAST **const A, PARSAST *const B)
{
        if (B)
                B->Prev = (*A);
        if (*A)
                (*A)->Next = B;
        *A = B;
}

static TOKEN *Consume(TOKEN **const Token)
{
        if (!Token) return NULL;
        TOKEN *const Tok = *Token;
        *Token = (*Token)->Next;
        return Tok;
}

static TOKEN *Expect(TOKEN **const Token, LEXCLAS Class, LEXFIL *File)
{
        if (Consume(Token)->Class != Class)
        {
                Parser_Error(File, **Token, "Expected token of class %ld when provided %ld", Class, (*Token)->Class);
        }

        return *Token;
}

static TOKEN *UnConsume(TOKEN **const Token)
{
        if (!Token) return NULL;
        TOKEN *const Tok = *Token;
        *Token = (*Token)->Prev;
        return Tok;
}

static PARSAST *ASTCreateNode(ASTCLAS Class, LEXFIL *File)
{
        PARSAST *AST = calloc(1, sizeof(*AST));
        if (!AST)
        {
                Parser_Error(File, (TOKEN){0}, "Could not create AST node of class %ld", Class);
        }

        AST->Class = Class;
        return AST;
}

static PARSAST *Parser_ConstructPrimary(TOKEN **const Tokens, LEXFIL *File)
{
        TOKEN *Tok = *Tokens;
        PARSAST *Node = NULL;
        switch (Tok->Class)
        {
        case LEXER_TOKEN_LPEXPR:
                Node = Parser_ConstructExpressions(Tokens, File);
                Expect(Tokens, LEXER_TOKEN_RPEXPR, File);
                break;
        case LEXER_TOKEN_IDENTIFIER:
                Node = ASTCreateNode(PARSER_SYMBOL, File);
                Node->Token = *Consume(Tokens);
                break;
        case LEXER_TOKEN_INTEGER_LITERAL:
                Node = ASTCreateNode(PARSER_LIT_INTEGER, File);
                Node->Token = *Consume(Tokens);
                break;
        case LEXER_TOKEN_CHAR_LITERAL:
                Node = ASTCreateNode(PARSER_LIT_CHAR, File);
                Node->Token = *Consume(Tokens);
                break;
        case LEXER_TOKEN_FLOAT_LITERAL:
                Node = ASTCreateNode(PARSER_LIT_STRING, File);
                Node->Token = *Consume(Tokens);
                break;
        case LEXER_TOKEN_STRING_LITERAL:
                Node = ASTCreateNode(PARSER_LIT_STRING, File);
                Node->Token = *Consume(Tokens);
                break;
        default:
                break;
        }

        return Node;
}

// &&
static PARSAST *Parser_ConstructAnd(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Nodes = NULL;
        PARSAST *Left  = Parser_ConstructPrimary(Tokens, File);
        TOKEN Token = **Tokens;
        while ((*Tokens)->Class == LEXER_TOKEN_AND)
        {
                Token = *Consume(Tokens);
                ASTAppend(&Nodes, Parser_ConstructPrimary(Tokens, File));
        }

        if (!Nodes)
                return Left;
        PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
        Expr->Token = Token;
        ASTAppend(&Nodes, Left);
        Expr->Children = Nodes;
        return Expr;
}

// ||
static PARSAST *Parser_ConstructOr(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Nodes = NULL;
        PARSAST *Left  = Parser_ConstructAnd(Tokens, File);
        TOKEN Token = **Tokens;
        while ((*Tokens)->Class == LEXER_TOKEN_AND)
        {
                Token = *Consume(Tokens);
                ASTAppend(&Nodes, Parser_ConstructAnd(Tokens, File));
        }

        if (!Nodes)
                return Left;
        PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
        Expr->Token = Token;
        ASTAppend(&Nodes, Left);
        Expr->Children = Nodes;
        return Expr;
}

// a?b:c;
static PARSAST *Parser_ConstructTernary(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Nodes = NULL;
        PARSAST *Expr  = Parser_ConstructOr(Tokens, File);
        TOKEN Token = **Tokens;
        if ((*Tokens)->Class == LEXER_TOKEN_TERNARY)
        {
                Token = *Consume(Tokens);
                ASTAppend(&Nodes, Parser_ConstructOr(Tokens, File));
                Expect(Tokens, LEXER_TOKEN_COLON, File);
                ASTAppend(&Nodes, Parser_ConstructOr(Tokens, File));
        }

        if (!Nodes)
                return Expr;
        PARSAST *Ternary = ASTCreateNode(PARSER_TERNARY_EXPRESSION, File);
        ASTAppend(&Nodes, Expr);
        Ternary->Token = Token;
        Ternary->Children = Nodes;
        return Ternary;
}

static PARSAST *Parser_ConstructAssignment(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Nodes = NULL;
        PARSAST *Left  = Parser_ConstructTernary(Tokens, File);
        TOKEN Token = **Tokens;
        while ((*Tokens)->Class == LEXER_TOKEN_SET) // others would follow, simplified for now
        {
                Token = *Consume(Tokens);
                ASTAppend(&Nodes, Parser_ConstructTernary(Tokens, File));
        }

        if (!Nodes)
                return Left;
        PARSAST *Assignment = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
        ASTAppend(&Nodes, Left);
        Assignment->Children = Nodes;
        Assignment->Token = Token;
        return Assignment;
}

static PARSAST *Parser_ConstructExpressions(TOKEN **const Tokens, LEXFIL *File)
{
        // not a special expr, just a list of them
        PARSAST *Nodes = NULL;
        ASTAppend(&Nodes, Parser_ConstructAssignment(Tokens, File));
        while ((*Tokens)->Class == LEXER_TOKEN_COMMA)
        {
                Consume(Tokens);
                ASTAppend(&Nodes, Parser_ConstructAssignment(Tokens, File));
        }

        return Nodes;
}

static PARSAST *Parser_ConstructDeclaration(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Nodes = Parser_ConstructExpressions(Tokens, File);
        Expect(Tokens, LEXER_TOKEN_SEMICOLON, File);
        return Nodes;
}

static PARSAST *Parser_ConstructStatement(TOKEN **const Tokens, LEXFIL *File)
{
        TOKEN *Token = *(Tokens);
        switch (Token->Class)
        {
        case LEXER_TOKEN_STATIC:
        case LEXER_TOKEN_AUTO:
        case LEXER_TOKEN_VOID:
        case LEXER_TOKEN_UNSIGNED:
        case LEXER_TOKEN_SIGNED:
        case LEXER_TOKEN_CHAR:
        case LEXER_TOKEN_SHORT:
        case LEXER_TOKEN_INT:
        case LEXER_TOKEN_LONG:
        case LEXER_TOKEN_STRUCT:
        case LEXER_TOKEN_ENUM:
        case LEXER_TOKEN_UNION:
        case LEXER_TOKEN_CONST:
//        case LEXER_TOKEN_TYPEDEF: // i think ill make typedef preproc to just struct,enum,union +T
                // declaration of either
                /*
                 * <T> x = (T)(..)
                 * <T> x()    {..}
                 * struct <I> {..}
                 *
                 * */
                Consume(Tokens);
                break;
        default: // expression
                {
                        PARSAST *Node = Parser_ConstructDeclaration(Tokens, File);
                        return Node;
                }
                break;
        }
        return NULL;
}

PARSAST *Parser_ConstructAST(TOKEN *Tokens, LEXFIL *File)
{
        if (!Tokens || !File) Parser_Error(File, *Tokens, "Provided corrupted state %p:%p", Tokens, File);
        PARSAST *Nodes = NULL;
        while (Tokens->Next)
        {
                ASTAppend(&Nodes, Parser_ConstructStatement(&Tokens, File));
        }

        return Nodes;
}

void Parser_DestroyAST(PARSAST *AST)
{
        if (!AST) return;
        Parser_DestroyAST(AST->Children);
        Parser_DestroyAST(AST->Prev);
        free(AST);
}

