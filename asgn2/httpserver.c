#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/socket.h>
#include "asgn2_helper_funcs.h"
#include "responses.h"
#include "regex_commands.h"
#define BUFFER_SIZE 4096

// Return Port Number from argv input
int getPortNumber(int argc, char *argv[]) {

    //error if not specifically one port entry
    if (argc != 2) {
        fprintf(stderr, "You must input one parameter, no more or less");
        exit(1);
    }

    char *stop;
    int portNum = strtol(argv[1], &stop, 10); //10 represents base 10

    //return error if port contains a char
    if (*stop != '\0') {
        invalidPortNumber();
    }
    //return error if port is not between 1 and 65535
    if (portNum < 1 || portNum > 65535) {
        invalidPortNumber();
    }

    return portNum;
}

//Initialize Socket
int initializeSocket(Listener_Socket *sock, int portNum) {
    int sd = listener_init(sock, portNum);
    if (sd == -1) {
        invalidPortNumber();
    }
    return sd;
}

// Accept Connections
int acceptConnection(Listener_Socket *sock) {
    int sd = listener_accept(sock);
    if (sd == -1) {
        close(sd);
        customError("listener_accept failed");
    }
    return sd;
}

//read the rest of the input into a trash buffer
void end(int sd) {
    char test[1000];
    ssize_t bytes_read = recv(sd, test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
    if (bytes_read <= 0) {
        return;
    } else {
        read_n_bytes(sd, test, bytes_read);
    }
    end(sd);
}

int checkValidGet(int sd, char buf[]) {

    //check if get is formatted correctly and doesn't have any trailing values
    const char *get_regex = "^[a-zA-Z]{1,8} /[a-zA-Z0-9._]{1,63} "
                            "HTTP/[0-9]\\.[0-9]\r\n([a-zA-Z0-9.-]{1,128}: [ -~]{0,128}\r\n)*\r\n";
    int valid = checkValid(buf, get_regex, 1);
    if (valid == -1) {
        return -1;
    }

    (void) sd;
    //check for extra bytes after input
    // char extra[3];
    // int extra_bytes = read_until(sd, extra, 2, NULL);
    // if (extra_bytes > 0) {
    //     return -1;
    // }
    return 0;
}

void parseURI(char temp_uri[], char uri[]) {
    const char *uri_regex = "/[a-zA-Z0-9.-]{1,64}";
    copyStr(temp_uri, uri_regex, 1, uri);
}

void parseVersion(char buf[], char version[]) {
    char temp_version[9];
    const char *version_regex = "HTTP/[0-9][.][0-9]";
    copyStr(buf, version_regex, 1, temp_version);

    version[0] = temp_version[5];
    version[1] = temp_version[6];
    version[2] = temp_version[7];
    version[3] = '\0';
}

int readURI(int sd, char uri[], struct Table1 *responseChart) {
    //check if uri is accesible

    //check if it is a file
    struct stat fileStat;
    if (lstat(uri, &fileStat) != -1) {
        if (!S_ISREG(fileStat.st_mode)) {
            produceMessage(sd, responseChart, 4, ""); //403
            return -1;
        }
    }

    //check if you can read file
    if (access(uri, R_OK) != 0) {
        produceMessage(sd, responseChart, 4, ""); //403
        return -1;
    }

    //get size of file
    int fd = open(uri, O_RDONLY);
    if (fd == -1) {
        perror("Can't open file");
        produceMessage(sd, responseChart, 6, ""); // 500
        return -1;
    }

    struct stat st;
    size_t fileSize = 0;
    if (stat(uri, &st) == 0) {
        fileSize = st.st_size;
    } else {
        produceMessage(sd, responseChart, 6, ""); // 500
        return -1;
    }

    //get takes 8 seconds up to here

    char preresult[60];
    sprintf(preresult, "HTTP/1.1 200 OK\r\nContent-Length: %lu\r\n\r\n", fileSize);
    int b = write_n_bytes(sd, preresult, strlen(preresult));
    if (b < 0) {
        produceMessage(sd, responseChart, 6, ""); //500
        close(fd);
        return -1;
    }

    int worked = pass_n_bytes(fd, sd, fileSize);
    if (worked == -1) {
        produceMessage(sd, responseChart, 6, ""); //500
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

//params needed: char* request string aka buffer, int sd,  Listener_Socket* sock,
void runGet(int sd, char buf[]) {

    //Initialize Response Table
    //x for table is one indexed.
    struct Table1 responseChart;
    initTable1(&responseChart);

    //check valid format with regex AND ensure there is no message body
    int validGet = checkValidGet(sd, buf);
    if (validGet == -1) {
        produceMessage(sd, &responseChart, 3, ""); // 400
        return;
    }

    //get the URI
    char temp_uri[65];
    memset(temp_uri, 0, 65);
    char *uri;
    parseURI(buf, temp_uri);
    uri = temp_uri + 1;

    //get the Version
    char version[4];
    memset(version, 0, 4);
    parseVersion(buf, version);
    //check if valid version

    if (strcmp(version, "1.1") != 0) {
        produceMessage(sd, &responseChart, 8, ""); //505
        return;
    }

    //check if file exists
    if (access(uri, F_OK) == -1) {
        produceMessage(sd, &responseChart, 5, ""); // 404
        return;
    } else {
        readURI(sd, uri, &responseChart);
    }
}

void runPut(int sd, char buf[]) {
    //Initialize Response Table
    //x for table is one indexed.
    struct Table1 responseChart;
    initTable1(&responseChart);
    const char *get_regex = "^[a-zA-Z]{1,8} /[a-zA-Z0-9._]{1,63} "
                            "HTTP/[0-9]\\.[0-9]\r\n([a-zA-Z0-9.-]{1,128}: [ -~]{0,128}\r\n)*\r\n";

    int valid = checkValid(buf, get_regex, 1);
    if (valid == -1) {
        produceMessage(sd, &responseChart, 3, ""); //400 Bad
        return;
    }

    char temp_uri[64];
    char *uri;
    parseURI(buf, temp_uri);
    uri = temp_uri + 1;

    //get the Version
    char version[4];
    memset(version, 0, 4);
    parseVersion(buf, version);
    //check if valid version
    if (strcmp(version, "1.1") != 0) {
        produceMessage(sd, &responseChart, 8, ""); // 505
        return;
    }

    //get content length and store in int contentLength
    const char *content_length_regex = "\r\nContent-Length: [0-9]+\r\n";
    char temp_content_length[100];
    copyStr(buf, content_length_regex, 1, temp_content_length);
    char *content_length;
    temp_content_length[strlen(temp_content_length) - 2] = '\0';
    content_length = temp_content_length;
    content_length += 18;
    int contentLength = atoi(content_length);

    int exists = 0;
    if (access(uri, F_OK) == -1) {
        exists = 1;
    }

    int fd = open(uri, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        produceMessage(sd, &responseChart, 4, ""); // 403
        return;
    }

    char test[contentLength];
    ssize_t bytes_read = recv(sd, test, sizeof(test), MSG_PEEK | MSG_DONTWAIT);
    if (bytes_read <= 0) {
        produceMessage(sd, &responseChart, 5, "");
    }
    ssize_t passed = pass_n_bytes(sd, fd, contentLength);
    close(fd);
    if (passed != contentLength) {
        produceMessage(sd, &responseChart, 5, "");
        return;
    }

    if (exists == 0) {
        produceMessage(sd, &responseChart, 1, "");
    } else {
        produceMessage(sd, &responseChart, 2, "");
    }

    return;
}

//Start Running the Server
void runServer(Listener_Socket *sock, int sd) {
    const size_t MAX_REQUEST_SIZE = 2048;
    const size_t MAX_COMMAND_SIZE = 8;

    struct Table1 responseChart;
    initTable1(&responseChart);

    char buf[BUFFER_SIZE];
    ssize_t requestSize = MAX_REQUEST_SIZE;
    //parsing for request
    const char *request_regex = "^([a-zA-z]{1,8}) (/[a-zA-Z0-9._]{1,63} "
                                "HTTP/[0-9]*\\.[0-9]*\r\n(([ -~]{1,128}\r\n)*)\r\n)";
    char command[MAX_COMMAND_SIZE];
    //parsing for command
    const char *command_regex = "^[a-zA-z]{1,8}";

    //Regex parsing does not scale with these sizes

    //continously accept connections made from client to port
    while (1) {
        //accept connection
        sd = acceptConnection(sock);
        if (sd == -1) {
            fprintf(stderr, "Error accepting connection\n");
            end(sd);
            continue;
        }

        //memset buffer
        memset(buf, 0, BUFFER_SIZE);

        //read the Full Request into buffer and store bytes read to requestSize
        requestSize = read_until(sd, buf, MAX_REQUEST_SIZE, "\r\n\r\n");
        if (requestSize < 1) {
            produceMessage(sd, &responseChart, 3, "");
            close(sd);
            continue;
        }

        buf[requestSize] = 0; // null terminate string

        //check for valid request Input
        if (checkValid(buf, request_regex, 1) != 0) {
            produceMessage(sd, &responseChart, 3, ""); //400 bad request
            end(sd);
            close(sd);
            continue;
        };

        //get the command
        copyStr(buf, command_regex, 1, command);
        //command[MAX_REQUEST_LINE_SIZE] = '\0';

        //perform action necessary: GET or PUT
        if (strcmp(command, "GET") == 0) {
            runGet(sd, buf);
        } else if (strcmp(command, "PUT") == 0) {
            runPut(sd, buf);
        } else {
            produceMessage(sd, &responseChart, 7, "\r\n"); // 501
        }

        end(sd);
        close(sd);
    }
}

//Main Function
int main(int argc, char *argv[]) {

    //get Port Number from argv user input
    int portNum = getPortNumber(argc, argv);

    //Create Socket
    Listener_Socket sock;

    //Initalize socket, bind socket to port, and listen.
    int sd = initializeSocket(&sock, portNum);

    //run the server
    runServer(&sock, sd);

    //end(sd);
    close(sd);
    return 0;
}
