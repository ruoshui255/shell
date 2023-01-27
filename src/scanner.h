#ifndef __SCANNER 
#define __SCANNER

#include <stdbool.h>


typedef enum {
    TokenTypePipe,
    TokenTypeRedirectionWrite, 
    TokenTypeRedirectionWriteAppend,
    TokenTypeRedirectionRead,
    TokenTypeLeftParen, 
    TokenTypeRightParen,
    TokenTypeSemicolon,
    TokenTypeBackTask,
    TokenTypeAnd,
    
    // args
    TokenTypeArg,
    TokenTypeEOF,

    TokenTypeError,
}TokenType;

typedef struct {
    TokenType type;
    char* start;
    int length;
}Token;


/* ======== scannner ===== */
// bool scanner_has_next();
// char scanner_next();
void scannerInit(char command[]);
// bool scanner_peek_equal(char* s);
Token scannerGetToken();
void scannerConsume(TokenType token, char* msg_err);

#endif