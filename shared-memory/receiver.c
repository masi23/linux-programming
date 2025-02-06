#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>

#define MEM_SIZE 101

int main(int argc, char* argv[]) {
	enum {
		DEFAULT = 101,
		RECEIVED,
		CLOSED,
		SENDING_FINISHED
	};



	if(argc != 3) {
		fprintf(stderr, "Usage: %s <key> <filename>\n", argv[0]);
		fprintf(stderr, " <key>: Key received from sender.\n");
		fprintf(stderr, " <filename>: File in which the received data will be stored.\n");
		return 1;
	}
	
	int ipcKey = atoi(argv[1]);
	
	int shmid = shmget(ipcKey, MEM_SIZE, 0666);
	if(shmid == -1) {
		fprintf(stderr, "Error occured while getting shared memory segment\n");
		return 1;
	}
	
	char *memseg = shmat(shmid, NULL, 0);
	if(memseg == (char*)-1) {
		fprintf(stderr, "Error occured while attaching memory\n");
		return 1;
	}
	printf("Shared memory attached\n");
	
	int fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if(fd == -1) {
		fprintf(stderr, "Error occured while opening file %s\n", argv[2]);
		return 1;
	}
	
	printf("Ready to receive\n");
	memseg[0] = RECEIVED;
	
	while(memseg[0] != SENDING_FINISHED) {
		while(memseg[0] == RECEIVED) {
			sleep(1);
		}
		if(memseg[0] < MEM_SIZE && memseg[0] >= 0) {
			ssize_t bytesRead = write(fd, memseg+1, memseg[0]);
			printf("%zd byte(s) received...\n", bytesRead);
			memseg[0] = RECEIVED;
		}
	}
	
	printf("Transfer finished\n");
	memseg[0] = CLOSED;
	//detaching memory
	if(shmdt(memseg) == -1) {
		fprintf(stderr, "Error occured while detaching shared memory segment\n");
		return 5;
	}
	printf("Shared memory detached\n");
	close(fd);
	
	return 0;
}
