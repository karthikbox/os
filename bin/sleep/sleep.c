#include<stdio.h>
#include <stdlib.h>

void atoi(char *numberString, int *number, int base);

int main(int argc, char* argv[], char* envp[]) {
	int nsecs;
	/* printf("Sleep binary called\n"); */
	if(argc==1){
		return 0;
	}

	atoi(argv[1], &nsecs, 10);
	sleep(nsecs);
	return 0;
}

void atoi(char *numberString, int *number, int base) {
	int i = 0, sign = 1;
	*number = 0;

	//check if the number is negative or not
	if(numberString[i] == '-')
	{
		sign = -1;
		i++;
	}

	while(numberString[i] != '\0') {

		//if the number is between 0 and a, subtract the ascii value of '0'
		if((numberString[i] >= '0') && (numberString[i] < 'a'))
			*number = (*number)*base + numberString[i] - '0';
		//if the number is between a and f subtract the ascii value of 'a' and add 10
		else if(numberString[i] >='a' && (numberString[i] <='f'))
			*number = (*number)*base + numberString[i] - 'a' + 10;

		i++;
	}

	*number = *number * sign;
}

