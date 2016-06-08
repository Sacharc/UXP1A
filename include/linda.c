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
#define OPERATOR_NOT_REGOGNIZED "Operator not recognized"
#define FTOK_PATH "/tmp"
#define OPERATOR_FIRST_CHARACTER_INDEX 1
#define OPERATOR_SECOND_CHARACTER_INDEX 2
#define OPERATOR_STRING_LENGTH_INDEX 2
#define FIRST_CHARACTER_TO_COMPARE_INDEX 3
#define VALUE_TYPE_INDEX 0
#define IS_INT 0
#define IS_FLOAT 1

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



















bool compare_string(const char * operator, const char * string_a, const char * string_b)
{
    if(strcmp(operator, "==") == 0)
        return strcmp(string_a, string_b) == 0;
    if(strcmp(operator, ">=") == 0)
        return strcmp(string_a, string_b) >= 0;
    if (strcmp(operator, "<=") == 0)
        return strcmp(string_a, string_b) <= 0;
    if (strcmp(operator, ">") == 0)
        return strcmp(string_a, string_b) > 0;
    if (strcmp(operator, "<") == 0)
        return strcmp(string_a, string_b) < 0;

    perror("Operator comparison error");
    return false;
}
bool compare_int(const char * operator, int a, int b)
{
    //TODO
}
bool compare_double(const char * operator, double a, double b)
{
    //TODO
}
























/**
		Sprawdza, czy info_string i match_string definiują taką samą krotkę
*/
bool info_string_match_string_equals(const char * info_string, const char * match_string)
{
    size_t info_string_position = 0;
    size_t match_string_position = 0;

    for(;;)
    {
        //Jeśli znaki nie są identyczne
        if(info_string[info_string_position] != match_string[match_string_position])
            return false;

        //Jeśli oba się kończą (to w zasadzie ma sens tylko dla pierwszego znaku)
        if(info_string[info_string_position] == NULL)
            return true;

        //Przesuwamy iterator info_string o jeden znak do przodu
        info_string_position++;

        //Przesuwamy iterator match_string na znak po przecinku (albo null)
        for(;;)
        {
            if(match_string[match_string_position] == ',' || match_string[match_string_position] == NULL)
                break;
            match_string_position++;
        }

        //Jeśli match-string się skończył - sprawdzamy czy info_string też się skończył
        if(match_string[match_string_position] == NULL)
            return info_string[info_string_position] == NULL;

        //Nie - jesteśmy na przecinku, przesuwamy się za niego
        match_string_position++;
    }
}

bool tuple_match_match_string(const struct tuple * tuple_to_match, const char * match_string)
{
    //Sprawdzamy, czy w ogóle typy są zgodne
    if(!info_string_match_string_equals(tuple_to_match->tuple_content, match_string))
        return false;


    size_t tuple_to_match_position = strlen(&tuple_to_match->tuple_content[0]) + 1; //pomijamy info_string

    //Właściwe filtry
    //current_match_string_token_start wskazuje na pierwszy znak filtra
    //current_match_string_token_end wskazuje na ostatni znak filtra

    const char * current_match_string_token_start = match_string;
    for(;;)
    {
        //Wyciągamy tekst od obecnej pozycji do nulla albo przecinka.
        //Pozycję zwraca nam strchr
        char current_match_string_token[TUPLE_CONTENT_LENGTH];

        //Szukamy ',' albo końca tekstu
        const char * current_match_string_token_end = strchr(current_match_string_token_start, ',');
        if(current_match_string_token_end == NULL) //Przecinka nie ma, jest koniec tekstu.
            current_match_string_token_end = current_match_string_token_start + strlen(current_match_string_token_start);

        //Długość od start do end
        const size_t current_match_string_token_length = current_match_string_token_end - current_match_string_token_start;

        //Wykopiowujemy do dedykowanego bufora
        strncpy(current_match_string_token, current_match_string_token_start, current_match_string_token_length);

        //Do kolejnego obiegu pętli przesuwamy start za end. current_match_string_token_start może być nieprawidłowy (wskazywać za końcem), ale to jeszcze sprawdzi nam check na dole
        current_match_string_token_start = current_match_string_token_end + 1;

        //Pomijamy 1, bo jeśli jest samo i / f / s to info_string_match_string_equals już to sprawdziło
        if(current_match_string_token_length > 1)
        {
            switch(current_match_string_token[0])
            {
                case 'i':
                {
                    //0. Wyciągnąć operator
                    //1. wyciągnąć int z tuple_to_match[tuple_to_match_position]
                    //2. wyciągnąć int z tekstu za operatorem
                    //3. Jeśli nie udało się compare_int - zwrócić false
                    break;
                }
            }
        }


        //Czy to koniec napisu? Jeśli wcześniej nie wyszliśmy - zwracamy true
        if(*current_match_string_token_end == NULL)
            return true;
    }
}

/**
 * Reads the tuple from shared memory.
 *
 * @param  match_string the pointer to match_string
 * @return tuple number, or -1 if fails.
 */
int extract_tuple_from_shmem(const char * match_string)
{
    unsigned int tuple_id = 0;
    for(;;)
    {
        struct tuple * tuple_to_match = &linda_memory->first_tuple[tuple_id];
        if(tuple_match_match_string(tuple_to_match, match_string))
            return tuple_id;
    }

    return -1;
}

bool linda_input(int timeout, char* match_string, ...)
{
    int tuple_index = extract_tuple_from_shmem(match_string);

    if (tuple_index == -1)
    {
        perror("Matched tuple not found");
        return false;
    }

    //Otrzymaliśmy krotkę z extract_tuple_from_shmem, więc jej dane na pewno zgadzają się z tym, co w va_list
    const struct tuple * found_tuple = &linda_memory->first_tuple[tuple_index];
    const size_t info_string_length = strlen(found_tuple->tuple_content);

    size_t info_string_position = 0;
    size_t tuple_position = info_string_length + 1;

    va_list vl;
    va_start(vl, match_string);

    //Memcpy for arguments in va_list
    while(found_tuple->tuple_content[info_string_position] != NULL)
    {
        switch(found_tuple->tuple_content[info_string_position])
        {
            case 'i':
            {
                memcpy(va_arg(vl, int *), &found_tuple->tuple_content[0] + tuple_position, sizeof(int));
                tuple_position += sizeof(int);
                break;
            }
            case 'f':
            {
                memcpy(va_arg(vl, double *), &found_tuple->tuple_content[0] + tuple_position, sizeof(double));
                tuple_position += sizeof(double);
                break;
            }
            case 's':
            {
                const size_t string_length = strlen(&found_tuple->tuple_content[0] + tuple_position);
                memcpy(va_arg(vl, char *), &found_tuple->tuple_content[0] + tuple_position, string_length + 1);
                tuple_position += string_length + 1;
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
    /*while (tuple_index < linda_memory->tuple_count - 1)
    {
        memcpy(linda_memory->first_tuple + tuple_index, linda_memory->first_tuple + tuple_index + 1, TUPLE_COUNT);
        ++tuple_index;
    }

    //Last index to NULL
    memset(linda_memory->first_tuple + tuple_index, 0, sizeof linda_memory->first_tuple + tuple_index);
    --linda_memory->tuple_count;*/

    //Kopiuje wszystkie krotki za wyciąganą o jedno miejsce do tyłu. Zmniejsza licznik krotek
    memcpy(&linda_memory->first_tuple[tuple_index], &linda_memory->first_tuple[tuple_index + 1], (--linda_memory->tuple_count - tuple_index) * sizeof(struct tuple));

}