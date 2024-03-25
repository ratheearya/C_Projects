//FOR ALL RESPONSES THAT NEED TO BE SENT TO SOCKET/USER
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "responses.h"
#include "asgn2_helper_funcs.h"

// For Errors with custom messages REMEMBER FIX
void customError(char *message) {
    //fprintf(stderr,"%s", message);
    perror(message);
    exit(1);
}

// Invalid Port Error Message
void invalidPortNumber(void) {
    fprintf(stderr, "Invalid Port\n");
    exit(1);
}

void initTable1(struct Table1 *Table) {

    strcpy(Table->Table[0].statusCode, "200");
    strcpy(Table->Table[0].statusPhrase, "OK");
    strcpy(Table->Table[0].messageBody, "OK\n");
    Table->Table[0].messageBodySize = 3;

    strcpy(Table->Table[1].statusCode, "201");
    strcpy(Table->Table[1].statusPhrase, "Created");
    strcpy(Table->Table[1].messageBody, "Created\n");
    Table->Table[1].messageBodySize = 8;

    strcpy(Table->Table[2].statusCode, "400");
    strcpy(Table->Table[2].statusPhrase, "Bad Request");
    strcpy(Table->Table[2].messageBody, "Bad Request\n");
    Table->Table[2].messageBodySize = 12;

    strcpy(Table->Table[3].statusCode, "403");
    strcpy(Table->Table[3].statusPhrase, "Forbidden");
    strcpy(Table->Table[3].messageBody, "Forbidden\n");
    Table->Table[3].messageBodySize = 10;

    strcpy(Table->Table[4].statusCode, "404");
    strcpy(Table->Table[4].statusPhrase, "Not Found");
    strcpy(Table->Table[4].messageBody, "Not Found\n");
    Table->Table[4].messageBodySize = 10;

    strcpy(Table->Table[5].statusCode, "500");
    strcpy(Table->Table[5].statusPhrase, "Internal Server Error");
    strcpy(Table->Table[5].messageBody, "Internal Server Error\n");
    Table->Table[5].messageBodySize = 22;

    strcpy(Table->Table[6].statusCode, "501");
    strcpy(Table->Table[6].statusPhrase, "Not Implemented");
    strcpy(Table->Table[6].messageBody, "Not Implemented\n");
    Table->Table[6].messageBodySize = 16;

    strcpy(Table->Table[7].statusCode, "505");
    strcpy(Table->Table[7].statusPhrase, "Version Not Supported");
    strcpy(Table->Table[7].messageBody, "Version Not Supported\n");
    Table->Table[7].messageBodySize = 22;
}

//FOR ALL RESPONSES EXCEPT VALID GET
//helper function to output a message in correct format
void outputMessage(
    int sd, char version[], char statCode[], char statPhrase[], char header[], char body[]) {
    char debug[3000];
    sprintf(debug, "HTTP/%s %s %s\r\n%s\r\n%s", version, statCode, statPhrase, header, body);
    //          v sc sp     h    b
    write_n_bytes(sd, debug, strlen(debug));
}

//given the response key, version, and header -> output the message response
void produceMessage(int sd, struct Table1 *Table, int x, char header[]) {
    (void) header;
    char temp_header[21];
    sprintf(temp_header, "Content-Length: %d\r\n", Table->Table[x - 1].messageBodySize);
    outputMessage(sd, "1.1", Table->Table[x - 1].statusCode, Table->Table[x - 1].statusPhrase,
        temp_header, Table->Table[x - 1].messageBody);
}
//------------------------------
