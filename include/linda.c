#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linda.h"
#include <stdarg.h>
#include <sys/shm.h>
#include <errno.h>

#define TUPLE_CONTENT_LENGTH 128
#define TUPLE_COUNT 128
#define MEMORY_KEY_ID 1001
#define INFO_STRING_PARAM_NOT_RECOGNIZED "Info string parameter not recognized"

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
int linda_init()
{
    /* Allocate a shared memory segment.  */
    //printf(stderr, "IPC_PRIVATE == %#lx\n", IPC_PRIVATE);
    key_t key = ftok("/tmp", 1); // z góry zadany id klucza naszego segemntu pozwala na obsluzenie wielu initow - 1 raz pamiec sie tworzy, potem tylko zwraca segemnt_id juz istniejacej pamieci
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
}

int linda_end()
{
    /* Detach the shared memory segment.  */
    if(shmdt(linda_memory) == -1)
    {
        perror("IPC error shmdt(). Cannot detach memory.");
    }

    /* Deallocate the shared memory segment.  */
    if(shmctl(linda_memory, IPC_RMID, NULL) == -1)
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
                break;
            }
        }
        if (input_tuple_length > TUPLE_CONTENT_LENGTH)
        {
            return 0;
        }
        ++info_string_position;
    }
    va_end(v_init);

    //Powiększyć przestrzeń w linda_memory o kolejną krotkę (+ zwiększyć licznik)
    //Zdobyć wskaźnik tuple * CurrentTuple na świeżo zaalokowaną krotkę
    linda_init();

    //3.
    size_t current_tuple_position = 0;
    struct tuple * current_tuple = linda_memory->first_tuple + linda_memory->tuple_count;
    linda_memory->tuple_count += 1;

    //Wrzucamy InfoString do pamięci
    memcpy(current_tuple->tuple_content + current_tuple_position, info_string_length, info_string_length + 1);
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


    linda_end();

    return 0;
}