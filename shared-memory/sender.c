#include <stdio.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>

#define MEM_SIZE 101

int main(int argc, char* argv[]) {

	enum {
		DEFAULT = 101,
		RECEIVED,
		CLOSED,
		SENDING_FINISHED
	};

	if(argc != 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		fprintf(stderr, " <filename>: File to transfer.\n");
		return(1);
	}
	
	key_t ipcKey = ftok(argv[1], 'a');
	
	//creating key from file
	if(ipcKey < 0) {
		fprintf(stderr, "Error occured while creating key.");
		return 1;
	}
	printf("Key: %d\n", ipcKey);
	
	//getting memory block
	int shmid = shmget(ipcKey, MEM_SIZE, 0666 | IPC_CREAT | IPC_EXCL);
	if(shmid < 0) {
		fprintf(stderr, "Error occured while getting shared memory\n");
		return 1;
	}
	
	//attaching memory
	char *memseg = shmat(shmid, NULL, 0);
	if(memseg == (char*)-1) {
		fprintf(stderr, "Error occured while attaching shared memory segment\n");
		return 4;
	}
	printf("Shared memory attached\n");
	
	int fd = open(argv[1], O_RDONLY);
	if(fd == -1) {
		fprintf(stderr, "Error occured while opening file %s\n", argv[1]);
		return 1;
	}
	memseg[0] = DEFAULT;
	//waiting for receiver connection (60s)
	printf("Waiting for receiver\n");
	int timePassed = 0;
	while(memseg[0] != RECEIVED) {
		sleep(1);
		timePassed += 1;
		if(timePassed > 60) {
			fprintf(stderr, "Connection time elapsed\n");
			return 1;	
		}
	}
	
	printf("Receiver connected\n");
	ssize_t bytesSent;
	while((bytesSent = read(fd, memseg+1, MEM_SIZE-1)) > 0) {
		printf("%zd byte(s) sent...\n", bytesSent);
		memseg[0] = bytesSent;
		while(memseg[0] != RECEIVED) {
			sleep(1);
		}
	} 
	memseg[0] = SENDING_FINISHED;
	printf("Transfer finished\n");
	
	while(memseg[0] != CLOSED) {
		sleep(1);
	}
	printf("Receiver closed\n");
	
	//detaching memory
	if(shmdt(memseg) == -1) {
		fprintf(stderr, "Error occured while detaching shared memory segment\n");
		return 1;
	}
	printf("Shared memory detached\n");
	
	//releasing memory
	if(shmctl(shmid, IPC_RMID, NULL) == -1) {
		fprintf(stderr, "Error occured while removing sharem memory\n");
		return 1;
	}
	printf("Memory released\n");
	close(fd);
	
	return 0;
}
