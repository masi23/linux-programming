#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <time.h>

#define MAX_LINES 370104

char** words;
int words_cnt;
pthread_mutex_t mutex;

struct alpha {
	char character;
	int count;
};

struct alpha histogram[26] = {
    {'a', 0}, {'b', 0}, {'c', 0}, {'d', 0}, {'e', 0}, {'f', 0}, {'g', 0},
    {'h', 0}, {'i', 0}, {'j', 0}, {'k', 0}, {'l', 0}, {'m', 0}, {'n', 0},
    {'o', 0}, {'p', 0}, {'q', 0}, {'r', 0}, {'s', 0}, {'t', 0}, {'u', 0},
    {'v', 0}, {'w', 0}, {'x', 0}, {'y', 0}, {'z', 0}
};

struct ThreadArgs {
	int start;
	int end;
};

void *update_histogram(void* arg) {
	struct ThreadArgs* args = (struct ThreadArgs*) arg;
	for(int i = args->start; i <= args->end; i++) {
		for(int j = 0; j < strlen(words[i]); j++) {
			char character = words[i][j];
			pthread_mutex_lock(&mutex);
			for(int k = 0; k < 26; k++) {
				if(character == histogram[k].character) {
					histogram[k].count += 1;
					break;
				}
			}
			pthread_mutex_unlock(&mutex);
		}
	}
}

void display_histogram() {
	for(int i = 0; i < 26; i++) {
		if(i % 5 == 0 && i > 0) {
			printf("\n");
		}
		printf("%c : %d \t", histogram[i].character, histogram[i].count);
		
	}
	printf("\n");
}

int main(int argc, char* argv[]) {
	//validate execution
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <w>\n", argv[0]);
		fprintf(stderr, " <w>: A number between 0 and 16.\n");
		exit(1);
	}
	
	int w;
	w = atoi(argv[1]);
	if(w < 0 || w > 16) {
		fprintf(stderr, "<w> is out of range [0, 16]\n");
		exit(1);
	}

	words = malloc(sizeof(char*)*MAX_LINES);
	FILE *f = fopen("words_alpha.txt", "r"); //opening file
	assert(f != NULL);
	char line[255];
	for(words_cnt = 0; words_cnt < MAX_LINES; words_cnt++) {
		if(fgets(line, sizeof(line), f)) {
			words[words_cnt] = strdup(line);
		} else {
			break;
		}
	}
	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);
	if(w == 0) {
		struct ThreadArgs args = {0, words_cnt-1};
		
		update_histogram(&args);
		
	} else {
		pthread_t threads[w];
		struct ThreadArgs args[w];
		for(int i = 0; i < w; i++) {
			args[i].start = (words_cnt / w) * i;
			args[i].end = args[i].start + (words_cnt / w)-1;
			if(words_cnt % w != 0 && i == w-1) 
				args[i].end += (words_cnt % w);
//args[i].end += 1;
			pthread_create(&threads[i], NULL, update_histogram, (void*)&args[i]);
			printf("Thread #%lu started start=%d, end=%d\n", threads[i], args[i].start, args[i].end);
		}
		for(int i = 0; i < w; i++) {
			pthread_join(threads[i], NULL);
			printf("Thread #%lu stopped\n", threads[i]);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end_time);
	double work_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0;
	printf("Execution time: %f s\n", work_time);
	display_histogram();
	
	fclose(f); //closing file
	//printf("lines in file: %d\n", words_cnt);
	for(int i = 0; i < words_cnt; i++)
		free(words[i]);
	free(words);
	return 0;
}
