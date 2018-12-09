#include "stdio.h"
#include <stdlib.h>

int
main(void)
{
	printf("This is Cgi!\n");	
	printf("Hello World!\n");
	char *envtest = getenv("QUERY_STRING");
	if (envtest == NULL)
		printf("%s\n", "Get env failed!");
	else
		printf("QUERY_STRING: %s\n", envtest);
	return 0;
}