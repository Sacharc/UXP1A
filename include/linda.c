#include "linda.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/shm.h>
#include <assert.h>
#include <errno.h>

#define INFO_STRING_PARAM_NOT_RECOGNIZED "Info string parameter not recognized"
#define MATCH_STRING_PARAM_NOT_RECOGNIZED "Match string parameter not recognized"
#define OPERATOR_NOT_REGOGNIZED "Operator not recognized"
#define FTOK_PATH "/tmp"

struct mem * linda_memory = NULL;
int linda_segment_id = 0;

bool linda_init()
{
	//Create virtual memory key
	key_t key = ftok(FTOK_PATH, 1);
	if(key == (key_t) -1)
	{
		printf("IPC error: ftok: %d", errno);
		return false;
	}
	
	//Allocate a shared memory segment.
	linda_segment_id = shmget(key, sizeof(struct mem), IPC_CREAT | 0660);
	if(linda_segment_id == -1)
	{
		printf("IPC error: shmget: %d", errno);
		return false;
	}
	
	//Map shared memory to our VM
	linda_memory = (struct mem*) shmat (linda_segment_id, NULL, 0);
	if(linda_memory == NULL)
	{
		printf("IPC error: linda_memory: %d", errno);
		return false;
	}

	/* Check number of currently attached processes. If we are the first process, we are obligated to initialize the memory */
	struct shmid_ds shm_data;
	if(shmctl(linda_segment_id, IPC_STAT, &shm_data) == -1)
	{
		printf("IPC error: shmctl(): %d", errno);
		return false;
	}

	if(shm_data.shm_nattch == 1)
	{
		linda_memory->tuple_count = 0;
	}
	
	return true;
}

void linda_end()
{
	/* Detach the shared memory segment.  */
	if(shmdt(linda_memory) == -1)
	{
		printf("IPC error shmdt(). Cannot detach memory.");
	}

	/* Deallocate the shared memory segment.  */
	if(shmctl(linda_segment_id, IPC_RMID, NULL) == -1)
	{
		printf("IPC error shmctl(). Cannot deallocate shared memory.");
	}
}

//OUTPUT Functions
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

bool linda_output(const char * info_string, ...)
{
	va_list v_init;
	va_start(v_init, info_string);
	bool ret = vlinda_output(info_string, &v_init);
	va_end(v_init);

	return ret;
}

bool vlinda_output(const char * info_string, va_list * v_init)
{
	if(linda_memory->tuple_count >= TUPLE_COUNT)
	{
		printf("Tuple memory exhausted (%lu) (%u)", linda_memory->tuple_count, TUPLE_COUNT);
		return false;
	}
	
	const size_t info_string_length = strlen(info_string);
	size_t current_tuple_length = info_string_length + 1;
	
	//Kopiujemy listę argumentów do sprawdzenia (pierwsza pętla) i wyciągania danych do krotki (druga pętla)
	va_list va_check, va_read;
	va_copy(va_check, *v_init);
	
	size_t info_string_position = 0;	
	while (info_string[info_string_position] != 0)
	{
		switch(info_string[info_string_position])
		{
			case 'i':
			{
				va_arg(va_check, int);
				current_tuple_length += sizeof(int);
				break;
			}
			case 'f':
			{
				va_arg(va_check, double);
				current_tuple_length += sizeof(double);
				break;
			}
			case 's':
			{
				char * c = va_arg(va_check, char *);
				current_tuple_length += strlen(c) + 1;
				break;
			}
			default:
			{
				printf("Unknown character in info_string: `%c` (%d)", info_string[info_string_position], info_string[info_string_position]);
				return false;
			}
		}
		
		if(current_tuple_length > TUPLE_CONTENT_LENGTH)
		{
			printf("Tuple content length (%lu) exceeds max size (%d)", current_tuple_length, TUPLE_CONTENT_LENGTH);
			return false;
		}
		
		++info_string_position;
	}
	
	va_end(va_check);


	//Bierzemy wskaźnik na krotkę, przesuwamy 
	size_t current_tuple_position = 0;
	struct tuple * current_tuple = linda_memory->first_tuple + linda_memory->tuple_count;
	linda_memory->tuple_count++;

	//Wrzucamy info_string do pamięci
	memcpy(current_tuple->tuple_content, info_string, info_string_length + 1);
	current_tuple_position += info_string_length + 1;

	//Wrzucamy dane do pamięci
	va_copy(va_read, *v_init);
	
	info_string_position = 0;
	while(info_string[info_string_position] != 0)
	{
		switch(info_string[info_string_position])
		{
			case 'i':
			{
				current_tuple_position += int_to_tuple((int) va_arg(va_read, int), current_tuple->tuple_content + current_tuple_position);
				break;
			}
			case 'f':
			{
				current_tuple_position += double_to_tuple((double) va_arg(va_read, double), current_tuple->tuple_content + current_tuple_position);
				break;
			}
			case 's':
			{
				current_tuple_position += string_to_tuple((char *) va_arg(va_read, char *), current_tuple->tuple_content + current_tuple_position);
				break;
			}
			default:
			{
				printf(INFO_STRING_PARAM_NOT_RECOGNIZED);
				break;
			}
		}

		++info_string_position;
	}
	
	va_end(va_read);

	return true;
}


//INPUT FUNCTIONS
bool compare_string(const char * operator_, const char * string_a, const char * string_b)
{
	if(strcmp(operator_, "==") == 0)
		return strcmp(string_a, string_b) == 0;
	if(strcmp(operator_, ">=") == 0)
		return strcmp(string_a, string_b) >= 0;
	if(strcmp(operator_, "<=") == 0)
		return strcmp(string_a, string_b) <= 0;
	if(strcmp(operator_, ">") == 0)
		return strcmp(string_a, string_b) > 0;
	if(strcmp(operator_, "<") == 0)
		return strcmp(string_a, string_b) < 0;

	printf("Operator comparison error");
	return false;
}

bool compare_int(const char * operator_, int a, int b)
{
	if(strcmp(operator_, "==") == 0)
		return a == b;
	if(strcmp(operator_, ">=") == 0)
		return a >= b;
	if(strcmp(operator_, "<=") == 0)
		return a <= b;
	if(strcmp(operator_, ">") == 0)
		return a > b;
	if(strcmp(operator_, "<") == 0)
		return a < b;

	printf("Operator comparison error");
	return false;
}

bool compare_double(const char * operator_, double a, double b)
{
	if(strcmp(operator_, "==") == 0)
	{
		printf("Cant perform equal for float");
		return false;
	}
	if(strcmp(operator_, ">=") == 0)
		return a >= b;
	if(strcmp(operator_, "<=") == 0)
		return a <= b;
	if(strcmp(operator_, ">") == 0)
		return a > b;
	if(strcmp(operator_, "<") == 0)
		return a < b;

	printf("Operator comparison error");
	return false;
}

/**
	Sprawdza, czy info_string i match_string definiują taką samą krotkę
	
	Testowane, działa
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
		if(info_string[info_string_position] == 0)
			return true;

		//Przesuwamy iterator info_string o jeden znak do przodu
		info_string_position++;

		//Przesuwamy iterator match_string na znak po przecinku (albo null)
		for(;;)
		{
			if(match_string[match_string_position] == ',' || match_string[match_string_position] == 0)
				break;
			match_string_position++;
		}

		//Jeśli match-string się skończył - sprawdzamy czy info_string też się skończył
		if(match_string[match_string_position] == 0)
			return info_string[info_string_position] == 0;

		//Nie - jesteśmy na przecinku, przesuwamy się za niego
		match_string_position++;
	}
}

/**
	Z podanego match_string wyciąga operator i umieszcza go w output_operator. output_operator musi mieć minimum 3 znaki.
	Zwraca wskaźnik na pierwszy znak za operatorem
*/
const char * match_string_extract_operator(const char * match_string, char * output_operator)
{
	if(match_string[2] == '=')
	{
		strncpy(output_operator, &match_string[1], 2);
		output_operator[2] = 0;
		return match_string + 3;
	}
	else
	{
		strncpy(output_operator, &match_string[1], 1);
		output_operator[1] = 0;
		return match_string + 2;
	}
}

/**
	Sprawdza, czy podana krotka pasuje do podanego wzorca
*/
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
		current_match_string_token[current_match_string_token_length] = 0;

		//Do kolejnego obiegu pętli przesuwamy start za end. current_match_string_token_start może być nieprawidłowy (wskazywać za końcem), ale to jeszcze sprawdzi nam check na dole
		current_match_string_token_start = current_match_string_token_end + 1;
		
		
		//Jesli nie sprawdzamy - przesuwamy wskaźnik krotki do przodu
		if(current_match_string_token_length == 1)
		{
			switch(current_match_string_token[0])
			{
				case 'i':
				{
					tuple_to_match_position += sizeof(int);
					break;
				}
				case 'f':
				{
					tuple_to_match_position += sizeof(double);
					break;
				}
				case 's':
				{
					tuple_to_match_position += strlen(&tuple_to_match->tuple_content[0] + tuple_to_match_position) + 1;
					break;
				}
				default:
				{
					printf(MATCH_STRING_PARAM_NOT_RECOGNIZED);
					break;
				}
			}
		}
		else
		{
			char operator_[3];
			const char * match_string_post_operator = match_string_extract_operator(current_match_string_token, operator_);
			//printf("Iteracja tuple_match_match_string: `%c` `%s` `%s`\n", current_match_string_token[0], operator, match_string_post_operator);
			
			switch(current_match_string_token[0])
			{
				case 'i':
				{
					int tuple_int;
					memcpy(&tuple_int, &tuple_to_match->tuple_content[0] + tuple_to_match_position, sizeof(int));
					int match_string_post_operator_int = atoi(match_string_post_operator);
					
					tuple_to_match_position += sizeof(int);
					
					if(!compare_int(operator_, tuple_int, match_string_post_operator_int))
						return false;
					
					break;
				}
				case 'f':
				{
					double tuple_double;
					memcpy(&tuple_double, &tuple_to_match->tuple_content[0] + tuple_to_match_position, sizeof(tuple_double));
					double match_string_post_operator_double = atof(match_string_post_operator);
					
					tuple_to_match_position += sizeof(tuple_double);
					
					if(!compare_double(operator_, tuple_double, match_string_post_operator_double))
						return false;
					
					break;
				}
				case 's':
				{
					const char * tuple_string = &tuple_to_match->tuple_content[0] + tuple_to_match_position;
					size_t tuple_string_length = strlen(tuple_string);
					
					if(!compare_string(operator_, tuple_string, match_string_post_operator))
						return false;
					
					tuple_to_match_position += tuple_string_length + 1;
					
					break;
				}
				default:
				{
					printf(MATCH_STRING_PARAM_NOT_RECOGNIZED);
					break;
				}
			}
		}

		//Czy to koniec napisu? Jeśli wcześniej nie wyszliśmy - zwracamy true
		if(*current_match_string_token_end == 0)
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
	while(tuple_id < linda_memory->tuple_count)
	{
		struct tuple * tuple_to_match = &linda_memory->first_tuple[tuple_id];
		if(tuple_match_match_string(tuple_to_match, match_string))
			return tuple_id;
		
		++tuple_id;
	}

	return -1;
}

bool linda_input(int timeout, const char * match_string, ...)
{
	va_list v_init;
	va_start(v_init, match_string);

	bool ret = vlinda_input(timeout, match_string, &v_init);

	va_end(v_init);
	return ret;
}

bool vlinda_input(int timeout, const char * match_string, va_list * v_init)
{
	int tuple_index = extract_tuple_from_shmem(match_string);

	if(tuple_index == -1)
	{
		//printf("Matched tuple not found\n");
		return false;
	}
	
	printf("Matching tuple found: %d\n", tuple_index);

	//Otrzymaliśmy krotkę z extract_tuple_from_shmem, więc jej dane na pewno zgadzają się z tym, co w va_list
	const struct tuple * found_tuple = linda_memory->first_tuple + tuple_index;
	const size_t info_string_length = strlen(found_tuple->tuple_content);

	size_t info_string_position = 0;
	size_t tuple_position = info_string_length + 1;
	
	va_list va_read;
	va_copy(va_read, *v_init);

	//Memcpy for arguments in va_list
	while(found_tuple->tuple_content[info_string_position] != 0)
	{
		switch(found_tuple->tuple_content[info_string_position])
		{
			case 'i':
			{
				memcpy(va_arg(v_init, int *), &found_tuple->tuple_content[0] + tuple_position, sizeof(int));
				tuple_position += sizeof(int);
				break;
			}
			case 'f':
			{
				memcpy(va_arg(v_init, double *), &found_tuple->tuple_content[0] + tuple_position, sizeof(double));
				tuple_position += sizeof(double);
				break;
			}
			case 's':
			{
				const size_t string_length = strlen(&found_tuple->tuple_content[0] + tuple_position);
				memcpy(va_arg(v_init, char *), &found_tuple->tuple_content[0] + tuple_position, string_length + 1);
				tuple_position += string_length + 1;
				break;
			}
			default:
			{
				printf(INFO_STRING_PARAM_NOT_RECOGNIZED);
				break;
			}
		}
		++info_string_position;
	}
	
	va_end(va_read);
	
	//Usuń krotkę, przesuń pozostałe krotki do tyłu
	memcpy(&linda_memory->first_tuple[tuple_index], &linda_memory->first_tuple[tuple_index + 1], (--linda_memory->tuple_count - tuple_index) * sizeof(struct tuple));
	return true;
}

bool linda_read(int timeout, const char * match_string, ...)
{
	va_list v_init;
	va_start(v_init, match_string);

	bool ret = vlinda_read(timeout, match_string, &v_init);

	va_end(v_init);
	return ret;
}
bool vlinda_read(int timeout, const char * match_string, va_list * v_init)
{
	return vlinda_input(timeout, match_string, v_init);
}