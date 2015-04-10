#include <sys/sbunix.h>



int main(int argc, char* argv[], char* envp[]) {
	
	char ch='X';

	printf("Enter a character ");
	
	printf("Entered character is %c\n", ch);

	while(1){
		ch++;
		printf("Entered character is %c\n", ch);
	}
	return 0;
}

