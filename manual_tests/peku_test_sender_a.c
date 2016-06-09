#include "../include/linda.h"

#include <string.h>
#include <stdio.h>

int main()
{
	linda_init();
	linda_output("ss", "s1", "s3");
	linda_output("ss", "s1", "s4");
	linda_end();
	
	return 0;
}