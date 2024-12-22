#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VECTOR_DATA char
struct Vector
{
	int count;
	int size;
	VECTOR_DATA* data;
};

void initVector(struct Vector* vector)
{
	const int VECTOR_INIT_SIZE = 10;
	vector->count = 0;
	vector->size = VECTOR_INIT_SIZE;
	vector->data = malloc(sizeof(VECTOR_DATA) * vector->size);
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

int findPattern(const struct Vector* patternVector, int numPatterns, char* pattern)
{
	for (int patternIndex = 0; patternIndex < numPatterns; patternIndex++)
	{
		if (memcmp(patternVector[patternIndex].data, pattern, sizeof(VECTOR_DATA) * patternVector[patternIndex].count) == 0)
		{
			if (pattern[patternVector[patternIndex].count] == 0)
			{
				return 1;
			}
			else
			{
				if (findPattern(patternVector, numPatterns, &pattern[patternVector[patternIndex].count]))
				{
					return 1;
				}
			}
		}
	}
	return 0;
}

void main()
{
#define MAX_PATTERNS 10000
	struct Vector* patternVector = malloc(sizeof(struct Vector) * MAX_PATTERNS);

	FILE* filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];

	// get our patterns
	fgets(input, INPUT_SIZE, filePointer);
	int advIndex = 0;
	int numPatterns = 0;
	initVector(&patternVector[numPatterns]);
	while (input[advIndex] != '\n')
	{
		if (input[advIndex] == ',')
		{
			numPatterns++;
			initVector(&patternVector[numPatterns]);
		}
		else if (isalpha(input[advIndex]))
		{
			addVector(&patternVector[numPatterns], input[advIndex]);
		}
		advIndex++;
	}
	numPatterns++;

	fgets(input, INPUT_SIZE, filePointer); // newline

	// we could get fancy with tokenizing or something, but let's just try brute force to start
	int possiblePatterns = 0;
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int length = (int)strlen(input);
		if (input[length - 1] == '\n')
		{
			input[length - 1] = 0; // remove the newline
			length--;
		}

		if (findPattern(patternVector, numPatterns, input))
		{
			possiblePatterns++;
		}
	}

	printf("%d\n", possiblePatterns);
}