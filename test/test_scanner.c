#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include "parse.h"
#include "scanner.h"

#define length(x) (sizeof(x)/sizeof(x[0]))

void
equalToken(Token t1, Token t2) {
    bool equal = (t1.type == t2.type) && (t1.length == t2.length) && (t1.start == t2.start);
    
    if (!equal) {
        printf("type:   src <%d> exp <%d>\n", t1.type, t2.type);
        printf("length: src <%d> exp <%d>\n", t1.length, t2.length);
        printf("start: src <%p> exp <%p>\n", t1.start, t2.start);
        exit(-1);
    }
}

void 
test1() {
    char buf[] = "ls|grep 123" ;

    scannerInit(buf);

    equalToken(scannerGetToken(), (Token){TokenTypeArg, buf, 2});
    equalToken(scannerGetToken(), (Token){TokenTypePipe, &(buf[2]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[3]), 4});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[8]), 3});
    equalToken(scannerGetToken(), (Token){TokenTypeEOF, buf+strlen(buf), 0});

    printf("%s success\n", __FUNCTION__);

    return;
}

void 
test2() {
    //                      1111111111
    //            01234567890123456789
    char buf[] = "ls | grep &&; 123 &;" ;

    scannerInit(buf);

    equalToken(scannerGetToken(), (Token){TokenTypeArg, buf, 2});
    equalToken(scannerGetToken(), (Token){TokenTypePipe, &(buf[3]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[5]), 4});
    equalToken(scannerGetToken(), (Token){TokenTypeAnd, &(buf[10]), 2});
    equalToken(scannerGetToken(), (Token){TokenTypeSemicolon, &(buf[12]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[14]), 3});
    equalToken(scannerGetToken(), (Token){TokenTypeBackTask, &(buf[18]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeSemicolon, &(buf[19]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeEOF, buf+strlen(buf), 0});

    printf("%s success\n", __FUNCTION__);
    
    return;
}

void 
test3() {
    //                      1111111111
    //            01234567890123456789
    char buf[] = "(| grep)&&; 123 &;" ;

    scannerInit(buf);

    equalToken(scannerGetToken(), (Token){TokenTypeLeftParen, buf, 1});
    equalToken(scannerGetToken(), (Token){TokenTypePipe, &(buf[1]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[3]), 4});
    equalToken(scannerGetToken(), (Token){TokenTypeRightParen, &(buf[7]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeAnd, &(buf[8]), 2});
    equalToken(scannerGetToken(), (Token){TokenTypeSemicolon, &(buf[10]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeArg, &(buf[12]), 3});
    equalToken(scannerGetToken(), (Token){TokenTypeBackTask, &(buf[16]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeSemicolon, &(buf[17]), 1});
    equalToken(scannerGetToken(), (Token){TokenTypeEOF, buf+strlen(buf), 0});

    printf("%s success\n", __FUNCTION__);
    
    return;
}

int 
main(int argc, char const *argv[]) {
    printf("%s\n", __FILE__);
    test1();    
    test2();    
    test3();    
    return 0;
}