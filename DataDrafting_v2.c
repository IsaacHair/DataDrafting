#include <stdlib.h>
#include <stdio.h>

//options for no-compress and compress
#define NOCOMP 0
#define COMP 1

//Using MACROSSS!!!!11!!!!!1!!!!!?????
//Put a 1 for every byte that was omitted b/c it was all zeros
//The way that we compress actually still leaves "gaps," but it makes the spacing more regular
#define COMPRESSION_CODE \
int shift; \
for (i = 0; i < size/block_size; i++) \
	for (j = 0; j < block_size/bus_width; j++) { \
		values[i][j][actual_width-1] = 0; \
		for (shift = 0; shift < bus_width; shift++) \
			if (values[i][j][shift] == 0) { \
				for (k = shift; k < bus_width-1; k++) \
					values[i][j][k] = values[i][j][k+1]; \
				values[i][j][bus_width-1] = 0; \
				values[i][j][actual_width-1] |= (1<<shift); \
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
	FILE* fp[5];
	char* name[5] = {"Text", "Text With Zeros", "Random", "Random With Zeros", "Real Trace"};
	int i;

	block_size = atoi(argv[1]);
	bus_width = atoi(argv[2]);

	if (block_size%bus_width != 0) {
		printf("bus_width must divide block_size\n");
		return 2;
	}

	fp[0] = fopen("SampleText.txt", "r");
	fp[1] = fopen("SampleTextWithZeros.txt", "r");
	fp[2] = fopen("RandomBits", "r");
	fp[3] = fopen("RandomBitsWithZeros", "r");
	fp[4] = fopen("RealTrace", "r");
	
	for (i = 0; i < 5; i++) {
		printf("\nTesting %s\n", name[i]);
		printf("\t# of bitflips with control: %d\n", inOrderFlips(block_size, bus_width, fp[i], NOCOMP));
		printf("\t# of bitflips with greedy: %d\n", greedy(block_size, bus_width, fp[i], NOCOMP));
		printf("\t# of bitflips with compression: %d\n", inOrderFlips(block_size, bus_width, fp[i], COMP));
		printf("\t# of bitflips with compression and greedy: %d\n", greedy(block_size, bus_width, fp[i], COMP));
	}
	printf("\n");

	fclose(fp[0]);
	fclose(fp[1]);
	fclose(fp[2]);
	fclose(fp[3]);
	fclose(fp[4]);
	
	return 0;
}
