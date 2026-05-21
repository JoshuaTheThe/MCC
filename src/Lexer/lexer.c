
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

char Lexer_Peek(LEXFIL *fil, long off)
{
        size_t _off = ftell(fil->fp);
        fseek(fil->fp, off, SEEK_CUR);
        char Character  = fgetc(fil->fp);
        fseek(fil->fp, _off, SEEK_SET);
        return Character;
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

        if (Last)
                free(Last);
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
        while ((Character = Lexer_Get(fil)) != EOF && (isdigit(Character) || (Token.Class == LEXER_TOKEN_INTEGER_LITERAL && Character == '.')))
        {
                Token.Identifier[Token.Number++] = Character;
                if (Character == '.') Token.Class = LEXER_TOKEN_FLOAT_LITERAL;
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
        static const char MultiClassText[][4] =
        {
                "==",
                ":=",
                "&&",
                "||",
                "^^",
                "+=",
                "-=",
                "*=",
                "/=",
                "&=",
                "|=",
                "^=",
                "%=",
                "!=",
                "<=",
                ">=",
                "<<",
                ">>",
                "->",
                "++",
                "--",
                "..",
                "<<=",
                ">>=",
                "<<<",
                ">>>",
                "...",
        };

        static const LEXCLAS MultiClass[] = {
                LEXER_TOKEN_EQUAL,              // ==
                LEXER_TOKEN_DECLARE_ASSIGN,     // :=
                LEXER_TOKEN_AND,                // &&
                LEXER_TOKEN_OR,                 // ||
                LEXER_TOKEN_XOR,                // ^^ (custom)
                LEXER_TOKEN_ADDSET,             // +=
                LEXER_TOKEN_SUBSET,             // -=
                LEXER_TOKEN_MULSET,             // *=
                LEXER_TOKEN_DIVSET,             // /=
                LEXER_TOKEN_ANDSET,             // &=
                LEXER_TOKEN_ORSET,              // |=
                LEXER_TOKEN_XORSET,             // ^=
                LEXER_TOKEN_MODSET,             // %=
                LEXER_TOKEN_NOTEQ,              // !=
                LEXER_TOKEN_LESSEQ,             // <=
                LEXER_TOKEN_GREATEREQ,          // >=
                LEXER_TOKEN_SHIFTLEFT,          // <<
                LEXER_TOKEN_SHIFTRIGHT,         // >>
                LEXER_TOKEN_ARROW,              // ->
                LEXER_TOKEN_INC,                // ++
                LEXER_TOKEN_DEC,                // --
                LEXER_TOKEN_RANGE,              // ..
                LEXER_TOKEN_COMMENT,            // //
                LEXER_TOKEN_LMULTICOMMENT,      // /*
                LEXER_TOKEN_RMULTICOMMENT,      // */
                LEXER_TOKEN_SHIFTLEFTSET,       // <<=
                LEXER_TOKEN_SHIFTRIGHTSET,      // >>=
                LEXER_TOKEN_ROLLLEFT,           // <<<
                LEXER_TOKEN_ROLLRIGHT,          // >>>
                LEXER_TOKEN_VARIADIC,           // ...
        };

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

        char Peek[4] = {First,0,0,0};
        size_t Len = 1;
        Peek[1] = Lexer_Peek(fil, 0);
        Peek[2] = Lexer_Peek(fil, 1);
        for (size_t i = 0; i < sizeof(MultiClassText)/4; i++)
        {
                if (strlen(MultiClassText[i]) == 3 && 
                        memcmp(Peek, MultiClassText[i], 3) == 0)
                {
                        Token.Class = MultiClass[i];
                        Len = 3;
                        goto found;
                }
        }
    
        for (size_t i = 0; i < sizeof(MultiClassText)/4; i++)
        {
                if (strlen(MultiClassText[i]) == 2 && 
                        memcmp(Peek, MultiClassText[i], 2) == 0)
                {
                        Token.Class = MultiClass[i];
                        Len = 2;
                        goto found;
                }
        }

        Token.Class = SingleClass[(unsigned char)First];
        Len = 1;
found:  Token.File = fil;
        Token.Column = fil->Column;
        Token.LineOffset = fil->LineOffset;
        Token.Line = fil->Line;
        for (size_t i = 1; i < Len; i++)
        {
                Lexer_Get(fil);
                Token.Identifier[Token.Number++] = Peek[i];
        }

        return Token;
}

// returns the next raw token
TOKEN Lexer_Next(LEXFIL *fil)
{
        char Character = 0, Saved = 0;
        // Skip WhiteSpace
        while ((Character = Lexer_Get(fil)) != EOF && isspace(Character))
                ;
        if (Character == '/')
        {
                Saved = Lexer_Get(fil);

                if (Saved == '/')
                {
                        while ((Character = Lexer_Get(fil)) != EOF && Character != '\n')
                                ;
                        return Lexer_Next(fil);
                }
                else if (Saved == '*')
                {
                        char Prev = 0;
                        while ((Character = Lexer_Get(fil)) != EOF)
                        {
                                if (Prev == '*' && Character == '/')
                                        break;
                                Prev = Character;
                        }
                        if (Character == EOF)
                        {
                                return (TOKEN){0};
                        }
                        return Lexer_Next(fil);
                }
                else
                {
                        if (Saved != EOF)
                                Lexer_Unget(fil, Saved);
                        return Lexer_Operator(fil, '/');
                }
        }

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

TOKEN *Lexer_FindTail(TOKEN *Tokens)
{
        static TOKEN *CachedTail = NULL;
        static TOKEN *CachedBase = NULL;
        if (CachedTail && CachedTail->Next == NULL && CachedBase == Tokens)
                return CachedTail;
        while (Tokens->Next)
                Tokens = Tokens->Next;
        CachedTail = Tokens;
        CachedBase = Tokens;
        return Tokens;
}

// create next token for a file
TOKEN *Lexer_ConstructNext(TOKEN **Tokens, LEXFIL *fil)
{
        TOKEN *NewToken = calloc(1, sizeof(*NewToken));
        TOKEN  Contents = Lexer_Next(fil);
        TOKEN *Tail     = NULL;
        if (!NewToken)
                abort();
        *NewToken = Contents;
        if (*Tokens == NULL)
        {
                *Tokens = NewToken;
                return NewToken;
        }
        Tail = Lexer_FindTail(*Tokens);
        Tail->Next     = NewToken;
        NewToken->Prev = Tail;
        return NewToken;
}

// load the entire contents of a file then lex it.
TOKEN *Lexer_LexFile(LEXFIL *fil)
{
        TOKEN *Tokens = NULL;
        TOKEN *Last   = NULL;
        while ((Last = Lexer_ConstructNext(&Tokens, fil))->Class != LEXER_TOKEN_EOF)
                ;
        Lexer_IndexLines(fil);
        return Tokens;
}

// this routine is slow and bad and stinky but oh well we only do it once per file
void Lexer_IndexLines(LEXFIL *fil)
{
        if (fil->LineOffsets) abort();
        fseek(fil->fp, 0, SEEK_SET);
        fil->LineOffsets = malloc(sizeof(long) * 1024);
        fil->LineCapacity = 1024;
        fil->LineCount = 1;
        fil->LineOffsets[0] = 0;
        int c;
        long pos = 0;
        while ((c = fgetc(fil->fp)) != EOF)
        {
                pos++;
                if (c == '\n')
                {
                        if (fil->LineCount >= fil->LineCapacity)
                        {
                                fil->LineCapacity *= 2;
                                fil->LineOffsets = realloc(fil->LineOffsets,
                                                           sizeof(long) * fil->LineCapacity);
                        }
                        
                        fil->LineOffsets[fil->LineCount++] = pos;
                }
        }
}

LEXFIL Lexer_Open(const char *Path)
{
        LEXFIL fil     = {0};
        fil.fp         = fopen(Path, "r");
        fil.Column     = 0;
        fil.Line       = 1;
        fil.LineOffset = 0;
        strncpy(fil.Identifier, Path, IDENTIFIER_SIZE - 1); // ewww
        if (!fil.fp)
        {
                abort();
        }

        return fil;
}

void Lexer_Close(LEXFIL fil)
{
        free(fil.LineOffsets);
        fclose(fil.fp);
}
