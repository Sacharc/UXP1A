#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linda.h"
#include <stdarg.h>
#include <sys/shm.h>
#include <assert.h>

#define TUPLE_CONTENT_LENGTH 128
#define TUPLE_COUNT 128
#define INFO_STRING_PARAM_NOT_RECOGNIZED "Info string parameter not recognized"
#define FTOK_PATH "/tmp"

struct tuple
{
    char tuple_content[TUPLE_CONTENT_LENGTH];
};

struct mem
{
    size_t tuple_count;
    struct tuple first_tuple[TUPLE_COUNT];
};

//Zwraca rozmiar skopiowanych danych
size_t int_to_tuple(int input, char * output)
{
    memcpy(output, &input, sizeof(input));
    return sizeof(input);
}

size_t double_to_tuple(double input, char * output)
{
    memcpy(output, &input, sizeof(input));
    return sizeof(input);
}

size_t string_to_tuple(const char * input, char * output)
{
    size_t length = strlen(input) + 1;
    memcpy(output, input, length);
    return length;
}

struct mem * linda_memory = NULL;

// TODO? http://stackoverflow.com/questions/16423789/can-i-resize-linux-shared-memory-with-shmctl
//Return segment_id if operation success, else return -1
int linda_init()
{
    /* Allocate a shared memory segment.  */
    key_t key = ftok(FTOK_PATH, 1);
    if (key == (key_t) -1)
    {
        perror("IPC error: ftok");
    }
    int segment_id = shmget(key, sizeof(struct mem), IPC_CREAT | 0660);
    if (segment_id == -1)
    {
        perror("IPC error: shmget. Probably doesn't exist.");
    }
    /* Attach the shared memory segment.  */
    linda_memory = (struct mem*) shmat (segment_id, NULL, 0);

    /* Check number of currently attached processes. If we are the first process, we are obligated to initialize the memory */
    struct shmid_ds shm_data;
    if(shmctl(segment_id, IPC_STAT, &shm_data) == -1)
    {
        perror("IPC error: shmctl()");
    }

    if (shm_data.shm_nattch == 1)
    {
        linda_memory->tuple_count = 0;
    }
    return segment_id;
}

int linda_end(int segment_id)
{
    /* Detach the shared memory segment.  */
    if(shmdt(linda_memory) == -1)
    {
        perror("IPC error shmdt(). Cannot detach memory.");
    }

    /* Deallocate the shared memory segment.  */
    if(shmctl(segment_id, IPC_RMID, NULL) == -1)
    {
        perror("IPC error shmctl(). Cannot deallocate shared memory.");
    }
}

bool linda_output(char * info_string, ...)
{

    //TODO exctract method with validation
    const size_t info_string_length = strlen(info_string);

    size_t input_tuple_length = info_string_length + 1;

    va_list v_init;
    va_start(v_init, info_string);
    size_t info_string_position = 0;
    while (info_string[info_string_position] != 0)
    {
        switch(info_string[info_string_position])
        {
            case 'i':
            {
                va_arg(v_init, int);
                input_tuple_length += sizeof(int);
                break;
            }
            case 'f':
            {
                va_arg(v_init, double); //va_arg nie przyjmuje float'a - korzystamy z double
                input_tuple_length += sizeof(double);
                break;
            }
            case 's':
            {
                char * c = va_arg(v_init, char *);
                input_tuple_length += strlen(c) + 1;
                break;
            }
            default:
            {
                perror(INFO_STRING_PARAM_NOT_RECOGNIZED);
                return false;
            }
        }
        if (input_tuple_length > TUPLE_CONTENT_LENGTH)
        {
            perror("Tuple content length exceeds max size.");
            return false;
        }
        ++info_string_position;
    }
    va_end(v_init);

    //Powiększyć przestrzeń w linda_memory o kolejną krotkę (+ zwiększyć licznik)
    //Zdobyć wskaźnik tuple * CurrentTuple na świeżo zaalokowaną krotkę

    //3.
    size_t current_tuple_position = 0;
    struct tuple * current_tuple = linda_memory->first_tuple + linda_memory->tuple_count;
    linda_memory->tuple_count += 1;

    //Wrzucamy InfoString do pamięci
    memcpy(current_tuple->tuple_content + current_tuple_position, info_string, info_string_length + 1);
    current_tuple_position += info_string_length + 1;

    //Wrzucamy dane do pamięci

    //data to tuple
    va_list vl;
    va_start(vl, info_string);
    info_string_position = 0;
    while(info_string[info_string_position] != 0)
    {
        switch(info_string[info_string_position])
        {
            case 'i':
            {
                current_tuple_position += int_to_tuple((int) va_arg(vl, int), current_tuple->tuple_content + current_tuple_position);
                break;
            }
            case 'f':
            {
                current_tuple_position += double_to_tuple((double) va_arg(vl, double), current_tuple->tuple_content + current_tuple_position);
                break;
            }
            case 's':
            {
                current_tuple_position += string_to_tuple((char *) va_arg(vl, char *), current_tuple->tuple_content + current_tuple_position);
                break;
            }
            default:
            {
                perror(INFO_STRING_PARAM_NOT_RECOGNIZED);
                break;
            }
        }

        ++info_string_position;
    }
    va_end(vl);

    return true;
}

//Read tuple functions

char* strdup(const char* p)
{
    char* np = (char*)malloc(strlen(p)+1);
    return np ? strcpy(np, p) : np;
}

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

/**
 * Checks if match string and info string match. Method doesn't check conditions, just checks types.
 *
 */
bool match_string_and_info_string_match(char *const *tokens, const char *tuple_content_read) {
    size_t tuple_content_iterator = 0;
    for (; tuple_content_read[tuple_content_iterator] != 0; ++tuple_content_iterator)
    {

        if (tuple_content_read[tuple_content_iterator] != *(tokens + tuple_content_iterator))
        {
            //match string and tuple info string doesn't match
            return false;
        }
    }

    if (*(tokens + tuple_content_iterator) != '\0')
    {
        //match_string is longer than info string
        return false;
    }

    return true;
}

/**
 * Checks if tuple contains values from match string.
 */
bool tuple_values_contain_match_string_values(char *const *match_string_tokens, const char *tuple_content_read)
{
    //TODO implement this method
}

void free_match_string_tokens_memory(char **match_string_tokens) {
    if (match_string_tokens)
    {
        for (size_t i = 0; *(match_string_tokens + i); ++i)
        {
            free(*(match_string_tokens + i));
        }
        free(match_string_tokens);
    }
}

/**
 * Reads the tuple from shared memory.
 *
 * @param  match_string the pointer to match_string
 * @return tuple number, or -1 if fails.
 */
int readTuple(char *match_string)
{
    char** match_string_toneks = str_split(match_string, ',');
    if (match_string_toneks)
    {
        for (size_t tuple_iterator; tuple_iterator < TUPLE_COUNT; ++tuple_iterator)
        {
            struct tuple tuple_read = linda_memory->first_tuple[tuple_iterator];
            if (match_string_and_info_string_match(match_string_toneks, tuple_read.tuple_content)
                    && tuple_values_contain_match_string_values(match_string_toneks, tuple_read.tuple_content))
            {
                return tuple_iterator;
            }
        }

        free_match_string_tokens_memory(match_string_toneks);
        return -1;
    }
    return -1;

}

bool linda_input(int timeout, char* match_string, ...)
{
    int tuple_index = readTuple(match_string);
    if (tuple_index == -1)
    {
        perror("Matched tuple not found");
    }

    struct tuple * found_tuple = linda_memory->first_tuple + tuple_index;
    size_t  info_string_length = strlen(found_tuple->tuple_content) + 1;

    char * info_string;
    memcpy(info_string, found_tuple->tuple_content, info_string_length);

    va_list vl;
    va_start(vl, info_string);
    size_t info_string_position = 0;
    size_t found_tuple_position = info_string_length;

    //Memcpy for arguments in va_list
    while(info_string[info_string_position] != 0)
    {
        switch(info_string[info_string_position])
        {
            case 'i':
            {
                memcpy(va_arg(vl, int *), found_tuple->tuple_content + found_tuple_position, sizeof(int));
                found_tuple_position += sizeof(int);
                break;
            }
            case 'f':
            {
                memcpy(va_arg(vl, double *), found_tuple->tuple_content + found_tuple_position, sizeof(double));
                found_tuple_position += sizeof(double);
                break;
            }
            case 's':
            {
                size_t string_length = strlen(found_tuple->tuple_content + found_tuple_position);
                memcpy(va_arg(vl, char *), found_tuple->tuple_content + found_tuple_position, string_length);
                found_tuple_position += string_length;
                break;
            }
            default:
            {
                perror(INFO_STRING_PARAM_NOT_RECOGNIZED);
                break;
            }
        }
        ++info_string_position;
    }
    va_end(vl);


    /*Delete this tuple by replacing it and decrement
    Each tuple goes to its index-1, last index is not modify*/

    //Replace tuple index by decr by 1
    while (tuple_index < linda_memory->tuple_count - 1)
    {
        memcpy(linda_memory->first_tuple + tuple_index, linda_memory->first_tuple + tuple_index + 1, TUPLE_COUNT);
        ++tuple_index;
    }

    //Last index to NULL
    memset(linda_memory->first_tuple + tuple_index, 0, sizeof linda_memory->first_tuple + tuple_index);
    --linda_memory->tuple_count;

}