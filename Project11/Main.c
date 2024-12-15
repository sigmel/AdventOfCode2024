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

void addStone(struct Vector* vector, VECTOR_DATA value, VECTOR_DATA count)
{
	// find the stone (my kingdom for a hash map)
	for (int vectorIndex = 0; vectorIndex < vector->count; vectorIndex += 2)
	{
		if (vector->data[vectorIndex] == value)
		{
			vector->data[vectorIndex + 1] += count;
			return;
		}
	}

	// not found, so add it
	addVector(vector, value);
	addVector(vector, count);
}

void main()
{
	// stone values will now be the number than the count
	struct Vector stoneVector;
	initVector(&stoneVector);

	FILE* filePointer = NULL;
	filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 256
	char input[INPUT_SIZE];
	fgets(input, INPUT_SIZE, filePointer);
	int advIndex = 0;
	while (input[advIndex] != 0)
	{
		int value = atoi(&input[advIndex]);
		addVector(&stoneVector, value);
		addVector(&stoneVector, 1);

		while (isdigit(input[advIndex])) { advIndex++; }
		while (isspace(input[advIndex])) { advIndex++; }
	}

	// oh well, can't exactly brute force anymore. however, each number produces the same results, so we can group them to simplify
	for (int blinkIndex = 0; blinkIndex < 75; blinkIndex++)
	{
		struct Vector blinkVector;
		initVectorWithSize(&blinkVector, stoneVector.count * 2); // avoid some unnecessary copying from growing

		for (int stoneIndex = 0; stoneIndex < stoneVector.count; stoneIndex += 2)
		{
			if (stoneVector.data[stoneIndex] == 0)
			{
				addStone(&blinkVector, 1, stoneVector.data[stoneIndex + 1]);
			}
			else
			{
				sprintf(input, "%lld", stoneVector.data[stoneIndex]);
				size_t numDigits = strlen(input);
				if (numDigits % 2 == 0)
				{
					size_t newDigits = numDigits / 2;
					addStone(&blinkVector, atoll(&input[newDigits]), stoneVector.data[stoneIndex + 1]);
					input[newDigits] = 0;
					addStone(&blinkVector, atoll(input), stoneVector.data[stoneIndex + 1]);
				}
				else
				{
					addStone(&blinkVector, stoneVector.data[stoneIndex] * 2024, stoneVector.data[stoneIndex + 1]);
				}
			}
		}

		// copy our blink vector over to our stone one
		freeVector(&stoneVector);
		stoneVector.count = blinkVector.count;
		stoneVector.data = blinkVector.data;
		stoneVector.size = blinkVector.size;
	}

	// sum all our counts
	long long sum = 0;
	for (int vectorIndex = 1; vectorIndex < stoneVector.count; vectorIndex += 2)
	{
		sum += stoneVector.data[vectorIndex];
	}
	printf("%lld\n", sum);
}