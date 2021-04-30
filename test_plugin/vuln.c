#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]){


	char name[0xdf];
        char surname[0x10];
        printf("Enter your name: \n");
        fgets(name, 0xff, stdin);
	printf(name);
        printf("Enter your surname: \n");
        fgets(surname, 0xff+0x50, stdin);
        printf("Welcome!\n");

	return 0;

}

