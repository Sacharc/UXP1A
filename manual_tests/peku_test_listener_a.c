#include "../include/linda.h"

#include <string.h>
#include <stdio.h>

int main()
{
	
	linda_init();
	
	struct timeval timeout = {10, 0};
	
	char Out1[10], Out2[10];
	while(linda_input(timeout, "s,s==s3", &Out1, &Out2))
	{
		printf("Out1: `%s`, Out2: `%s`\n", Out1, Out2);
	}
	
	linda_end();
	
	return 0;
}