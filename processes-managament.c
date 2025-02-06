#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char *argv[]) {
	//printf("argc: %d\n",argc);
	if(argc == 1) {
		fprintf(stderr,"Brak argumentów w wywołaniu\n");
		exit(201);
	}
	int argsLength = argc-1;
	int args[argsLength];
	
	for(int i = 0; i < argsLength; i++) {
		char *endptr;
		errno = 0;
		long val = strtol(argv[i+1], &endptr, 10); 
		if(*endptr != '\0') {
			fprintf(stderr,"Co najmniej jeden z argumentów nie jest liczba calkowita.\n");
			exit(202);
		} else if(val > 100 || val < 0) {
			fprintf(stderr, "Argument (%ld) wykroczyl poza zakres [0,100]\n", val);
			exit(202);
		}
		args[i] = val;
	}
	
	switch(argsLength) {
		case 1:
			exit(args[0]);
		break;
		case 2:
			if(args[0] > args[1]) {
				exit(args[0]);
			} else {
				exit(args[1]);
			}
		break;
		default:
			bool even = false;
			int size1, size2;
			if(argsLength % 2 == 0) {
				even = true;
				size1 = argsLength / 2;
				size2 = size1;
			} else {
				size1 = argsLength / 2;
				size2 = size1 + 1;
			}
			char* list1[size1+2];
			char* list2[size2+2];
			list1[0] = argv[0];
			list2[0] = argv[0];
			if(even) {
				for(int i = 1; i <= size1; i++) {
					list1[i] = argv[i];
					list2[i] = argv[size1+i];
				}
				list1[size1+1] = NULL;
				list2[size2+1] = NULL;
			} else {
				for(int i = 1; i <= size1; i++) {
					list1[i] = argv[i];
				}
				list1[size1+1] = NULL;
				for(int i = 1; i <= size2; i++) {
					list2[i] = argv[size1+i];
				}
				list2[size2+1] = NULL;
			}
			pid_t pid1 = fork();
			
			if(pid1 == 0) {
				execv(argv[0], list1);
			}
			
			
			pid_t pid2 = fork();
			if(pid2 == 0) {
				execv(argv[0], list2);
			} 
			
			if(pid1 > 0 && pid2 > 0) {
				pid_t finished1, finished2;
				int wstatus1, wstatus2;
				
				finished1 = waitpid(pid1, &wstatus1, 0);
				finished2 = waitpid(pid2, &wstatus2, 0);
				
				printf("%d  %d  %d  | ", getpid(), pid1, WEXITSTATUS(wstatus1));
				for(int i = 1; list1[i] != NULL; i++) {
					printf("%s ", list1[i]);
				}
				printf("\n");
				printf("%d  %d  %d  | ", getpid(), pid2, WEXITSTATUS(wstatus2));
				for(int i = 1; list2[i] != NULL; i++) {
					printf("%s ", list2[i]);
				}
				printf("\n");
				int biggerExit;
				if(WEXITSTATUS(wstatus1) > WEXITSTATUS(wstatus2)) {
					
					biggerExit = WEXITSTATUS(wstatus1);
				} else {
					biggerExit = WEXITSTATUS(wstatus2);
				}
				printf("%d\t\t%d\n\n", getpid(), biggerExit);
				exit(biggerExit);
			}
		break;
	}
}
