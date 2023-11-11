#include <stdlib.h>
#include <stdio.h>

int hammingDistance(char* x, char* y, int len) { //Must be same length
	int i, distance;
	char xtemp, ytemp;

	for (distance = i = 0; i < len; i++)
		for (xtemp = x[i], ytemp = y[i]; xtemp > 0 || ytemp > 0; xtemp /= 2, ytemp /= 2)
		if (xtemp%2 != ytemp%2)
			distance++;
	return distance;
}

int inOrderFlips(int block_size, int bus_width, FILE* fp) { //If file size is not divisible by block_size, ignore the last part of the file
	//For this procedure, we don't need bits to keep track of the correct ordering
	int i, j, size;
	for (size = 0; fgetc(fp) != EOF; size++)
		;
	char values[size/bus_width][bus_width];
	int flips;
	rewind(fp);

	for (i = 0; i < size/bus_width; i++)
		for (j = 0; j < bus_width; j++)
			values[i][j] = fgetc(fp);

	for (flips = 0, i = 0; i < size/bus_width - 1; i++)
		flips += hammingDistance(values[i], values[i+1], bus_width);

	rewind(fp);

	return flips;
}

int greedy(int block_size, int bus_width, FILE* fp) { //Assuming at most 256 bus widths per block
	//At very start, always grab the first bus width's worth, then go greedy from there
	int i, j, k, size;
	for (size = 0; fgetc(fp) != EOF; size++)
		;
	char values[size/block_size][block_size/bus_width][bus_width + 1]; //need extra to transmit the ordering information
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

	for (flips = 0, i = 0, previous = values[0][0]; i < size/block_size; i++) { //Actually ok to just set previous to the first bus width's worth
		for (j = 0; j < block_size/bus_width; j++)
			used[j] = 0;

		for (j = 0; j < block_size/bus_width; j++) {
			for (k = 0; used[k]; k++)
				;
			for (min = hammingDistance(previous, values[i][k], bus_width+1), mink = k; k < block_size/bus_width; k++)
				if (hammingDistance(previous, values[i][k], bus_width+1) < min && !used[k]) {
					mink = k;
					min = hammingDistance(previous, values[i][k], bus_width+1);
				}
			//printf("mink: %d; min:%d betweeen %c%c%c%c and %c%c%c%c\n", mink, min, previous[0], previous[1], previous[2], previous[3], values[i][mink][0], values[i][mink][1], values[i][mink][2], values[i][mink][3]);
			flips += min;
			previous = values[i][mink];
			used[mink] = 1;
		}
	}

	rewind(fp);

	return flips;
}

int histogram(int block_size, int bus_width, FILE* fp) { //Assuming at most 256 bus widths per block
	int i, j, k, bit, idx, hist, histwidth, size, incrementsize, numones, histmax;
	for (size = 0; fgetc(fp) != EOF; size++)
		;
	char values[size/block_size][block_size/bus_width][bus_width + 1]; //need extra to transmit the ordering information
	int histograms[size/block_size][block_size/bus_width];
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

	for (i = histwidth, histmax = 1; histmax < block_size/bus_width; histmax *= 2, histwidth++) //Calculates i = log_2(block_size/bus_width)
		//Also sets histmax.
		;

	incrementsize = (bus_width*8)/histwidth;
	if ((bus_width*8)%histwidth)
		incrementsize++;
	//printf("increment:%d\n", incrementsize);

	for (i = 0; i < size/block_size; i++)
		for (j = 0; j < block_size/bus_width; j++) {
			for (k = 0, numones = 0, hist = 0, idx = 0; k < bus_width+1; k++)
				for (bit = 1; bit < 256; bit *= 2, idx++) {
					if ((values[i][j][k]/bit)%2)
						numones++;
					if (idx == incrementsize-1) {
						idx = -1;
						hist *= 2;
						if (numones > incrementsize/2)
							hist++;
						numones = 0;
					}
				}
			if (idx > 0)
				if (numones > idx/2)
					hist++;
			//printf("hist:%d\n", hist);
			histograms[i][j] = hist;
		}
					
	for (flips = 0, i = 0, previous = values[0][0]; i < size/block_size; i++) { //Actually ok to just set previous to the first bus width's worth
		//This isn't fully optimized; we should go up, then down, then up, etc.
		//We should also start on a bus_width's worth that has a histogram of 0, not an arbitrary one.
		for (hist = 0; hist < histmax; hist++)
			for (j = 0; j < block_size/bus_width; j++) {
				if (histograms[i][j] == hist) {
					flips += hammingDistance(previous, values[i][j], bus_width+1);
					//printf("Have j=%d: betweeen %c%c%c%c and %c%c%c%c\n", j, previous[0], previous[1], previous[2], previous[3], values[i][j][0], values[i][j][1], values[i][j][2], values[i][j][3]);
					previous = values[i][j];
					//printf("issued:(%d,%d) with hist:%d\n", i, j, hist);
				}
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
	FILE* fp[4];
	char* name[4] = {"Text", "Text With Zeros", "Random", "Random With Zeros"};
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
	
	for (i = 0; i < 4; i++) {
		printf("\nTesting %s\n", name[i]);
		printf("\t# of bitflips with in-order procedure: %d\n", inOrderFlips(block_size, bus_width, fp[i]));
		printf("\t# of bitflips with greedy procedure: %d\n", greedy(block_size, bus_width, fp[i]));
		printf("\t# of bitflips with histogram procedure: %d\n", histogram(block_size, bus_width, fp[i]));
	}
	printf("\n");

	fclose(fp[0]);
	fclose(fp[1]);
	fclose(fp[2]);
	fclose(fp[3]);
	
	return 0;
}
