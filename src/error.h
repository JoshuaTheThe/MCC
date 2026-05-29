
#ifndef ERROR_H
#define ERROR_H

#include <Lexer/lexer.h>

void Error(LEXFIL *File, TOKEN ReferenceToken, const char *const fmt, ...);

#endif
