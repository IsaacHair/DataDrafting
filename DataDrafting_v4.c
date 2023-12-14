#include <stdlib.h>
#include <stdio.h>

//options for no-compress and compress
#define NOCOMP 0
#define COMP 1


//Using MACROSSS!!!!11!!!!!1!!!!!?????
#define COMPRESSION_CODE \
{int shift, iters; \
char temp[size/block_size][block_size]; \
for (i = 0; i < size/block_size; i++) \
	for (j = 0; j < block_size/bus_width; j++) { \
		values[i][j][actual_width-1] = 0; \
		for (k = 0; k < bus_width; k++) \
			temp[i][j*bus_width + k] = values[i][j][k]; \
	} \
for (i = 0; i < size/block_size; i++) \
	for (shift = iters = 0; iters < block_size; iters++) \
		if (temp[i][shift] == 0) { \
			for (k = shift; k < block_size-1; k++) \
				temp[i][k] = temp[i][k+1]; \
			temp[i][block_size-1] = 0; \
			values[i][k/bus_width][actual_width-1] |= (1<<(k%bus_width)); \
		} \
		else \
			shift++; \
for (i = 0; i < size/block_size; i++/*, printf("\n")*/) \
	for (j = 0; j < block_size/bus_width; j++) { \
		for (k = 0; k < bus_width; k++) \
			values[i][j][k] = temp[i][j*bus_width + k]; \
/*		printf("comp:%d %d %d %d %d %d %d %d\n", values[i][j][0], values[i][j][1], values[i][j][2], values[i][j][3], values[i][j][4], values[i][j][5], values[i][j][6], values[i][j][7]); */\
	} \
}

int hammingDistance(char* x, char* y, int len) { //Must be same length
	int i, distance;
	char xtemp, ytemp;

	for (distance = i = 0; i < len; i++)
		for (xtemp = x[i], ytemp = y[i]; xtemp > 0 || ytemp > 0; xtemp /= 2, ytemp /= 2)
		if (xtemp%2 != ytemp%2)
			distance++;
	return distance;
}

int inOrderFlips(int block_size, int bus_width, FILE* fp, int comp) {
	//If file size is not divisible by block_size, ignore the last part of the file
	//For this procedure, we don't need bits to keep track of the correct ordering
	//Assume bus_width is at most 8 bytes
	//Ignore last block of data if it doesen't occupy a whole block

	int i, j, k, size;
	for (size = 0; fgetc(fp) != EOF; size++)
		;
	
	//Deciding if there should be an extra word to indicate which bytes were compressed out
	//If there are unnecessary bits, don't care b/c will always be zero anyways
	int actual_width;
	if (comp)
		actual_width = bus_width+1;
	else
		actual_width = bus_width;
	
	char values[size/block_size][block_size/bus_width][actual_width];
	char* previous;

	int flips;
	rewind(fp);

	for (i = 0; i < size/block_size; i++)
		for (j = 0; j < block_size/bus_width; j++)
			for (k = 0; k < bus_width; k++)
				values[i][j][k] = fgetc(fp);

	if (comp) {
		COMPRESSION_CODE
	}

	for (flips = 0, i = 0, previous = values[0][0]; i < size/block_size; i++)
		for (j = 0; j < block_size/bus_width; j++) {
			flips += hammingDistance(previous, values[i][j], actual_width);
			previous = values[i][j];
		}

	rewind(fp);

	return flips;
}

int greedy(int block_size, int bus_width, FILE* fp, int comp) {
	//Assuming at most 256 bus widths per block
	//Assuming bus_width is at most 8 bytes
	//At very start, always grab the first bus width's worth, then go greedy from there
	//Ignore last block of data if it doesen't occupy a whole block

	int i, j, k, size;
	for (size = 0; fgetc(fp) != EOF; size++)
		;

	//Deciding if there should be an extra byte for ordering info AND for compression info
	//If there are unnecessary bits, don't care b/c will always be zero anyways
	int actual_width;
	if (comp)
		actual_width = bus_width+2;
	else
		actual_width = bus_width+1;
	
	char values[size/block_size][block_size/bus_width][actual_width];

	int used[block_size/bus_width];
	char* previous;
	int flips, min, mink;
	rewind(fp);

	for (i = 0; i < size/block_size; i++)
		for (j = 0; j < block_size/bus_width; j++) {
			for (k = 0; k < bus_width; k++)
				values[i][j][k] = fgetc(fp);
			values[i][j][k] = j;
		}
	
	if (comp) {
		COMPRESSION_CODE
	}

	for (flips = 0, i = 0, previous = values[0][0]; i < size/block_size; i++) {
		for (j = 0; j < block_size/bus_width; j++)
			used[j] = 0;

		for (j = 0; j < block_size/bus_width; j++) {
			for (k = 0; used[k]; k++)
				;
			for (min = hammingDistance(previous, values[i][k], actual_width), mink = k; k < block_size/bus_width; k++)
				if (hammingDistance(previous, values[i][k], actual_width) < min && !used[k]) {
					mink = k;
					min = hammingDistance(previous, values[i][k], actual_width);
				}
			flips += min;
			previous = values[i][mink];
			used[mink] = 1;
		}
	}

	rewind(fp);

	return flips;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		printf("Usage: ./DataDrafting <cache block size in bytes> <transmission bus width in bytes>\n");
		return 1;
	}
	
	int block_size; //Number of bytes per cache block
	int bus_width; //Width of bus in number of bytes
	FILE* fp[10];
	char* name[10] = {"Text", "Text With Zeros", "Random", "Random With Zeros", "mkfs32", "mkfs128", "mkfs256", "firefox32", "firefox128", "firefox256"};
	int i;

	block_size = atoi(argv[1]);
	bus_width = atoi(argv[2]);

	if (block_size%bus_width != 0) {
		printf("bus_width must divide block_size\n");
		return 2;
	}
	if (block_size != 64) {
		printf("Only 64 byte blocks allowed.\n");
		return 3;
	}

	fp[0] = fopen("SampleText.txt", "r");
	fp[1] = fopen("SampleTextWithZeros.txt", "r");
	fp[2] = fopen("RandomBits", "r");
	fp[3] = fopen("RandomBitsWithZeros", "r");
	fp[4] = fopen("RealTrace_mkfs32", "r");
	fp[5] = fopen("RealTrace_mkfs128", "r");
	fp[6] = fopen("RealTrace_mkfs256", "r");
	fp[7] = fopen("RealTrace_firefox32", "r");
	fp[8] = fopen("RealTrace_firefox128", "r");
	fp[9] = fopen("RealTrace_firefox256", "r");
	
	for (i = 0; i < 10; i++) {
		printf("\nTesting %s\n", name[i]);
		printf("\t# of bitflips with control: %d\n", inOrderFlips(block_size, bus_width, fp[i], NOCOMP));
		printf("\t# of bitflips with greedy: %d\n", greedy(block_size, bus_width, fp[i], NOCOMP));
		printf("\t# of bitflips with compression: %d\n", inOrderFlips(block_size, bus_width, fp[i], COMP));
		printf("\t# of bitflips with compression and greedy: %d\n", greedy(block_size, bus_width, fp[i], COMP));
	}
	printf("\n");

	for (i = 0; i < 10; i++)
		fclose(fp[i]);
	
	return 0;
}
