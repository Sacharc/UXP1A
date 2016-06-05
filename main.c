#include <argp.h>
#include <stdlib.h>
#include <string.h>

#include "include/linda.h"

static char noLogging = 0;

static int parse_opt(int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'n': {
            noLogging = 1;
            break;
        }
    }
    return 0;
}

int input();
int output();
int read();
char* trimWhitespace(char *str);

int main(int argc, char **argv) {
    struct argp_option options[] =
            {
                    { "no-logging", 'n', 0, 0, "Disable logging to syslog"},
                    { 0 }
            };
    struct argp argp = { options, parse_opt, 0, 0 };
    argp_parse (&argp, argc, argv, 0, 0, 0);


//    linda_init();

    char *rawLine = NULL;
    size_t length = 0;

    while(1) {
        printf(">");

        size_t bytesRead = getline(&rawLine, &length, stdin);
        if (bytesRead == -1) {
            break;
        }

        char *line = trimWhitespace(rawLine);

        if (strlen(line) == 0){
            continue;
        }

        if (strcmp(line, "EXIT") == 0) {
            break;
        }

        char *instruction = strtok(line, "  \t");
        if (strcmp(instruction, "OUTPUT") == 0) {
            output();
        } else if (strcmp(instruction, "INPUT") == 0) {
            input();
        } else if (strcmp(instruction, "READ") == 0) {
            read();
        } else {
            printf("Instruction not recognized\n");
        }
    }

    free(rawLine);
//    linda_end();
}

int input() {
    
}

int output() {

}

int read() {

}

//http://stackoverflow.com/a/122721/5459240
char* trimWhitespace(char *str) {
    char *end;

    // Trim leading space
    while(isspace(*str)) {
        str++;
    }

    if(*str == 0)
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while(end > str && isspace(*end)) {
        end--;
    }

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}
