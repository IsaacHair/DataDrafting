#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	//Don't even need to care about block size b/c direct concatenation
	if (argc != 4) {
		printf("Usage: ./GenTrace <original file> <buffer file> <target file>\n");
		return 1;
	}
	
	FILE* source;
	FILE* buffer;
	FILE* target;

	source = fopen(argv[1], "r");
	buffer = fopen(argv[2], "w+");
	target = fopen(argv[3], "w");

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
		if (word[0] == 'R') {
			//Treat all reads as if they were cache misses, and we load the entire associated block
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			getline(&word, &len, buffer);
			data = strtoull(word+3, NULL, 16);
			fprintf(target, "%c", data%256);
			if (word[4] == '1')
				continue;
			data = data/256;
			fprintf(target, "%c", data%256);
			if (word[4] == '2')
				continue;
			data = data/256;
			fprintf(target, "%c", data%256);
			data = data/256;
			fprintf(target, "%c", data%256);
			if (word[4] == '4')
				continue;
			data = data/256;
			fprintf(target, "%c", data%256);
			data = data/256;
			fprintf(target, "%c", data%256);
			data = data/256;
			fprintf(target, "%c", data%256);
			data = data/256;
			fprintf(target, "%c", data%256);
		}

	return 0;
}
