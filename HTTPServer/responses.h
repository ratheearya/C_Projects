//FOR ALL RESPONSES THAT NEED TO BE SENT TO SOCKET/USER
#include <string.h>
#include <stdio.h>
#pragma once

struct Codes {
    char statusCode[4];
    char statusPhrase[22];
    char messageBody[23];
    int messageBodySize;
};

struct Table1 {
    struct Codes Table[8];
};

extern struct Table1 responseChart;

void customError(char *message);

void invalidPortNumber(void);

void initTable1(struct Table1 *Table);

void outputMessage(
    int sd, char version[], char statCode[], char statPhrase[], char header[], char body[]);

void produceMessage(int sd, struct Table1 *Table, int x, char header[]);

//void outputValidGetMessage(int sd, char version[], char statCode[], char statPhrase[], char header[], char body[]);

//void prouduceValidGetMessage(int sd, struct Table1 *Table, int x, char header[], char body[]);
