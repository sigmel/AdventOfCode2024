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

void copyVector(struct Vector* dst, struct Vector* src)
{
	dst->data = malloc(sizeof(int) * src->size);
	if (dst->data)
	{
		memcpy(dst->data, src->data, sizeof(int) * src->size);
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

void removeElementAtVector(struct Vector* vector, int index)
{
	assert(index < vector->count);
	for (int vectorIndex = index; vectorIndex < vector->count - 1; vectorIndex++)
	{
		vector->data[vectorIndex] = vector->data[vectorIndex + 1];
	}
	vector->count--;
}

void clearVector(struct Vector* vector)
{
	vector->count = 0;
}

int compare(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}

int testSafeInc(struct Vector* vector, int distance, int* bad1, int* bad2)
{
	for (int vectorIndex = 0; vectorIndex < vector->count - 1; vectorIndex++)
	{
		int diff = vector->data[vectorIndex + 1] - vector->data[vectorIndex];
		if (diff <= 0 || diff > distance)
		{
			if (bad1 && bad2)
			{
				*bad1 = vectorIndex;
				*bad2 = vectorIndex + 1;
			}
			return 0;
		}
	}
	return 1;
}

int testSafeDec(struct Vector* vector, int distance, int* bad1, int* bad2)
{
	for (int vectorIndex = 0; vectorIndex < vector->count - 1; vectorIndex++)
	{
		int diff = vector->data[vectorIndex + 1] - vector->data[vectorIndex];
		if (diff >= 0 || diff < -distance)
		{
			if (bad1 && bad2)
			{
				*bad1 = vectorIndex;
				*bad2 = vectorIndex + 1;
			}
			return 0;
		}
	}
	return 1;
}

void main()
{
	struct Vector inputVector;

	initVector(&inputVector);

	FILE* filePointer = NULL;
	filePointer = fopen("puzzle.txt", "r");

	int safeNum = 0;

#define INPUT_SIZE 100
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int value = 0;
		int advIndex = 0;
		do
		{
			value = atoi(&input[advIndex]);
			addVector(&inputVector, value);

			while (isdigit(input[advIndex])) { advIndex++; }
			while (isspace(input[advIndex])) { advIndex++; }
		} while (input[advIndex] != 0);

		const int SAFE_DISTANCE = 3;

		// check if increasing safe
		const int TOLERANCE_START = 1;
		int tolerance = TOLERANCE_START;
		int safe = 1; // safe until proven otherwise
		int bad1, bad2;

		// test if inc is safe
		if (!testSafeInc(&inputVector, SAFE_DISTANCE, &bad1, &bad2))
		{
			struct Vector testVector;
			copyVector(&testVector, &inputVector);
			removeElementAtVector(&testVector, bad1);
			if (!testSafeInc(&testVector, SAFE_DISTANCE, NULL, NULL))
			{
				freeVector(&testVector);
				copyVector(&testVector, &inputVector);
				removeElementAtVector(&testVector, bad2);
				safe = testSafeInc(&testVector, SAFE_DISTANCE, NULL, NULL);
			}
			freeVector(&testVector);
		}
		if (!safe)
		{
			// test is dec is safe
			safe = 1;
			if (!testSafeDec(&inputVector, SAFE_DISTANCE, &bad1, &bad2))
			{
				struct Vector testVector;
				copyVector(&testVector, &inputVector);
				removeElementAtVector(&testVector, bad1);
				if (!testSafeDec(&testVector, SAFE_DISTANCE, NULL, NULL))
				{
					freeVector(&testVector);
					copyVector(&testVector, &inputVector);
					removeElementAtVector(&testVector, bad2);
					safe = testSafeDec(&testVector, SAFE_DISTANCE, NULL, NULL);
				}
				freeVector(&testVector);
			}
		}

		safeNum += safe;
		clearVector(&inputVector);
	}

	fclose(filePointer);

	printf("%d\n", safeNum);
}