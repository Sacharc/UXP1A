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

int input(char* line, size_t lineLength);
int output(char* line, size_t lineLength);
int read();

int checkIntegerMatchString(char*);
int checkFloatMatchString(char*);
int checkStringMatchString(char*);
int getNumberStart(char* string, char** numberStart);
char* getWord(char *str);
char* getWordsInsideQuotes(char *str);
char* trimWhitespace(char *str);
void freeInputContent(char *, char *);

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
//            linda_output("isf", 777, "testowe", 3.14);
            status = output(line, lineLength);
            if(status) {
                printErrorMessage(status);
            }
        } else if(strcmp(instruction, "INPUT") == 0) {
            status = input(line, lineLength);
            if(status) {
                printErrorMessage(status);
            }
        } else if(strcmp(instruction, "READ") == 0) {
            read();
        } else {
            printf("Instruction not recognized\n");
        }
    }

    free(rawLine);
//    linda_end();
}


#define ERR_NO_INFOSTR 1
#define ERR_WRONG_INFOSTR 2
#define ERR_VARIABLES_NUM 3
#define ERR_INT_PARSE 4
#define ERR_FLOAT_PARSE 5
#define ERR_UNRECOGNIZED_OPT 6
#define ERR_NEGATIVE_TIMEOUT 7
#define ERR_NO_MATCH_STRING 8
#define ERR_WRONG_MATCH_STRING 9
#define ERR_NO_TIMEOUT 10
#define ERR_NOT_FOUND 11

int input(char* line, size_t lineLength) {
    char* matchString = strtok(NULL, " \t");

    if(matchString == NULL) {
        return ERR_NO_MATCH_STRING;
    }

    // Check if timeout is specified
    char* option = strtok(NULL, " \t");

    int timeout = 0;
    if (option != NULL) {
        if(strcmp(option, "timeout") == 0) {
            char *timeoutString = strtok(NULL, " \t");
            if(timeoutString == NULL) {
                return ERR_NO_TIMEOUT;
            }
            // Check if it was the last option
            if (timeoutString + strlen(timeoutString) != line + lineLength) {
                return ERR_VARIABLES_NUM;
            }
            char *end;
            timeout = (int)strtol(timeoutString, &end, 10);
            if(*end != '\0') {
                return ERR_INT_PARSE;
            }
            if(timeout < 0) {
                return ERR_NEGATIVE_TIMEOUT;
            }
        } else {
            return ERR_UNRECOGNIZED_OPT;
        }
    }

    char* input = NULL;
    size_t offset = 0;

    size_t matchStringLength = strlen(matchString);

    // Maximum number of variables is the length of matchString
    char types[matchStringLength];
    types[0] = '\0';

    // We will need to modify matchString in order to extract it's parts, so we make copy of it first.
    char matchStringCopy[matchStringLength + 1];
    strcpy(matchStringCopy, matchString);

    char* typeString = strtok(matchStringCopy, ",");
    int error;

    while(1) {
        switch(typeString[0]) {
            case 'i':
                error = checkIntegerMatchString(typeString);
                if(error) {
                    freeInputContent(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "i");
                input = (char*)realloc(input, offset + sizeof(int*));
                int* integer = (int*)malloc(sizeof(int));
                *((int**)(input + offset)) = integer;
                offset += sizeof(int*);
                break;
            case 'f': {
                error = checkFloatMatchString(typeString);
                if(error) {
                    freeInputContent(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "f");
                input = (char*)realloc(input, offset + sizeof(float*));
                float* floatingPoint = (float*)malloc(sizeof(float));
                *((float**)(input + offset)) = floatingPoint;
                offset += sizeof(float*);
                break;
            }
            case 's':
                error = checkStringMatchString(typeString);
                if(error) {
                    freeInputContent(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "s");
                input = (char*)realloc(input, offset + sizeof(char**));
                char* string = (char*)malloc(sizeof(char) * 100);
                *((char**)(input + offset)) = string;
                offset += sizeof(char*);
                break;
            default:
                freeInputContent(input, types);
                free(input);
                return ERR_WRONG_MATCH_STRING;
        }
        typeString = strtok(NULL, ",");
        if(typeString == NULL) {
            break;
        }
    }

    // Get data
    dynamic_va_list va_list_linda;
    dynamic_va_start(&va_list_linda, input);

    // For testing purpose
    vscanf("%i %s %f", va_list_linda._va_list);

//    int error = vlinda_input(timeout, matchString, va_list_linda._va_list);
//    if(error) {
//        return ERR_NOT_FOUND;
//    }

    // Build format string for vprintf

    //6 characters per every printed variable and 2 characters for new line and 0
    size_t formatStringLength = matchStringLength * 6 + 2;
    char printfFormatString[formatStringLength];
    printfFormatString[0] = '\0';

    size_t typesLength = strlen(types);
    for(size_t i = 0; i < typesLength; ++i) {
        strncat(printfFormatString, types + i, 1);
        strcat(printfFormatString, ": %");
        strncat(printfFormatString, types + i, 1);
        strcat(printfFormatString, " ");
    }
    strcat(printfFormatString, "\n");

    // Get va_list for vprintf

    char* output = NULL;
    offset = 0;
    for(size_t i = 0; i < typesLength; ++i) {
        switch(types[i]) {
            case 'i':
                output = (char*)realloc(output, offset + VLIST_CHUNK_SIZE);
                *((int*)(output + offset)) = **((int**)(input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
            case 'f':
                output = (char*)realloc(output, offset + VLIST_CHUNK_SIZE);
                *((float*)(output + offset)) = **((float**)(input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
            case 's':
                output = (char*)realloc(output, offset + VLIST_CHUNK_SIZE);
                *((char**)(output + offset)) = *((char**)(input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
        }
    }

    dynamic_va_list va_list_printf;
    dynamic_va_start(&va_list_printf, output);

    vprintf(printfFormatString, va_list_printf._va_list);

    // Free memory
    dynamic_va_end(&va_list_printf);
    freeInputContent(input, types);
    dynamic_va_end(&va_list_linda);
    return 0;
}

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
                variable = getWord(variable);
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
                variable = getWord(variable);
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
                variable = getWordsInsideQuotes(variable);
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
    return 0;
}

void printErrorMessage(int status) {
    switch(status) {
        case ERR_NO_MATCH_STRING:
            printf("No match string\n");
            break;
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
            printf("Could not parse floating point number\n");
            break;
        case ERR_UNRECOGNIZED_OPT:
            printf("Unrecognized option\n");
            break;
        case ERR_NEGATIVE_TIMEOUT:
            printf("Negative timeout\n");
            break;
        case ERR_WRONG_MATCH_STRING:
            printf("Wrong match string\n");
            break;
        case ERR_NO_TIMEOUT:
            printf("No timeout\n");
            break;
        case ERR_NOT_FOUND:
            printf("Tuple not found\n");
            break;
    }
}

int read() {

}

char* getWord(char *string) {
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


char* getWordsInsideQuotes(char *string) {
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

int checkIntegerMatchString(char* string) {
    size_t length = strlen(string);
    if(length == 1) {
        return 0;
    } else if(length < 2) {
        return -1;
    }

    char* integerStart;
    int status = getNumberStart(string, &integerStart);

    if(status) {
        return status;
    }
    char* end;
    (int)strtol(integerStart, &end, 10);
    if(*end != '\0') {
        return status;
    }
    return 0;
}

int checkFloatMatchString(char* string) {
    size_t length = strlen(string);
    if(length == 1) {
        return 0;
    } else if(length < 2) {
        return -1;
    }

    char* floatStart;
    int status = getNumberStart(string, &floatStart);
    if(status) {
        return status;
    }
    char* end;
    strtof(floatStart, &end);
    if(*end != '\0') {
        return status;
    }
    return 0;
}

int checkStringMatchString(char* string) {
    size_t length = strlen(string);
    if(length == 1) {
        return 0;
    }
    // String has to be longer than 3 to contain s==string
    if(length < 4) {
        return -1;
    }

    if(string[1] != '=' || string[2] != '=') {
        return -1;
    }

    return 0;
}

int getNumberStart(char* string, char** numberStart) {
    size_t length = strlen(string);

    switch(string[1]) {
        case '=':
            // String has to be longer than 3 to contain i==int
            if(length < 4) {
                return -1;
            }
            if(string[2] == '=') {
                *numberStart = string + 3;
            } else {
                return -1;
            }
            break;
        case '<':
            if(string[2] == '=') {
                // String has to be longer than 3 to contain i<=int
                if(length < 4) {
                    return -1;
                }
                *numberStart = string + 3;
            } else {
                *numberStart = string + 2;
            }
            break;
        case '>':
            if(string[2] == '=') {
                // String has to be longer than 3 to contain i>=int
                if(length < 4) {
                    return -1;
                }
                *numberStart = string + 3;
            } else {
                *numberStart = string + 2;
            }
            break;
        default:
            return -1;
    }
    return 0;
}

void freeInputContent(char *input, char *types) {
    size_t length = strlen(types);
    char* currentPlace = input;
    for(size_t i = 0; i < length; ++i) {
        switch(types[i]) {
            case 'i':
                free(*((int**)currentPlace));
                currentPlace += sizeof(int*);
                break;
            case 'f':
                free(*((float**)currentPlace));
                currentPlace += sizeof(float*);
                break;
            case 's':
                free(*((char**)currentPlace));
                currentPlace += sizeof(char*);
                break;
        }
    }
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
