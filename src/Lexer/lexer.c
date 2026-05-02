
#include <lexer/lexer.h>

char Lexer_Get(LEXFIL *fil)
{
        char Character   = fgetc(fil->fp);
        fil->_Column     = fil->Column;
        fil->_Line       = fil->Line;
        fil->_LineOffset = fil->LineOffset;
        if (Character == '\n') { fil->Line   += 1; fil->LineOffset = ftell(fil->fp); fil->Column = 0; }
        else                   { fil->Column += 1; }
        return Character;
}

void Lexer_Unget(LEXFIL *fil, char Character)
{
        fil->Column     = fil->_Column;
        fil->Line       = fil->_Line;
        fil->LineOffset = fil->_LineOffset;
        ungetc(Character, fil->fp);
}

// free the entire token tree
void Lexer_Destroy(TOKEN *Tokens)
{
        TOKEN *Last = NULL;
        while (Tokens)
        {
                if (Last)
                        free(Last);
                Last   = Tokens;
                Tokens = Tokens->Next;
        }
}

TOKEN Lexer_Character(LEXFIL *fil, char First)
{
        TOKEN Token      = {0};
        char Character   = 0;
        Token.Class      = First == '\'' ? LEXER_TOKEN_CHAR_LITERAL : LEXER_TOKEN_STRING_LITERAL;
        Token.File       = fil;
        Token.Column     = fil->Column;
        Token.LineOffset = fil->LineOffset;
        Token.Line       = fil->Line;
        while ((Character = Lexer_Get(fil)) != EOF && Character != First)
        {
                Token.Identifier[Token.Number++] = Character;
        }

        return Token;
}

TOKEN Lexer_Number(LEXFIL *fil, char First)
{
        TOKEN Token      = {0};
        char Character   = 0;
        Token.Class      = LEXER_TOKEN_INTEGER_LITERAL;
        Token.File       = fil;
        Token.Column     = fil->Column;
        Token.LineOffset = fil->LineOffset;
        Token.Line       = fil->Line;
        Token.Identifier[Token.Number++] = First;
        while ((Character = Lexer_Get(fil)) != EOF && isdigit(Character))
        {
                Token.Identifier[Token.Number++] = Character;
        }

        Lexer_Unget(fil, Character);
        return Token;
}

TOKEN Lexer_Identifier(LEXFIL *fil, char First)
{
        TOKEN Token      = {0};
        char Character   = 0;
        Token.Class      = LEXER_TOKEN_IDENTIFIER;
        Token.File       = fil;
        Token.Column     = fil->Column;
        Token.LineOffset = fil->LineOffset;
        Token.Line       = fil->Line;
        Token.Identifier[Token.Number++] = First;
        while ((Character = Lexer_Get(fil)) != EOF && (isalpha(Character) || Character == '_' || isalnum(Character)))
        {
                Token.Identifier[Token.Number++] = Character;
        }

        Lexer_Unget(fil, Character);
        return Token;
}

TOKEN Lexer_Operator(LEXFIL *fil, char First)
{
        TOKEN Token = {0};
        // TODO - check for triple and double symbol before single
        static const LEXCLAS SingleClass[256] =
        {
                ['+'] = LEXER_TOKEN_ADD,
                ['-'] = LEXER_TOKEN_SUB,
                ['*'] = LEXER_TOKEN_MUL_PTR,
                ['/'] = LEXER_TOKEN_DIV,
                ['%'] = LEXER_TOKEN_MOD,
                ['^'] = LEXER_TOKEN_XOR,
                ['&'] = LEXER_TOKEN_AND,
                ['|'] = LEXER_TOKEN_OR,
                ['~'] = LEXER_TOKEN_BNOT,
                ['!'] = LEXER_TOKEN_NOT,
                [','] = LEXER_TOKEN_COMMA,
                ['.'] = LEXER_TOKEN_DOT,
                [';'] = LEXER_TOKEN_SEMICOLON,
                [':'] = LEXER_TOKEN_COLON,
                ['?'] = LEXER_TOKEN_TERNARY,
                ['('] = LEXER_TOKEN_LPEXPR,
                [')'] = LEXER_TOKEN_RPEXPR,
                ['{'] = LEXER_TOKEN_LBLOCK,
                ['}'] = LEXER_TOKEN_RBLOCK,
                ['['] = LEXER_TOKEN_LARRAY,
                [']'] = LEXER_TOKEN_RARRAY,
                ['>'] = LEXER_TOKEN_GREATER,
                ['<'] = LEXER_TOKEN_LESS,
                ['='] = LEXER_TOKEN_SET,
                ['#'] = LEXER_TOKEN_PREPROC,
        };
        
        Token.Class      = SingleClass[First];
        Token.File       = fil;
        Token.Column     = fil->Column;
        Token.LineOffset = fil->LineOffset;
        Token.Line       = fil->Line;
        Token.Identifier[Token.Number++] = First;
        return Token;
}

// returns the next raw token
TOKEN Lexer_Next(LEXFIL *fil)
{
        char Character = 0;
        // Skip WhiteSpace
        while ((Character = Lexer_Get(fil)) != EOF && isspace(Character))
                ;
        if (isdigit(Character))
                return Lexer_Number(fil, Character);
        else if (isalpha(Character) || Character == '_' || isalnum(Character))
                return Lexer_Identifier(fil, Character);
        else if (Character == '\'' || Character == '"')
                return Lexer_Character(fil, Character);
        else
                return Lexer_Operator(fil, Character);
        return (TOKEN){0};
}

// create next token for a file
TOKEN *Lexer_ConstructNext(TOKEN **Tokens, LEXFIL *fil)
{
        (void)Tokens;
        (void)fil;
        return NULL;
}

// load the entire contents of a file then lex it.
TOKEN *Lexer_LexFile(LEXFIL *fil)
{
        fseek(fil->fp, 0, SEEK_END);
        size_t size = ftell(fil->fp);
        fseek(fil->fp, 0, SEEK_SET);
        return NULL;
}
