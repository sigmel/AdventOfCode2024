#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VECTOR_DATA long long
struct Vector
{
	int count;
	int size;
	VECTOR_DATA* data;
};

void initVectorWithSize(struct Vector* vector, int initialSize)
{
	vector->count = 0;
	vector->size = initialSize;
	vector->data = malloc(sizeof(VECTOR_DATA) * vector->size);
}

void initVector(struct Vector* vector)
{
	const int VECTOR_INIT_SIZE = 10;
	initVectorWithSize(vector, VECTOR_INIT_SIZE);
}

void copyVector(struct Vector* dst, struct Vector* src)
{
	dst->data = malloc(sizeof(VECTOR_DATA) * src->size);
	if (dst->data)
	{
		memcpy(dst->data, src->data, sizeof(VECTOR_DATA) * src->size);
		dst->count = src->count;
		dst->size = src->size;
	}
}

void freeVector(struct Vector* vector)
{
	vector->count = 0;
	vector->size = 0;
	free(vector->data);
}

void addVector(struct Vector* vector, VECTOR_DATA value)
{
	if (vector->count == vector->size)
	{
		vector->size *= 2;
		VECTOR_DATA* newData = malloc(sizeof(VECTOR_DATA) * vector->size);
		if (newData)
		{
			memcpy(newData, vector->data, sizeof(VECTOR_DATA) * vector->count);
			free(vector->data);
			vector->data = newData;
		}
	}

	vector->data[vector->count] = value;
	vector->count++;
}

void removeElementAtVector(struct Vector* vector, int index)
{
	assert(index < vector->count);
	for (int vectorIndex = index; vectorIndex < vector->count - 1; vectorIndex++)
	{
		vector->data[vectorIndex] = vector->data[vectorIndex + 1];
	}
	vector->count--;
}

int containsValueInVector(struct Vector* vector, VECTOR_DATA value)
{
	int number = 0;
	for (int vectorIndex = 0; vectorIndex < vector->count; vectorIndex++)
	{
		if (vector->data[vectorIndex] == value)
		{
			number++;
		}
	}
	return number;
}

void clearVector(struct Vector* vector)
{
	vector->count = 0;
	memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

void main()
{
	// will be x,y vX,vY
	struct Vector robotVector;
	initVector(&robotVector);

	FILE* filePointer = NULL;
	filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 256
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int advIndex = 0;
		while (!isdigit(input[advIndex])) { advIndex++; }
		addVector(&robotVector, atoll(&input[advIndex])); // x
		while (isdigit(input[advIndex])) { advIndex++; }
		while (!isdigit(input[advIndex])) { advIndex++; }
		addVector(&robotVector, atoll(&input[advIndex])); // y
		while (isdigit(input[advIndex])) { advIndex++; }
		while (!isdigit(input[advIndex]) && input[advIndex] != '-') { advIndex++; }
		addVector(&robotVector, atoll(&input[advIndex])); // vX
		while (isdigit(input[advIndex]) || input[advIndex] == '-') { advIndex++; }
		while (!isdigit(input[advIndex]) && input[advIndex] != '-') { advIndex++; }
		addVector(&robotVector, atoll(&input[advIndex])); // vY
	}
	fclose(filePointer);

	const int tileColumns = 101;
	const int tileRows = 103;
	const int quadrantWidth = tileColumns / 2;
	const int quadrantHeight = tileRows / 2;

	// didn't quite understand what this one was asking for, so looking on reddit helped guide this solution
	// basically, we want to find when a large number are in a single quadrant
	// so we can compute all the possibilities and then print out the top couple to find it
	// the "possibility number" is basically the value we calculated in part 1
	// might not be the lowest, so we'll just print them out in increasing order (shouldn't be many, so we'll take a slow approach)

	struct Vector totals;
	initVectorWithSize(&totals, tileColumns * tileRows);
	for (int simulationIndex = 0; simulationIndex < tileColumns * tileRows; simulationIndex++)
	{
		int quadrantCount[4];
		memset(quadrantCount, 0, sizeof(quadrantCount));

		for (int robotIndex = 0; robotIndex < robotVector.count; robotIndex += 4)
		{
			long long x = robotVector.data[robotIndex];
			long long y = robotVector.data[robotIndex + 1];
			long long vX = robotVector.data[robotIndex + 2];
			long long vY = robotVector.data[robotIndex + 3];

			x += vX * simulationIndex;
			x %= tileColumns;
			if (x < 0) { x += tileColumns; }
			y += vY * simulationIndex;
			y %= tileRows;
			if (y < 0) { y += tileRows; }

			// compute the quadrant
			if (x < quadrantWidth && y < quadrantHeight)
			{
				quadrantCount[0]++;
			}
			else if (x < quadrantWidth && y > quadrantHeight)
			{
				quadrantCount[1]++;
			}
			else if (x > quadrantWidth && y < quadrantHeight)
			{
				quadrantCount[2]++;
			}
			else if (x > quadrantWidth && y > quadrantHeight)
			{
				quadrantCount[3]++;
			}
		}

		int total = quadrantCount[0] * quadrantCount[1] * quadrantCount[2] * quadrantCount[3];
		addVector(&totals, total);
	}

	char* grid = malloc(sizeof(char) * tileColumns * tileRows);

	struct Vector triedValues;
	initVector(&triedValues);

	for (int attemptIndex = 0; attemptIndex < tileColumns * tileRows; attemptIndex++)
	{
		// find the lowest value (that hasn't been tried before)
		long long simulateTime = 0;
		long long lowestTotal = totals.data[0];
		for (int searchIndex = 1; searchIndex < totals.count; searchIndex++)
		{
			if (totals.data[searchIndex] < lowestTotal)
			{
				if (!containsValueInVector(&triedValues, totals.data[searchIndex]))
				{
					simulateTime = searchIndex;
					lowestTotal = totals.data[searchIndex];
				}
			}
		}

		// simulate to that step and record it
		memset(grid, '.', sizeof(char) * tileColumns * tileRows);

		int quadrantCount[4];
		memset(quadrantCount, 0, sizeof(quadrantCount));
		for (int robotIndex = 0; robotIndex < robotVector.count; robotIndex += 4)
		{
			long long x = robotVector.data[robotIndex];
			long long y = robotVector.data[robotIndex + 1];
			long long vX = robotVector.data[robotIndex + 2];
			long long vY = robotVector.data[robotIndex + 3];

			x += vX * simulateTime;
			x %= tileColumns;
			if (x < 0) { x += tileColumns; }
			y += vY * simulateTime;
			y %= tileRows;
			if (y < 0) { y += tileRows; }

			grid[y * tileColumns + x] = 'X';

			// compute the quadrant
			if (x < quadrantWidth && y < quadrantHeight)
			{
				quadrantCount[0]++;
			}
			else if (x < quadrantWidth && y > quadrantHeight)
			{
				quadrantCount[1]++;
			}
			else if (x > quadrantWidth && y < quadrantHeight)
			{
				quadrantCount[2]++;
			}
			else if (x > quadrantWidth && y > quadrantHeight)
			{
				quadrantCount[3]++;
			}
		}
		int total = quadrantCount[0] * quadrantCount[1] * quadrantCount[2] * quadrantCount[3];
		assert(total == lowestTotal);

		printf("%lld %lld\n", simulateTime, lowestTotal);
		for (int y = 0; y < tileRows; y++)
		{
			for (int x = 0; x < tileColumns; x++)
			{
				printf("%c", grid[y * tileColumns + x]);
			}
			printf("\n");
		}

		addVector(&triedValues, lowestTotal);
	}
}