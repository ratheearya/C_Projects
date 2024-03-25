//FOR ALL STRING PARSING FUNCTIONS
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "responses.h"

//Return -1 if no match
// LEN DESTINATION MUST BE > 1 Use regex to copy the first match of the string you find. Return Int for the end of the string(.rm_eo)
ssize_t copyStr(char buf[], const char *regex, size_t n_matches, char *destination) {
    char str[strlen(buf)];
    strcpy(str, buf);
    //intialize values
    regex_t rc;
    regmatch_t matches[n_matches];
    if (sizeof(destination) < 1) {
        customError("Destination must be > 1");
    }

    //Compile
    int result = regcomp(&rc, regex, REG_EXTENDED);
    if (result != 0) {
        regfree(&rc);
        customError("regcomp didn't work");
    }

    //execute regex
    int request_regexec = regexec(&rc, str, n_matches, matches, 0);
    if (request_regexec != 0 && request_regexec != 1) {
        regfree(&rc);
        customError("Not Valid according to regexec");
    }

    //Return -1 if no copy found
    if (matches[0].rm_so == -1 || matches[0].rm_eo == -1) {
        destination[0] = '\0';
        return -1;
    }
    //copy result to destination
    size_t match_length = matches[0].rm_eo - matches[0].rm_so;
    strncpy(destination, str + matches[0].rm_so, match_length);
    destination[match_length] = '\0';

    //Free memory
    regfree(&rc);
    return matches[0].rm_eo;
}

// Use Regex to check if your string is Valid NEED SLIGHT FIX FOR REGEXEC ERROR HANDLING
int checkValid(char str[], const char *regex, size_t n_matches) {
    //intialize values
    regex_t rc;
    regmatch_t matches[n_matches];

    //Compile
    int result = regcomp(&rc, regex, REG_EXTENDED);
    if (result != 0) {
        regfree(&rc);
        customError("regcomp didn't work");
    }

    //execute regex
    int request_regexec = regexec(&rc, str, n_matches, matches, 0);
    if (request_regexec != 0) {
        regfree(&rc);
        printf("Not Valid according to regexec (Seeing this does not mean bad!)\n");
        return -1;
    }

    //Free memory
    regfree(&rc);
    return 0;
}
