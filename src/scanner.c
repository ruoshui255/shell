#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "parse.h"
#include "scanner.h"
#include "wrapper.h"

/*
****************************
* global variable
****************************
*/

static struct {
    char* start;
    char* current;
    char* end;
    // save the char that modity to '\0' in get_token
    char pre_char;
    char* pre_s;
    int line;
}scanner;

/*
****************************
* function declaration
****************************
*/
void 
scannerInit(char src[]) {
    scanner.start = src;
    scanner.current = src;
    scanner.end = src + strlen(src);
    scanner.line = 0;
    scanner.pre_s = src;
}

static char
scannerPeek() {
    return scanner.current[0];
}

static char
scannerAdvance() {
    scanner.current++;
    return scanner.current[-1];
}

static void
scannerSkipWhitespace() {
    for(;;) {
        char c = scannerPeek();
        switch (c) {
            case ' ':
            case '\t':
            case '\r':
                scannerAdvance();
                break;
            case '\n':
                scanner.line++;
                scannerAdvance();
                break;
            default:
                return;
        }
    }
}

bool
scanner_has_next() {
    if (scanner.pre_char != 0 || scanner.current < scanner.end) {
        return true;
    }
    return false;
}

bool
scanner_peek_equal(char* s){
    if (scanner.pre_char != 0) {
        return strchr(s, scanner.pre_char);
    }
    
    scannerSkipWhitespace();

    if (scanner_has_next()) {
        char c = *scanner.current;
        return strchr(s, c);
    }
    return false;
}

static bool
scannerAtEnd() {
    return scanner.current[0] == '\0';
}

static Token
makeToken(TokenType type) {
    Token token;
    token.start = scanner.start;
    token.length = (int)(scanner.current - scanner.start);
    token.type = type;
    return token;
}

static Token
makeString() {
    while (scannerPeek() != '"' && !scannerAtEnd()) {
        if (scannerPeek() == '\n') {
            scanner.line++;
        }
        scannerAdvance();
    }
    
    // close the '"'
    scannerAdvance();
    
    Token t = makeToken(TokenTypeArg);
    // remove "
    t.start++;
    t.length = t.length - 2;
    return t;
}

static Token
errorToken(char *msg){
    Token token;
    token.start = msg;
    token.length = strlen(msg);
    token.type = TokenTypeError;
    return token;
}

static bool
Argument(char c) {
    bool alpha = ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_' || c == '.' || c == '/';

    bool digit = '0' <= c && c <= '9';

    return alpha || digit;
}

static Token
makeArgument() {
    while (Argument(scannerPeek()) && scannerPeek() != ' ') {
        scannerAdvance();
    }

    return makeToken(TokenTypeArg);
}


Token
scannerGetToken() {
    scannerSkipWhitespace();
    scanner.start = scanner.current;

    if (scannerAtEnd()) {
        return makeToken(TokenTypeEOF);
    }

    char c = scannerAdvance();
    if (Argument(c)) {
        return makeArgument();
    }

    switch (c) {
        case '"': return makeString();
        case '<': return makeToken(TokenTypeRedirectionRead);
        case '>': {
            if (scannerPeek() == '>') {
                scannerAdvance();
                return makeToken(TokenTypeRedirectionWriteAppend);
            } else{
                return makeToken(TokenTypeRedirectionWrite);
            }
        }
        case '|': return makeToken(TokenTypePipe);
        case '(': return makeToken(TokenTypeLeftParen);
        case ')': return makeToken(TokenTypeRightParen);
        case ';': return makeToken(TokenTypeSemicolon);
        case '&': {
            if (scannerPeek() == '&') {
                scannerAdvance();
                return makeToken(TokenTypeAnd);
            } else{

                return makeToken(TokenTypeBackTask);
            }
        }
    }
    log_info("Error Token: can't arrive here\n")
    return errorToken("Unexpect character");
}

