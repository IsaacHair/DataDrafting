#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
	if (argc != 6) {
		printf("Usage: ./GenTrace <original file> <target file> <start block> <end block> <block size>\n");
		return 1;
	}
	
	FILE* source;
	FILE* target;
	int start;
	int end;
	int block_size;

	source = fopen(argv[1], "r");
	target = fopen(argv[2], "w");
	start = atoi(argv[3]);
	end = atoi(argv[4]);
	block_size = atoi(argv[5]);

	int i;

	for (i = 0; i < start*block_size; i++)
		fgetc(source);

	for (; i < end*block_size; i++)
		fprintf(target, "%c", fgetc(source));

	return 0;
}
