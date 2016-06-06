#include <argp.h>
#include <stdlib.h>
#include <string.h>

#include "include/linda.h"
#include "include/dynamic_va_list.h"

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
int output(char* line, size_t lineLength);
int read();

char* getNumber(char* str);
char* getString(char* str);
char* trimWhitespace(char *str);

void printErrorMessage(int status);

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

    int status;
    while(1) {
        printf(">");

        __ssize_t bytesRead = getline(&rawLine, &length, stdin);
        if (bytesRead == -1) {
            break;
        }

        char *line = trimWhitespace(rawLine);
        size_t lineLength = strlen(line);
        if (lineLength == 0){
            continue;
        }

        if (strcmp(line, "EXIT") == 0) {
            break;
        }

        char *instruction = strtok(line, "  \t");
        if(strcmp(instruction, "OUTPUT") == 0) {
            status = output(line, lineLength);
            if(status) {
                printErrorMessage(status);
            }
        } else if(strcmp(instruction, "INPUT") == 0) {
            input();
        } else if(strcmp(instruction, "READ") == 0) {
            read();
        } else {
            printf("Instruction not recognized\n");
        }
    }

    free(rawLine);
//    linda_end();
}


char* getNumber(char* string) {
    size_t length = strlen(string);

    char* numberStart = NULL;
    for(size_t i = 0; i < length; ++i) {
        if(!isspace(string[i])) {
            numberStart = string + i;
            break;
        }
    }

    if(numberStart == NULL) {
        return NULL;
    }

    length = strlen(numberStart);

    for(size_t i = 1; i < length; ++i) {
        if(isspace(numberStart[i])) {
            numberStart[i] = '\0';
            break;
        }
    }
    return numberStart;
}


char* getString(char* string) {
    size_t length = strlen(string);

    char* strStart = NULL;
    for(size_t i = 0; i < length; ++i) {
        if(!isspace(string[i])) {
            // String must start with '"'
            if(string[i] == '"') {
                strStart = string + i + 1;
            }
            break;
        }
    }

    if(strStart == NULL) {
        return NULL;
    }

    length = strlen(strStart);

    char endOfStringFound = 0;
    for(size_t i = 0; i < length; ++i) {
        if(strStart[i] == '"') {
            strStart[i] = '\0';
            endOfStringFound = 1;
            break;
        }
    }

    if(endOfStringFound) {
        return strStart;
    } else {
        return NULL;
    }
}

int input() {

}

#define ERR_NO_INFOSTR 1
#define ERR_WRONG_INFOSTR 2
#define ERR_VARIABLES_NUM 3
#define ERR_INT_PARSE 4
#define ERR_FLOAT_PARSE 5

int output(char* line, size_t lineLength) {
    char* infoString = strtok(NULL, " \t");

    if(infoString == NULL) {
        return ERR_NO_INFOSTR;
    }

    size_t outputSize = 0;

    // Check info string validity
    for (int i = 0; infoString[i] != '\0'; ++i) {
        switch(infoString[i]) {
            case 'i':
            case 'f':
            case 's':
                outputSize += VLIST_CHUNK_SIZE;
                break;
            default:
                return ERR_WRONG_INFOSTR;
        }
    }

    char* output = malloc(outputSize);

    void* currentPlace = output;
    char *variable = infoString;
    for(int i = 0; infoString[i] != '\0'; ++i) {
        // Check if the previous variable was the last one.
        size_t variableLength = strlen(variable);
        if(variable + variableLength == line + lineLength) {
            free(output);
            return ERR_VARIABLES_NUM;
        }
        variable += variableLength + 1;

        switch(infoString[i]) {
            case 'i': {
                variable = getNumber(variable);
                if (variable == NULL) {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }

                char* end;
                int integer = (int)strtol(variable, &end, 10);
                if(*end != '\0') {
                    free(output);
                    return ERR_INT_PARSE;
                }
                *(int*)currentPlace = integer;
                currentPlace += VLIST_CHUNK_SIZE;
                break;
            }
            case 'f': {
                variable = getNumber(variable);
                if (variable == NULL) {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }

                char* end;
                float floatingPoint = strtof(variable, &end);
                if(*end != '\0') {
                    free(output);
                    return ERR_FLOAT_PARSE;
                }
                *(float*)currentPlace = floatingPoint;
                currentPlace += VLIST_CHUNK_SIZE;
                break;
            }
            case 's': {
                variable = getString(variable);
                if (variable == NULL) {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }
                *(char**)currentPlace = variable;
                currentPlace += VLIST_CHUNK_SIZE;
                break;
            }
        }
    }
    dynamic_va_list args;
    dynamic_va_start(&args, output);

//    vlinda_output(infoString, args._valist);

    dynamic_va_end(&args);

//    linda_end();

    return 0;
}

void printErrorMessage(int status) {
    switch(status) {
        case ERR_NO_INFOSTR:
            printf("No info string\n");
            break;
        case ERR_WRONG_INFOSTR:
            printf("Could not parse info string\n");
            break;
        case ERR_VARIABLES_NUM:
            printf("Wrong number of variables\n");
            break;
        case ERR_INT_PARSE:
            printf("Could not parse integer\n");
            break;
        case ERR_FLOAT_PARSE:
            printf("Could not parse floating point number");
            break;
    }
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
