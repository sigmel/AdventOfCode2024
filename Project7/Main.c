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

void clearVector(struct Vector* vector)
{
	vector->count = 0;
	memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");
	
	#define INPUT_SIZE 256
	char input[INPUT_SIZE];

	struct Vector equation;
	initVector(&equation);

	long long sum = 0;
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int advIndex = 0;
		long long value = atoll(&input[advIndex]);

		while (isdigit(input[advIndex])) { advIndex++; }
		advIndex += 2; // skip past the ': ' characters

		do
		{
			long long number = atoll(&input[advIndex]);
			addVector(&equation, number);

			while (isdigit(input[advIndex])) { advIndex++; }
			while (isspace(input[advIndex])) { advIndex++; }
		} while (input[advIndex] != 0);

		// algorithm is just to take an increasing number and treat it as a bitfield
		// where 0 is add and 1 is multiply
		// we'll keep trying until we find one that works or run out of digits
		// input doesn't look like it exceeds 32 numbers, so should be easy to represent as an int
		assert(equation.count < 32);
		int numPossibilities = 1 << (equation.count - 1);
		for (int bitfield = 0; bitfield < numPossibilities; bitfield++)
		{
			long long computedSum = equation.data[0];
			for (int bitIndex = 0; bitIndex < equation.count - 1; bitIndex++)
			{
				int sign = (bitfield >> bitIndex) & 1;
				if (sign)
				{
					computedSum *= equation.data[bitIndex + 1];
				}
				else
				{
					computedSum += equation.data[bitIndex + 1];
				}
			}

			if (computedSum == value)
			{
				sum += computedSum;
				break;
			}
		}

		clearVector(&equation);
	}

	printf("%lld", sum);
}