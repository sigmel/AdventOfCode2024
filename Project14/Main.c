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

	const int simulateTime = 100;
	const int tileColumns = 101;
	const int tileRows = 103;
	const int quadrantWidth = tileColumns / 2;
	const int quadrantHeight = tileRows / 2;

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

		robotVector.data[robotIndex] = x;
		robotVector.data[robotIndex + 1] = y;

		// compute the quadrant
		// (could just count up everything that doesn't line on the boundary, but I want to verify my solutions are matching the test examples)
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
	printf("%d\n", total);
}