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

	source = fopen(argv[1], "r");
	buffer = fopen(argv[2], "w+");
	target = fopen(argv[3], "w");
	block_size = atoi(argv[4]);
	hash_size = prime_greater(atoi(argv[5]));

	//We will purposefully leave the hash table unitialized to simulate what unitialized memory looks like
	char hash_table[hash_size][block_size];

	unsigned long long addr;
	unsigned long long data;
	char* word;
	size_t len;
	int i;
	char c;

	//First, copy source into target, delimiting all words by spaces
	while ((c = fgetc(source)) != EOF)
		if (c == ' ')
			fprintf(buffer, "\n");
		else
			fprintf(buffer, "%c", c);
	rewind(buffer);

	//We now populate the hash table by reading the target file
	//We read out the blocks as we go
	while (getline(&word, &len, buffer) != -1)
		if (word[0] == 'W') {
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			addr = strtoull(word, NULL, 0);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			data = strtoull(word+3, NULL, 16);
			if (word[5] == '1') {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%(1ULL<<8);
			}
			else if (word[5] == '2') {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%(1ULL<<8);
				hash_table[((addr+1)/block_size)%hash_size][(addr+1)%block_size] = (data%(1ULL<<16))/(1ULL<<8);
			}
			else if (word[5] == '4') {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%(1ULL<<8);
				hash_table[((addr+1)/block_size)%hash_size][(addr+1)%block_size] = (data%(1ULL<<16))/(1ULL<<8);
				hash_table[((addr+2)/block_size)%hash_size][(addr+2)%block_size] = (data%(1ULL<<24))/(1ULL<<16);
				hash_table[((addr+3)/block_size)%hash_size][(addr+3)%block_size] = (data%(1ULL<<32))/(1ULL<<24);
			}
			else {
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data%(1ULL<<8);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<16))/(1ULL<<8);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<24))/(1ULL<<16);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<32))/(1ULL<<24);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<40))/(1ULL<<32);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<48))/(1ULL<<40);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = (data%(1ULL<<56))/(1ULL<<48);
				addr++;
				hash_table[(addr/block_size)%hash_size][addr%block_size] = data/(1ULL<<56);
			}
		}
		else if (word[0] == 'R') {
			//Treat all reads as if they were cache misses, and we load the entire associated block
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
			for (i = 0; i < block_size-(addr%block_size); i++)
				fprintf(target, "%c", hash_table[(addr/block_size)%hash_size][i]);
			if (word[4] == '1') {
				fprintf(target, "%c", data%(1ULL<<8));
				i++;
			}
			else if (word[4] == '2') {
				fprintf(target, "%c%c", data%(1ULL<<8), (data%(1ULL<<16))/(1ULL<<8));
				i += 2;
			}
			else if (word[4] == '4') {
				fprintf(target, "%c%c", data%(1ULL<<8), (data%(1ULL<<16))/(1ULL<<8));
				fprintf(target, "%c%c", (data%(1ULL<<24))/(1ULL<<16), (data%(1ULL<<32))/(1ULL<<24));
				i += 4;
			}
			else {
				fprintf(target, "%c%c", data%(1ULL<<8), (data%(1ULL<<16))/(1ULL<<8));
				fprintf(target, "%c%c", (data%(1ULL<<24))/(1ULL<<16), (data%(1ULL<<32))/(1ULL<<24));
				fprintf(target, "%c%c", (data%(1ULL<<40))/(1ULL<<32), (data%(1ULL<<48))/(1ULL<<40));
				fprintf(target, "%c%c", (data%(1ULL<<56))/(1ULL<<48), data/(1ULL<<56));
				i += 8;
			}
			for (; i < block_size; i++)
				fprintf(target, "%c", hash_table[(addr/block_size)%hash_size][i]);
		}

	return 0;
}
