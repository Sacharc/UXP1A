#define _POSIX_C_SOURCE 200809L

#include <argp.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "include/linda.h"

#include "include/dynamic_va_list.h"

extern bool linda_logging;

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    switch (key)
    {
        case 'n':
        {
            linda_logging = false;
            break;
        }
    }
    return 0;
}

int input(char *line, size_t line_length, bool (*)(int, char*, va_list));

int output(char *line, size_t line_length);

int check_integer_match_string(char *);

int check_float_match_string(char *);

int check_string_match_string(char *);

int get_number_start(char *string, char **number_start);

char *get_word(char *str);

char *get_words_inside_quotes(char *str);

char *trim_whitespace(char *str);

void free_input_content(char *, char *);

void print_error_message(int status);

int main(int argc, char **argv)
{
    struct argp_option options[] =
            {
                    {"no-logging", 'n', 0, 0, "Disable logging to syslog"},
                    {0}
            };
    struct argp argp = {options, parse_opt, 0, 0};
    argp_parse(&argp, argc, argv, 0, 0, 0);

    int segment_id = linda_init();

    char *raw_line = NULL;
    size_t length = 0;

    int status;
    while (1)
    {
        printf(">");

        __ssize_t bytesRead = getline(&raw_line, &length, stdin);
        if (bytesRead == -1)
        {
            break;
        }

        char *line = trim_whitespace(raw_line);
        size_t line_length = strlen(line);
        if (line_length == 0)
        {
            continue;
        }

        if (strcmp(line, "EXIT") == 0)
        {
            break;
        }

        char *instruction = strtok(line, "  \t");
        if (strcmp(instruction, "OUTPUT") == 0)
        {
            status = output(line, line_length);
            if (status)
            {
                print_error_message(status);
            }
        }
        else if (strcmp(instruction, "INPUT") == 0)
        {
            status = input(line, line_length, &vlinda_input);
            if (status)
            {
                print_error_message(status);
            }
        }
        else if (strcmp(instruction, "READ") == 0)
        {
            status = input(line, line_length, &vlinda_read);
            if (status)
            {
                print_error_message(status);
            }
        }
        else
        {
            printf("Instruction not recognized\n");
        }
    }

    free(raw_line);
    linda_end(segment_id);
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

int input(char *line, size_t line_length, bool (*input_function)(int, char*, va_list))
{
    char *match_string = strtok(NULL, " \t");

    if (match_string == NULL)
    {
        return ERR_NO_MATCH_STRING;
    }

    // Check if timeout is specified
    char *option = strtok(NULL, " \t");

    int timeout = 0;
    if (option != NULL)
    {
        if (strcmp(option, "timeout") == 0)
        {
            char *timeout_string = strtok(NULL, " \t");
            if (timeout_string == NULL)
            {
                return ERR_NO_TIMEOUT;
            }
            // Check if it was the last option
            if (timeout_string + strlen(timeout_string) != line + line_length)
            {
                return ERR_VARIABLES_NUM;
            }
            char *end;
            timeout = (int) strtol(timeout_string, &end, 10);
            if (*end != '\0')
            {
                return ERR_INT_PARSE;
            }
            if (timeout < 0)
            {
                return ERR_NEGATIVE_TIMEOUT;
            }
        }
        else
        {
            return ERR_UNRECOGNIZED_OPT;
        }
    }

    char *input = NULL;
    size_t offset = 0;

    size_t match_string_length = strlen(match_string);

    // Maximum number of variables is the length of match_string
    char types[match_string_length];
    types[0] = '\0';

    // We will need to modify match_string in order to extract it's parts, so we make copy of it first.
    char match_string_copy[match_string_length + 1];
    strcpy(match_string_copy, match_string);

    char *type_string = strtok(match_string_copy, ",");

    int error;
    while (1)
    {
        switch (type_string[0])
        {
            case 'i':
                error = check_integer_match_string(type_string);
                if (error)
                {
                    free_input_content(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "i");
                input = (char *) realloc(input, offset + sizeof(int *));
                int *integer = (int *) malloc(sizeof(int));
                *((int **) (input + offset)) = integer;
                offset += sizeof(int *);
                break;
            case 'f':
            {
                error = check_float_match_string(type_string);
                if (error)
                {
                    free_input_content(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "f");
                input = (char *) realloc(input, offset + sizeof(double *));
                double *floating_point = (double *) malloc(sizeof(double));
                *((double **) (input + offset)) = floating_point;
                offset += sizeof(double *);
                break;
            }
            case 's':
                error = check_string_match_string(type_string);
                if (error)
                {
                    free_input_content(input, types);
                    free(input);
                    return ERR_WRONG_MATCH_STRING;
                }
                strcat(types, "s");
                input = (char *) realloc(input, offset + sizeof(char **));
                char *string = (char *) malloc(sizeof(char) * 100);
                *((char **) (input + offset)) = string;
                offset += sizeof(char *);
                break;
            default:
                free_input_content(input, types);
                free(input);
                return ERR_WRONG_MATCH_STRING;
        }
        type_string = strtok(NULL, ",");
        if (type_string == NULL)
        {
            break;
        }
    }

    // Get data
    dynamic_va_list va_list_linda;
    dynamic_va_start(&va_list_linda, input);


    bool found = (*input_function)(timeout, match_string, va_list_linda._va_list);
    if(!found) {
        free_input_content(input, types);
        dynamic_va_end(&va_list_linda);
        return ERR_NOT_FOUND;
    }

    // Build format string for vprintf

    //6 characters per every printed variable and 2 characters for new line and 0
    size_t format_string_length = match_string_length * 6 + 2;
    char printf_format_string[format_string_length];
    printf_format_string[0] = '\0';

    size_t types_length = strlen(types);
    for (size_t i = 0; i < types_length; ++i)
    {
        strncat(printf_format_string, types + i, 1);
        strcat(printf_format_string, ": %");
        strncat(printf_format_string, types + i, 1);
        strcat(printf_format_string, " ");
    }
    strcat(printf_format_string, "\n");

    // Get va_list for vprintf

    char *output = NULL;
    offset = 0;
    for (size_t i = 0; i < types_length; ++i)
    {
        switch (types[i])
        {
            case 'i':
                output = (char *) realloc(output, offset + VLIST_CHUNK_SIZE);
                *((int *) (output + offset)) = **((int **) (input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
            case 'f':
                output = (char *) realloc(output, offset + VLIST_CHUNK_SIZE);
                *((double *) (output + offset)) = **((double **) (input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
            case 's':
                output = (char *) realloc(output, offset + VLIST_CHUNK_SIZE);
                *((char **) (output + offset)) = *((char **) (input + offset));
                offset += VLIST_CHUNK_SIZE;
                break;
        }
    }

    dynamic_va_list va_list_printf;
    dynamic_va_start(&va_list_printf, output);

    vprintf(printf_format_string, va_list_printf._va_list);

    // Free memory
    dynamic_va_end(&va_list_printf);
    free_input_content(input, types);
    dynamic_va_end(&va_list_linda);
    return 0;
}

int output(char *line, size_t line_length)
{
    char *info_string = strtok(NULL, " \t");

    if (info_string == NULL)
    {
        return ERR_NO_INFOSTR;
    }

    size_t output_size = 0;

    // Check info string validity
    for (int i = 0; info_string[i] != '\0'; ++i)
    {
        switch (info_string[i])
        {
            case 'i':
            case 'f':
            case 's':
                output_size += VLIST_CHUNK_SIZE;
                break;
            default:
                return ERR_WRONG_INFOSTR;
        }
    }

    char *output = malloc(output_size);

    void *current_place = output;
    char *variable = info_string;
    for (int i = 0; info_string[i] != '\0'; ++i)
    {
        // Check if the previous variable was the last one.
        size_t variable_length = strlen(variable);
        if (variable + variable_length == line + line_length)
        {
            free(output);
            return ERR_VARIABLES_NUM;
        }
        variable += variable_length + 1;

        switch (info_string[i])
        {
            case 'i':
            {
                variable = get_word(variable);
                if (variable == NULL)
                {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }

                char *end;
                int integer = (int) strtol(variable, &end, 10);
                if (*end != '\0')
                {
                    free(output);
                    return ERR_INT_PARSE;
                }
                *(int *) current_place = integer;
                current_place += VLIST_CHUNK_SIZE;
                break;
            }
            case 'f':
            {
                variable = get_word(variable);
                if (variable == NULL)
                {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }

                char *end;
                double floating_point = strtof(variable, &end);
                if (*end != '\0')
                {
                    free(output);
                    return ERR_FLOAT_PARSE;
                }
                *(double *) current_place = floating_point;
                current_place += VLIST_CHUNK_SIZE;
                break;
            }
            case 's':
            {
                variable = get_words_inside_quotes(variable);
                if (variable == NULL)
                {
                    free(output);
                    return ERR_VARIABLES_NUM;
                }
                *(char **) current_place = variable;
                current_place += VLIST_CHUNK_SIZE;
                break;
            }
        }
    }
    dynamic_va_list args;
    dynamic_va_start(&args, output);

    vlinda_output(info_string, args._va_list);

    dynamic_va_end(&args);
    return 0;
}

void print_error_message(int status)
{
    switch (status)
    {
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

char *get_word(char *string)
{
    size_t length = strlen(string);

    char *word_start = NULL;
    for (size_t i = 0; i < length; ++i)
    {
        if (!isspace(string[i]))
        {
            word_start = string + i;
            break;
        }
    }

    if (word_start == NULL)
    {
        return NULL;
    }

    length = strlen(word_start);

    for (size_t i = 1; i < length; ++i)
    {
        if (isspace(word_start[i]))
        {
            word_start[i] = '\0';
            break;
        }
    }
    return word_start;
}

char *get_words_inside_quotes(char *string)
{
    size_t length = strlen(string);

    char *str_start = NULL;
    for (size_t i = 0; i < length; ++i)
    {
        if (!isspace(string[i]))
        {
            // String must start with '"'
            if (string[i] == '"')
            {
                str_start = string + i + 1;
            }
            break;
        }
    }

    if (str_start == NULL)
    {
        return NULL;
    }

    length = strlen(str_start);

    bool end_of_string_found = false;
    for (size_t i = 0; i < length; ++i)
    {
        if (str_start[i] == '"')
        {
            str_start[i] = '\0';
            end_of_string_found = true;
            break;
        }
    }

    if (end_of_string_found)
    {
        return str_start;
    }
    else
    {
        return NULL;
    }
}

int check_integer_match_string(char *string)
{
    size_t length = strlen(string);
    if (length == 1)
    {
        return 0;
    }
    else if (length < 2)
    {
        return -1;
    }

    char *integer_start;
    int status = get_number_start(string, &integer_start);

    if (status)
    {
        return status;
    }
    char *end;
    (int) strtol(integer_start, &end, 10);
    if (*end != '\0')
    {
        return status;
    }
    return 0;
}

int check_float_match_string(char *string)
{
    size_t length = strlen(string);
    if (length == 1)
    {
        return 0;
    }
    else if (length < 2)
    {
        return -1;
    }

    char *float_start;
    int status = get_number_start(string, &float_start);
    if (status)
    {
        return status;
    }
    char *end;
    strtof(float_start, &end);
    if (*end != '\0')
    {
        return status;
    }
    return 0;
}

int check_string_match_string(char *string)
{
    size_t length = strlen(string);
    if (length == 1)
    {
        return 0;
    }
    // String has to be longer than 3 to contain s==string
    if (length < 4)
    {
        return -1;
    }

    if (string[1] != '=' || string[2] != '=')
    {
        return -1;
    }

    return 0;
}

int get_number_start(char *string, char **number_start)
{
    size_t length = strlen(string);

    switch (string[1])
    {
        case '=':
            // String has to be longer than 3 to contain i==int
            if (length < 4)
            {
                return -1;
            }
            if (string[2] == '=')
            {
                *number_start = string + 3;
            }
            else
            {
                return -1;
            }
            break;
        case '<':
            if (string[2] == '=')
            {
                // String has to be longer than 3 to contain i<=int
                if (length < 4)
                {
                    return -1;
                }
                *number_start = string + 3;
            }
            else
            {
                *number_start = string + 2;
            }
            break;
        case '>':
            if (string[2] == '=')
            {
                // String has to be longer than 3 to contain i>=int
                if (length < 4)
                {
                    return -1;
                }
                *number_start = string + 3;
            }
            else
            {
                *number_start = string + 2;
            }
            break;
        default:
            return -1;
    }
    return 0;
}

void free_input_content(char *input, char *types)
{
    size_t length = strlen(types);
    char *current_place = input;
    for (size_t i = 0; i < length; ++i)
    {
        switch (types[i])
        {
            case 'i':
                free(*((int **) current_place));
                current_place += sizeof(int *);
                break;
            case 'f':
                free(*((double **) current_place));
                current_place += sizeof(double *);
                break;
            case 's':
                free(*((char **) current_place));
                current_place += sizeof(char *);
                break;
        }
    }
}

//http://stackoverflow.com/a/122721/5459240
char *trim_whitespace(char *str)
{
    char *end;

    // Trim leading space
    while (isspace(*str))
    {
        str++;
    }

    if (*str == 0)
        return str;

    // Trim trailing space
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        end--;
    }

    // Write new null terminator
    *(end + 1) = 0;

    return str;
}
