#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
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

		// we have three operators, so bitfield is out
		// will just represent the values as separate bytes then,
		// but the basic procedure is more or less the same
		char* operators = malloc(sizeof(char) * (equation.count - 1));
		memset(operators, 0, sizeof(char) * (equation.count - 1));
		long long numPossibilities = (long long)pow(3, equation.count - 1);
		for (long long bitfield = 0; bitfield < numPossibilities; bitfield++)
		{
			// set our operand values
			long long opPossibility = bitfield;
			for (int opIndex = 0; opIndex < equation.count - 1; opIndex++)
			{
				operators[opIndex] = opPossibility % 3;
				opPossibility /= 3;
			}

			long long computedSum = equation.data[0];
			for (int bitIndex = 0; bitIndex < equation.count - 1; bitIndex++)
			{
				int sign = operators[bitIndex];
				if (sign == 0)
				{
					computedSum += equation.data[bitIndex + 1];
				}
				else if (sign == 1)
				{
					computedSum *= equation.data[bitIndex + 1];
				}
				else if (sign == 2)
				{
					char digitString[INPUT_SIZE];
					sprintf(digitString, "%lld%lld", computedSum, equation.data[bitIndex + 1]);
					computedSum = atoll(digitString);
				}
			}

			if (computedSum == value)
			{
				sum += computedSum;
				break;
			}
		}
		free(operators);
		clearVector(&equation);
	}

	printf("%lld", sum);
}