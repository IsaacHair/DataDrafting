#include <stdlib.h>
#include <stdio.h>

int main() {
	FILE *fp;
	FILE* fp2;
	char c;

	fp = fopen("RandomBits", "w");
	srand(2525);
	for (int i = 0; i < 1000; i++)
		fprintf(fp, "%c", rand()%256);
	fclose(fp);

	fp = fopen("RandomBitsWithZeros", "w");
	srand(5252);
	for (int i = 0; i < 1000; i++)
		if (rand()%8)
			fprintf(fp, "%c", rand()%256);
		else {
			int max = rand()%8;
			for (int j = 0; j < max; j++)
				fprintf(fp, "%c", '\0');
		}
	fclose(fp);

	fp = fopen("SampleTextWithZeros.txt", "w");
	fp2 = fopen("SampleText.txt", "r");
	srand(5555);
	//Purposely deletes characters to some degree
	while ((c = fgetc(fp2)) != EOF)
		if (rand()%8)
			fputc(c, fp);
		else {
			int max = rand()%8;
			for (int j = 0; j < max; j++)
				fprintf(fp, "%c", '\0');
		}
	
	return 0;
}
