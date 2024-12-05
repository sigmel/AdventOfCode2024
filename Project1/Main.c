#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

struct Vector
{
	int count;
	int size;
	int* data;
};

void initVector(struct Vector* vector)
{
	const int VECTOR_INIT_SIZE = 10;
	vector->count = 0;
	vector->size = VECTOR_INIT_SIZE;
	vector->data = malloc(sizeof(int) * vector->size);
}

void addVector(struct Vector* vector, int value)
{
	if (vector->count == vector->size)
	{
		vector->size *= 2;
		int* newData = malloc(sizeof(int) * vector->size);
		if (newData)
		{
			memcpy(newData, vector->data, sizeof(int) * vector->count);
			free(vector->data);
			vector->data = newData;
		}
	}

	vector->data[vector->count] = value;
	vector->count++;
}

int compare(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}

void main()
{
	struct Vector leftVector;
	struct Vector rightVector;

	initVector(&leftVector);
	initVector(&rightVector);

	FILE* filePointer = NULL;
	filePointer = fopen("puzzle.txt", "r");

	#define INPUT_SIZE 100
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int value = atoi(input);
		addVector(&leftVector, value);

		int advIndex = 0;
		while (isdigit(input[advIndex])) { advIndex++; }
		while (isspace(input[advIndex])) { advIndex++; }

		value = atoi(&input[advIndex]);
		addVector(&rightVector, value);
	}

	fclose(filePointer);

	qsort(leftVector.data, leftVector.count, sizeof(int), &compare);
	qsort(rightVector.data, rightVector.count, sizeof(int), &compare);

	assert(leftVector.count == rightVector.count);

	int sum = 0;
	for (int vectorIndex = 0; vectorIndex < leftVector.count; vectorIndex++)
	{
		int difference = leftVector.data[vectorIndex] - rightVector.data[vectorIndex];
		sum += abs(difference);
	}

	printf("%d\n", sum);

	int sum2 = 0;
	for (int vectorIndex = 0; vectorIndex < leftVector.count; vectorIndex++)
	{
		int comparison = leftVector.data[vectorIndex];

		// Just do a lazy O(n2) search since there aren't that many elements ultimately
		for (int searchIndex = 0; searchIndex < rightVector.count; searchIndex++)
		{
			if (comparison == rightVector.data[searchIndex])
			{
				sum2 += comparison;
			}
		}
	}

	printf("%d\n", sum2);
}