#include <stdlib.h>
#include <stdio.h>

//this is largest prime less than 1 billion
#define MAX_PRIME 999999937

int prime_greater(int val) {
	//We assume val >= 2

	int i, j;

	for (i = val; i < MAX_PRIME; i++) {
		for (j = 2; j*j < i; j++)
			if ((i/j)*j == i)
				break;
		if (j*j > i)
			return i;
	}

	return MAX_PRIME;
}

int main(int argc, char **argv) {
	if (argc != 6) {
		printf("Usage: ./GenTrace <original file> <buffer file> <target file> <hash table size> <block size>\n");
		return 1;
	}
	
	int block_size; //Number of bytes per cache block
	FILE* source;
	FILE* buffer;
	FILE* target;
	int hash_size;
	int print;

	source = fopen(argv[1], "r");
	buffer = fopen(argv[2], "w+");
	target = fopen(argv[3], "w");
	hash_size = prime_greater(atoi(argv[4]));
	block_size = atoi(argv[5]);

	printf("hash_size:%d\nblock_size:%d\n", hash_size, block_size);

	//We will purposefully leave the hash table unitialized to simulate what unitialized memory looks like
	int hash_table[hash_size][block_size];

	unsigned long long addr;
	unsigned long long data;
	char* word;
	size_t len;
	int i;
	char c;
	int bytes;
	int foo, bar;

	//First, copy source into target, delimiting all words by spaces
	while ((c = fgetc(source)) != EOF)
		if (c == ' ')
			fprintf(buffer, "\n");
		else
			fprintf(buffer, "%c", c);
	rewind(buffer);

	//We now populate the hash table by reading the target file
	//We read out the blocks as we go
	while (getline(&word, &len, buffer) != -1) {
		if (!(rand()%10000))
			print = 1;
		else
			print = 0;
		if (word[0] == 'W') {
			bytes = word[5]-'0';
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			addr = strtoull(word, NULL, 0);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			data = strtoull(word+3, NULL, 16);
			if (print)
				printf("write addr:%llx data:%llx bytes:%d", addr, data, bytes);
			for (i = 0; i < bytes && i < 8; i++) {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%256;
				addr++;
				data = data/256;
			}
			if (print)
				printf("\n\n");
		}
		else if (word[0] == 'R') {
			//Treat all reads as if they were cache misses, and we load the entire associated block
			bytes = word[4]-'0';
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			addr = strtoull(word, NULL, 0);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			data = strtoull(word+3, NULL, 16);
			if (print)
				printf("read addr:%llx data:%llx bytes:%d cache block:\n", addr, data, bytes);
			for (i = 0; i < bytes && i < 8; i++) {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%256;
				addr++;
				data = data/256;
			}
			for (i = 0; i < block_size; i++) {
				if (print) {
					printf("%c%c ", ((foo = hash_table[(addr/block_size)%hash_size][i]%16) > 9 ? foo-10+'a' : foo+'0'), ((bar = hash_table[(addr/block_size)%hash_size][i]/16) > 9 ? bar-10+'a' : bar+'0'));
					if (i%16 == 7)
						printf(" ");
					if (i%16 == 15)
						printf("\n");
				}
				fprintf(target, "%c", hash_table[(addr/block_size)%hash_size][i]);
			}
			if (print)
				printf("\n");
		}
	}

	return 0;
}
