
#include <Parser/parser.h>
#include <stdarg.h>
#include <error.h>

static PARSAST *Parser_ConstructExpressions(TOKEN **const Tokens, LEXFIL *File);

static void ASTAppend(PARSAST **const A, PARSAST *const B)
{
        if (B)
                B->Prev = (*A);
        if (*A)
                (*A)->Next = B;
        *A = B;
}

static PARSAST *ASTCreateNode(ASTCLAS Class, LEXFIL *File)
{
        PARSAST *AST = calloc(1, sizeof(*AST));
        if (!AST)
        {
                Error(File, (TOKEN){0}, "Could not create AST node of class %ld", Class);
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
                Node = ASTCreateNode(PARSER_LIT_FLOAT, File);
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

static PARSAST *Parser_ConstructUnary(TOKEN **const Tokens, LEXFIL *File)
{
        TOKEN *Tok = *Tokens;
        switch (Tok->Class)
        {
        case LEXER_TOKEN_ADD:     // +
        case LEXER_TOKEN_SUB:     // -
        case LEXER_TOKEN_BAND:    // &
        case LEXER_TOKEN_MUL_PTR: // *
        case LEXER_TOKEN_NOT:     // !
        case LEXER_TOKEN_BNOT:    // ~
        case LEXER_TOKEN_INC:     // ++
        case LEXER_TOKEN_DEC:     // --
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Operand = Parser_ConstructUnary(Tokens, File);

                PARSAST *Unary = ASTCreateNode(PARSER_UNARY_EXPRESSION, File);
                Unary->Token = Op;
                Unary->Children = Operand;
                return Unary;
        }
        default:
                return Parser_ConstructPrimary(Tokens, File);
        }
}

static int IsOperatorAtLevel(TOKEN *Token, const LEXCLAS *Operators, int OpCount)
{
        for (int i = 0; i < OpCount; i++)
        {
                if (Token->Class == Operators[i])
                        return 1;
        }
        return 0;
}

static PARSAST *Parser_ConstructMultiplicative(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructUnary(Tokens, File);

        static const LEXCLAS Operators[] = {
            LEXER_TOKEN_MUL_PTR,
            LEXER_TOKEN_DIV,
            LEXER_TOKEN_MOD};

        while ((*Tokens) && IsOperatorAtLevel(*Tokens, Operators, sizeof(Operators) / sizeof(Operators[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructUnary(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructAdditive(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructMultiplicative(Tokens, File);

        static const LEXCLAS Operators[] = {
            LEXER_TOKEN_ADD, // +
            LEXER_TOKEN_SUB  // -
        };

        while ((*Tokens) && IsOperatorAtLevel(*Tokens, Operators, sizeof(Operators) / sizeof(Operators[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructMultiplicative(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructShift(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructAdditive(Tokens, File);

        static const LEXCLAS Operators[] = {
            LEXER_TOKEN_SHIFTLEFT,
            LEXER_TOKEN_SHIFTRIGHT,
            LEXER_TOKEN_ROLLLEFT,
            LEXER_TOKEN_ROLLRIGHT,
        };

        while ((*Tokens) && IsOperatorAtLevel(*Tokens, Operators, sizeof(Operators) / sizeof(Operators[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructAdditive(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructRelational(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructShift(Tokens, File);

        static const LEXCLAS Operators[] = {
            LEXER_TOKEN_LESS,
            LEXER_TOKEN_GREATER,
            LEXER_TOKEN_LESSEQ,
            LEXER_TOKEN_GREATEREQ};

        while ((*Tokens) && IsOperatorAtLevel(*Tokens, Operators, sizeof(Operators) / sizeof(Operators[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructShift(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructEquality(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructRelational(Tokens, File);

        static const LEXCLAS Operators[] = {
            LEXER_TOKEN_EQUAL,
            LEXER_TOKEN_NOTEQ};

        while ((*Tokens) && IsOperatorAtLevel(*Tokens, Operators, sizeof(Operators) / sizeof(Operators[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructRelational(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructBitwiseAnd(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructEquality(Tokens, File);

        while ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_BAND)
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructEquality(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructBitwiseXor(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructBitwiseAnd(Tokens, File);

        while ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_BXOR)
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructBitwiseAnd(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructBitwiseOr(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructBitwiseXor(Tokens, File);

        while ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_BOR)
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructBitwiseXor(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructLogicalAnd(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructBitwiseOr(Tokens, File);

        while ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_AND)
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructBitwiseOr(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructLogicalOr(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructLogicalAnd(Tokens, File);

        while ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_OR)
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructLogicalAnd(Tokens, File);

                PARSAST *Expr = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                Expr->Token = Op;

                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Expr->Children = Children;

                Left = Expr;
        }

        return Left;
}

static PARSAST *Parser_ConstructTernary(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Expr = Parser_ConstructLogicalOr(Tokens, File);

        if ((*Tokens) && (*Tokens)->Class == LEXER_TOKEN_TERNARY)
        {
                TOKEN Token = *Consume(Tokens);
                PARSAST *TrueBranch = Parser_ConstructExpressions(Tokens, File);
                Expect(Tokens, LEXER_TOKEN_COLON, File);
                PARSAST *FalseBranch = Parser_ConstructExpressions(Tokens, File);

                PARSAST *Ternary = ASTCreateNode(PARSER_TERNARY_EXPRESSION, File);
                PARSAST *Children = NULL;
                ASTAppend(&Children, Expr);
                ASTAppend(&Children, TrueBranch);
                ASTAppend(&Children, FalseBranch);
                Ternary->Children = Children;
                Ternary->Token = Token;

                return Ternary;
        }

        return Expr;
}

static PARSAST *Parser_ConstructAssignment(TOKEN **const Tokens, LEXFIL *File)
{
        PARSAST *Left = Parser_ConstructTernary(Tokens, File);
        static const LEXCLAS AssignOps[] = {
            LEXER_TOKEN_SET,
            LEXER_TOKEN_ADDSET,
            LEXER_TOKEN_SUBSET,
            LEXER_TOKEN_MULSET,
            LEXER_TOKEN_DIVSET,
            LEXER_TOKEN_MODSET,
            LEXER_TOKEN_SHIFTLEFTSET,
            LEXER_TOKEN_SHIFTRIGHTSET,
            LEXER_TOKEN_DECLARE_ASSIGN,
        };

        if ((*Tokens) && IsOperatorAtLevel(*Tokens, AssignOps, sizeof(AssignOps) / sizeof(AssignOps[0])))
        {
                TOKEN Op = *Consume(Tokens);
                PARSAST *Right = Parser_ConstructAssignment(Tokens, File); // Right-associative

                PARSAST *Assignment = ASTCreateNode(PARSER_BINARY_EXPRESSION, File);
                PARSAST *Children = NULL;
                ASTAppend(&Children, Left);
                ASTAppend(&Children, Right);
                Assignment->Children = Children;
                Assignment->Token = Op;

                return Assignment;
        }

        return Left;
}

static PARSAST *Parser_ConstructExpressions(TOKEN **const Tokens, LEXFIL *File)
{
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
        if (!Tokens || !File)
                Error(File, *Tokens, "Provided corrupted state %p:%p", Tokens, File);
        PARSAST *Nodes = NULL, *Root = NULL;
        while (Tokens->Next)
        {
                ASTAppend(&Nodes, Parser_ConstructStatement(&Tokens, File));
                if (!Root) Root = Nodes;
        }

        return Nodes;
}

void Parser_DestroyAST(PARSAST *AST)
{
        if (!AST)
                return;
        if (AST->Children)
        {
                PARSAST *End = AST->Children;
                for (;End->Next;End=End->Next);
                Parser_DestroyAST(End);
        }
        
        Parser_DestroyAST(AST->Prev);
        free(AST);
}
