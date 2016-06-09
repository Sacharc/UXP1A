#include "linda.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <sys/shm.h>
#include <assert.h>
#include <errno.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/time.h>

#define FTOK_PATH "/tmp"

bool linda_logging = true;

/*
* Pointer to mem structure
* to assigned to allocated shared memory
*/
struct mem * linda_memory = NULL;

/*
* Id segment of shared memory
*/
int linda_segment_id = 0;

bool linda_init()
{
	//Create virtual memory key
	key_t key = ftok(FTOK_PATH, 1);
	openlog("linda", LOG_PID, 0);
	if(key == (key_t) -1)
	{
		printf("IPC error: ftok: %d", errno);
		if (linda_logging)
		{
			syslog(3, "IPC error: ftok: %d", errno);
		}
		closelog();
		return false;
	}
	
	//Allocate a shared memory segment.
	linda_segment_id = shmget(key, sizeof(struct mem), IPC_CREAT | 0660);
	if(linda_segment_id == -1)
	{
		printf("IPC error: shmget: %d", errno);
		if (linda_logging)
		{
			syslog(3, "IPC error: shmget: %d", errno);
		}
		closelog();
		return false;
	}
	
	//Map shared memory to our VM
	linda_memory = (struct mem*) shmat (linda_segment_id, NULL, 0);
	if(linda_memory == NULL)
	{
		printf("IPC error: linda_memory: %d", errno);
		if (linda_logging)
		{
			syslog(3, "IPC error: linda_memory: %d", errno);
		}
		closelog();
		return false;
	}

	//Check number of currently attached processes. If we are the first process, we are obligated to initialize the memory
	struct shmid_ds shm_data;
	if(shmctl(linda_segment_id, IPC_STAT, &shm_data) == -1)
	{
		printf("IPC error: shmctl(): %d", errno);
		if (linda_logging)
		{
			syslog(3, "IPC error: shmctl(): %d", errno);
		}
		closelog();
		return false;
	}

	//Init tuple_count, if it's first process
	if(shm_data.shm_nattch == 1)
	{
		printf("-> Initializing!\n");
		
		//mem_mutex
		pthread_mutexattr_t mem_mutex_attr;
		pthread_mutexattr_init(&mem_mutex_attr);
		pthread_mutexattr_setpshared(&mem_mutex_attr, PTHREAD_PROCESS_SHARED);
		pthread_mutex_init(&linda_memory->mem_mutex, &mem_mutex_attr);
		pthread_mutexattr_destroy(&mem_mutex_attr);
		
		//output_cond
		pthread_condattr_t output_cond_attr;
		pthread_condattr_init(&output_cond_attr);
		pthread_condattr_setpshared(&output_cond_attr, PTHREAD_PROCESS_SHARED);
		pthread_cond_init(&linda_memory->output_cond, &output_cond_attr);
		pthread_condattr_destroy(&output_cond_attr); 
		
		linda_memory->tuple_count = 0;
	}
	
	return true;
}

void linda_end()
{
	//Destroy members if we are the last instance
	struct shmid_ds shm_data;
	if(shmctl(linda_segment_id, IPC_STAT, &shm_data) == -1)
	{
		printf("IPC error: shmctl(): %d", errno);
		if (linda_logging)
		{
			syslog(3, "IPC error: shmctl(): %d", errno);
		}
	}
	
	
	//Czy usunąć pamięć po zakończeniu dealokacji?
	bool remove_shmem = false;
	
	if(shm_data.shm_nattch == 1)
	{
		printf("-> Cleaning up!\n");
		pthread_cond_destroy(&linda_memory->output_cond);
		pthread_mutex_destroy(&linda_memory->mem_mutex);
		
		remove_shmem = true;
	}
	
	/* Detach the shared memory segment.  */
	if(shmdt(linda_memory) == -1)
	{
		printf("IPC error shmdt(). Cannot detach memory.");
		if (linda_logging)
		{
			syslog(3, "IPC error shmdt(). Cannot detach memory.");
		}
	}

	// Deallocate the shared memory segment.
	if(remove_shmem)
	{
		if(shmctl(linda_segment_id, IPC_RMID, NULL) == -1)
		{
			printf("IPC error shmctl(). Cannot deallocate shared memory.");
			if (linda_logging)
			{
				syslog(3, "IPC error shmctl(). Cannot deallocate shared memory.");
			}
		}
	}
	closelog();
}

/*
* Bytes of input integer copied to char buffer
*/
size_t int_to_tuple(int input, char * output)
{
	memcpy(output, &input, sizeof(input));
	return sizeof(input);
}

/*
* Bytes of input double copied to char buffer
*/
size_t double_to_tuple(double input, char * output)
{
	memcpy(output, &input, sizeof(input));
	return sizeof(input);
}

/*
* Bytes of input char* copied to char buffer
*/
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
	if(pthread_mutex_lock(&linda_memory->mem_mutex) != 0)
	{
		printf("pthread_mutex_lock(): %d", errno);
		return false;
	}
	
	bool inserted = vlinda_output_unsafe(info_string, v_init);
	
	if(pthread_mutex_unlock(&linda_memory->mem_mutex) != 0)
	{
		printf("pthread_mutex_unlock(): %d", errno);
		return false;
	}
	
	if(inserted)
	{
		printf("Calling pthread_cond_broadcast()\n");
		if(pthread_cond_broadcast(&linda_memory->output_cond) != 0)
		{
			printf("pthread_cond_broadcast(): %d", errno);
		}
	}
	
	return inserted;
}
bool vlinda_output_unsafe(const char * info_string, va_list * v_init)
{
	if(linda_memory->tuple_count >= TUPLE_COUNT)
	{
		printf("Tuple memory exhausted (%lu) (%u)", linda_memory->tuple_count, TUPLE_COUNT);
		if (linda_logging)
		{
			syslog(6, "Tuple memory exhausted (%lu) (%u)", linda_memory->tuple_count, TUPLE_COUNT);
		}
		return false;
	}
	
	const size_t info_string_length = strlen(info_string);
	size_t current_tuple_length = info_string_length + 1;
	
	//Copy for first iteration to check and validate info_string 
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


	//Pointer at tuple, incremented
	size_t current_tuple_position = 0;
	struct tuple * current_tuple = linda_memory->first_tuple + linda_memory->tuple_count;
	linda_memory->tuple_count++;

	//Info_string to memory
	memcpy(current_tuple->tuple_content, info_string, info_string_length + 1);
	current_tuple_position += info_string_length + 1;

	//Data from va_list to memory
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
				printf("Unknown character in info_string: `%c` (%d)", info_string[info_string_position], info_string[info_string_position]);
				break;
			}
		}
		++info_string_position;
	}
	
	va_end(va_read);
	if (linda_logging)
	{
		syslog(6, "Saved tuple");
	}
	return true;
}

 /**
 * Compares string using operator.

 * @param operator_ operator
 * @param string_a first string to compare
 * @param string_b second string to compare
 * @return true when condition fulfilled, otherwise returns false
 */
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

 /**
 * Compares int using operator.

 * @param operator_ operator
 * @param a first int to compare
 * @param b second itn to compare
 * @return true when condition fulfilled, otherwise returns false
 */
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

 /**
 * Compares int using operator.

 * @param operator_ operator
 * @param a first double to compare
 * @param b second double to compare
 * @return true when condition fulfilled, otherwise returns false
 */
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
 * Checks if info_string and match_string define the same types in tuple.
 *
 * @param info_string the info string
 * @param match_string the match string
 * @return true when types equal, otherwise false
 */
bool info_string_match_string_equals(const char * info_string, const char * match_string)
{
	size_t info_string_position = 0;
	size_t match_string_position = 0;

	for(;;)
	{
		//Characters are different.
		if(info_string[info_string_position] != match_string[match_string_position])
			return false;

		//If info_string ends (it's basically only makes sense for the first character).
		if(info_string[info_string_position] == 0)
			return true;

		//We are moving info_string iterator.
		info_string_position++;

		//We are moving match_string to first character after null or comma.
		for(;;)
		{
			if(match_string[match_string_position] == ',' || match_string[match_string_position] == 0)
				break;
			match_string_position++;
		}

		if(match_string[match_string_position] == 0)
			return info_string[info_string_position] == 0;

		//Match_string_position points at comma, so we are incrementing match_string_position.
		match_string_position++;
	}
}

/**
* From selected match_string gets operator and puts in operator_output. Output_operator must have at least 3 characters.
*
* @param output_operator the output operator
* @param match_string the match string
* @return pointer at first character after operator
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
* Checks if tuple match to match_string pattern.
*
* @param tuple_to_match the tuple to match
* @param match_string the match string
* @return true if matches
*/
bool tuple_match_match_string(const struct tuple * tuple_to_match, const char * match_string)
{
	//We check if types match.
	if(!info_string_match_string_equals(tuple_to_match->tuple_content, match_string))
		return false;


	size_t tuple_to_match_position = strlen(&tuple_to_match->tuple_content[0]) + 1; //pomijamy info_string

	//Filters
	//current_match_string_token_start points at first filter character
	//current_match_string_token_end points at last filter character

	const char * current_match_string_token_start = match_string;
	for(;;)
	{
		//We gets text from current position to null or comma.
		char current_match_string_token[TUPLE_CONTENT_LENGTH];

		//We are looking for ',' or end of string.
		const char * current_match_string_token_end = strchr(current_match_string_token_start, ',');
		if(current_match_string_token_end == NULL) //Przecinka nie ma, jest koniec tekstu.
			current_match_string_token_end = current_match_string_token_start + strlen(current_match_string_token_start);

		//Length from start to end.
		const size_t current_match_string_token_length = current_match_string_token_end - current_match_string_token_start;

		strncpy(current_match_string_token, current_match_string_token_start, current_match_string_token_length);
		current_match_string_token[current_match_string_token_length] = 0;

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
					printf("Unknown match_string character: `%c` (%d)", current_match_string_token[0], current_match_string_token[0]);
					break;
				}
			}
		}
		else
		{
			char operator_[3];
			const char * match_string_post_operator = match_string_extract_operator(current_match_string_token, operator_);
			
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
					printf("Unknown match_string character: `%c` (%d)", current_match_string_token[0], current_match_string_token[0]);
					break;
				}
			}
		}

		//Is it the end of string?
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

bool vlinda_in_generic(bool to_remove, struct timeval timeout, const char * match_string, va_list * v_init)
{
	if(pthread_mutex_lock(&linda_memory->mem_mutex) != 0)
	{
		printf("pthread_mutex_lock(): %d", errno);
		return false;
	}
	
	bool result = vlinda_in_generic_unsafe(to_remove, timeout, match_string, v_init);
	
	if(pthread_mutex_unlock(&linda_memory->mem_mutex) != 0)
	{
		printf("pthread_mutex_unlock(): %d", errno);
		return false;
	}
	
	return result;
}
bool vlinda_in_generic_unsafe(bool to_remove, struct timeval timeout, const char * match_string, va_list * v_init)
{
	int tuple_index = -1;
	
	//Czas teraz
	struct timeval now;
	gettimeofday(&now, NULL);
	
	//Timestamp zakończenia oczekiwania
	struct timeval timeout_end_timeval; //Po tym timestampie poddajemy się z szukaniem
	timeradd(&now, &timeout, &timeout_end_timeval);
	
	while(true)
	{
		//Sprawdzamy, czy udało się wyjąć krotkę
		tuple_index = extract_tuple_from_shmem(match_string);
		if(tuple_index != -1)
			break;
		
		//Jeśli się nie udało - czekamy. Jeśli wybije timeout - rezygnujemy
		struct timespec timeout_timespec = {timeout_end_timeval.tv_sec, timeout_end_timeval.tv_usec * 1000};
		int wait_result = pthread_cond_timedwait(&linda_memory->output_cond, &linda_memory->mem_mutex, &timeout_timespec);
		if(wait_result != 0)
		{
			if(wait_result == ETIMEDOUT)
			{
				printf("pthread_cond_timedwait timeout\n");
				break;
			}
			
			printf("pthread_cond_timedwait(): %d\n", wait_result);
			return false;
		}
		
		printf("pthread_cond_timedwait() finished\n");
	}

	if (tuple_index == -1)
	{
		//printf("Matched tuple not found\n");
		return false;
	}

	//Tuple is returned by extract_tuple_from_shmem, so its validate with va_list arguments.
	const struct tuple *found_tuple = linda_memory->first_tuple + tuple_index;
	const size_t info_string_length = strlen(found_tuple->tuple_content);

	size_t info_string_position = 0;
	size_t tuple_position = info_string_length + 1;

	va_list va_read;
	va_copy(va_read, *v_init);

	//Memcpy for arguments in va_list
	while (found_tuple->tuple_content[info_string_position] != 0)
	{
		switch (found_tuple->tuple_content[info_string_position])
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
				printf("Unknown character in info_string: `%c` (%d)", found_tuple->tuple_content[info_string_position],
					   found_tuple->tuple_content[info_string_position]);
				break;
			}
		}
		++info_string_position;
	}

	va_end(va_read);

	if (to_remove)
	{
		//Delete tuple by replacing it and decrementing tuple_count.
		memcpy(&linda_memory->first_tuple[tuple_index], &linda_memory->first_tuple[tuple_index + 1],
			   (--linda_memory->tuple_count - tuple_index) * sizeof(struct tuple));
		if (linda_logging)
		{
			syslog(6, "Removed tuple");
		}
	}
	return true;
}


bool linda_input(struct timeval timeout, const char * match_string, ...)
{
	va_list v_init;
	va_start(v_init, match_string);

	bool ret = vlinda_input(timeout, match_string, &v_init);

	va_end(v_init);
	return ret;
}

bool vlinda_input(struct timeval timeout, const char * match_string, va_list * v_init)
{
	return vlinda_in_generic(true, timeout, match_string, v_init);
}

bool linda_read(struct timeval timeout, const char * match_string, ...)
{
	va_list v_init;
	va_start(v_init, match_string);

	bool ret = vlinda_read(timeout, match_string, &v_init);

	va_end(v_init);
	return ret;
}

bool vlinda_read(struct timeval timeout, const char * match_string, va_list * v_init)
{
	return vlinda_in_generic(false, timeout, match_string, v_init);
}