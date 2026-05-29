
#include <Lexer/lexer.h>
#include <error.h>

const char C_Keywords[][16] =
{
	"if",
	"else",
	"while",
	"do",
	"return",
	"unsigned",
	"signed",
	"static",
	"const",
	"auto",
	"case",
	"switch",
	"break",
	"continue",
	"register",
	"union",
	"void",
	"int",
	"short",
	"char",
	"long",
	"extern",
	"enum",
	"sizeof",
	"struct",
	"typedef",
	"goto",
	"float",
	"double",
	"default",
	"volatile",
	"optional",
	"some",
	"unwrap",
	"safe",
	"unsafe",
	"include",
	"define",
	"ifdef",
	"elseif",
	"endif",
	"pragma",
	"for",
};

const char P_Keywords[][16] =
{
	"Module",
	"Program",
	"Require",
        "Embed",
	"Procedure",
	"Begin",
	"End",
	"Integer",
	"Real",
	"Char",
	"Optional",
	"Nothing",
	"Address",
	"Byte",
	"Some",
	"Return",
	"Type",
	"Record",
	"If",
	"Else",
	"While",
	"For",
	"To",
	"In",
	"Step",
	"Version",
	"External",
	"Except",
	"Boolean",
	"True",
	"False",
	"As",
	"Constant",
	"Author",
	"License",
};

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
        {
                printf("Could not create Token\n");
                abort();
        }
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
        if (fil->LineOffsets)
        {
                printf("File already indexed\n");
                abort();
        }
        
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

        const size_t Length  = fil->Lang == LEXER_LANG_C ? sizeof(C_Keywords) / sizeof(C_Keywords[0]) : sizeof(P_Keywords) / sizeof(P_Keywords[0]);
        const size_t LengthP = fil->Lang == LEXER_LANG_C ? sizeof(C_Keywords[0]) : sizeof(P_Keywords[0]);
        const char (*Keywords)[16]= fil->Lang == LEXER_LANG_C ? C_Keywords : P_Keywords;
        for (size_t i = 0; i < Length; ++i)
        {
                if (!strncmp(Token.Identifier, Keywords[i], LengthP))
                {
                        Token.Class = LEXER_TOKEN_C_KEYWORD_START + i;
                        break;
                }
        }

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
                ['\\'] = LEXER_TOKEN_CNT,
                ['@'] = LEXER_TOKEN_P_ATTR,
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
                printf("Could not open file %s\n", Path);
                abort();
        }

        // even more eww
        static const LANGUAGE _map[256] = {
                ['p'] = LEXER_LANG_P,
                ['c'] = LEXER_LANG_C,
        };

        fil.Lang = _map[(unsigned char)fil.Identifier[strnlen(fil.Identifier, IDENTIFIER_SIZE - 1) - 1]];
        return fil;
}

void Lexer_Close(LEXFIL fil)
{
        free(fil.LineOffsets);
        fclose(fil.fp);
}

void Lexer_RemoveToken(TOKEN *Token)
{
        if (Token->Prev)
                Token->Prev->Next = Token->Next;
        if (Token->Next)
                Token->Next->Prev = Token->Prev;
}

TOKEN *Consume(TOKEN **const Token)
{
        if (!Token)
                return NULL;
        TOKEN *const Tok = *Token;
        *Token = (*Token)->Next;
        return Tok;
}

TOKEN *Expect(TOKEN **const Token, LEXCLAS Class, LEXFIL *File)
{
        if (Consume(Token)->Class != Class)
        {
                Error(File, **Token, "Expected token of class %ld when provided %ld", Class, (*Token)->Class);
        }

        return *Token;
}

TOKEN *UnConsume(TOKEN **const Token)
{
        if (!Token)
                return NULL;
        TOKEN *const Tok = *Token;
        *Token = (*Token)->Prev;
        return Tok;
}

// returns the next raw token
TOKEN Lexer_Next(LEXFIL *fil)
{
        char Character = 0, Saved = 0;
        // Skip WhiteSpace
        while ((Character = Lexer_Get(fil)) != EOF && isspace(Character))
                if (Character == '\n') return (TOKEN){.Class = LEXER_TOKEN_EOL, .File = fil, .Column = fil->Column, .LineOffset = fil->LineOffset, .Line = fil->Line};
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
        {
                TOKEN Tok = Lexer_Operator(fil, Character);
                if (Tok.Class == LEXER_TOKEN_COMMENT)
                {
                        while (Character != '\n')
                                Character = Lexer_Get(fil);
                        Lexer_Get(fil);
                        return Lexer_Next(fil);
                }
                else if (Tok.Class == LEXER_TOKEN_LMULTICOMMENT)
                {
                        while (Tok.Class != LEXER_TOKEN_RMULTICOMMENT)
                        {
                                Tok = Lexer_Next(fil);
                        }

                        return Lexer_Next(fil);
                }
                return Tok;
        }
        return (TOKEN){0};
}
