#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdbool.h>

size_t BUFFER_SIZE = 4024;
//size_t PATH_MAX = 9999;

void error2(void) {
    fprintf(stderr, "Operation Failed\n");
    exit(1);
}

void write_all(int fd, char *buf, size_t num_chars) {
    ssize_t result;

    size_t num_written = 0;

    while (num_written < num_chars) {
        result = write(fd, buf + num_written, 1);

        if (result < 0) {
            close(fd);
            error2();
        }
        if (result == 0)
            break;

        num_written += result;
    }
}

bool isValidFile(char file[]) {
    if (file == NULL) {
        return false;
    }
    int fsize = strlen(file);
    if (fsize == 0 || fsize > PATH_MAX) {
        return false;
    }
    for (int i = 0; i < fsize; i++) {
        if (file[i] == '\0') {
            return false;
        }
    }
    return true;
}

bool exists(char file[]) {
    if (access(file, F_OK) == -1) {
        return false;
    }
    return true;
}

//Return error on input
void error(void) {
    fprintf(stderr, "Invalid Command\n");
    exit(1);
}

int checkNewLine(char *input) {
    while (*input != '\0') {
        if (*input == '\n') {
            return 0;
        }
        input++;
    }
    return -1;
}

void get(char input[]) {
    char *location;
    char *token = strtok(input, "\n");
    char buffer[BUFFER_SIZE];

    if (checkNewLine(input + 5) == -1) {
        error();
    }

    token = strtok(NULL, "\n");
    if (token == NULL) {
        error();
    }

    location = token;
    if (!isValidFile(location)) {
        error();
    }
    if (!exists(location)) {
        error();
    }

    token = strtok(NULL, "\n");
    if (token != NULL) {
        error();
    }

    int fd = open(location, O_RDONLY);
    if (fd == -1) {
        close(fd);
        error();
    }

    int res_r;
    do {
        res_r = read(fd, buffer, BUFFER_SIZE);
        if (res_r < 0) {
            close(fd);
            error();
        } else if (res_r > 0) {
            int res_w = 0;
            do {
                int bytes = write(STDOUT_FILENO, buffer + res_w, res_r - res_w);
                if (bytes <= 0) {
                    close(fd);
                    error2();
                }
                res_w += bytes;
            } while (res_w < res_r);
        }
    } while (res_r > 0);

    if (res_r == -1) {
        close(fd);
        error();
    }

    close(fd);
}

void set(char input[]) {
    char *location, *content_length_input;
    const char *name;
    char *token;
    int content_size; //actual size = content_length_input unless c_l_i > content length

    //get user input with strtok (get or set). If no new line give an error
    token = strtok(input, "\n");
    if (token == NULL) {
        error();
    }

    //get location of file to create or edit
    token = strtok(NULL, "\n");
    if (token == NULL) {
        error();
    }
    location = token;
    if (!isValidFile(location)) {
        error();
    }
    name = location;

    //get the number of bytes to write
    token = strtok(NULL, "\n");
    if (token == NULL) {
        error();
    }
    content_length_input = token;

    //check if non-numeric characters are in the length. If so give error.
    char *stop;
    content_size = strtol(content_length_input, &stop, 10); //content_size is the int
    if (*stop != '\0') {
        error();
    }

    token = strtok(NULL, "");
    char content[content_size];
    int real_size = 0;

    for (int i = 0; i < content_size; i++) {
        if (token == NULL) {
            content[i] = '\0';
            break;
        }
        content[i] = *token;
        token++;
        real_size++;
    }
    //content = token;

    int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd == -1) {
        close(fd);
        error();
    }
    if (real_size < content_size) {
        content_size = real_size;
    }

    write_all(fd, content, content_size);

    close(fd);
    printf("OK\n");
}

void get_input(char *input, size_t INPUT_SIZE) {
    char buf[BUFFER_SIZE];
    int input_res;
    size_t count = 0;

    do {
        input_res = read(STDIN_FILENO, buf, BUFFER_SIZE);
        for (int i = 0; i < input_res; i++) {
            if (buf[i] == '\0') {
                break;
            }
            if (buf[i] == '\n') {
                input[count] = '\n';
            }
            input[count] = buf[i];
            count++;
            if (count >= INPUT_SIZE - 1) {
                error();
                break;
            }
        }
    } while (input_res > 0);

    input[count] = '\0';
    count++;
}

int main() {
    char command[4];
    size_t INPUT_SIZE = PATH_MAX;
    char input[INPUT_SIZE];
    get_input(input, INPUT_SIZE);

    //Get the command get or set
    int i = 0;
    while (input[i] != '\0' && i < 3) {
        command[i] = input[i];
        i++;
    }
    command[i] = '\0';

    //Check if command is get or set. If they are execute get or set function otherwise return error
    if (strcmp(command, "get") == 0) {
        get(input);
    } else if (strcmp(command, "set") == 0) {
        set(input);
    } else {
        error();
    }
    return 0;
}
