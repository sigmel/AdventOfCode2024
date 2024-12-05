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

void clearVector(struct Vector* vector)
{
	vector->count = 0;
	memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

// lastRead contains the last character read so we can use it
void readNumber(FILE* filePointer, struct Vector* inputVector, int* lastRead)
{
	clearVector(inputVector);

	*lastRead = fgetc(filePointer);
	while (isdigit(*lastRead))
	{
		addVector(inputVector, *lastRead);
		*lastRead = fgetc(filePointer);
	}
}

void main()
{

	FILE* filePointer = NULL;
	filePointer = fopen("puzzle.txt", "r");

	int sum = 0;

	int value = 0;
	do
	{
		// this is ugly but I don't care
		value = fgetc(filePointer);
		if (value == 'm')
		{
			value = fgetc(filePointer);
			if (value == 'u')
			{
				value = fgetc(filePointer);
				if (value == 'l')
				{
					value = fgetc(filePointer);
					if (value == '(')
					{
						struct Vector inputVector;
						initVector(&inputVector);
						readNumber(filePointer, &inputVector, &value);
						if (inputVector.count > 0 && value == ',')
						{
							int num1 = atoi(inputVector.data);
							readNumber(filePointer, &inputVector, &value);
							if (inputVector.count > 0 && value == ')')
							{
								int num2 = atoi(inputVector.data);
								sum += num1 * num2;
							}
						}
					}
				}
			}
		}
	} while (value != EOF);

	fclose(filePointer);

	printf("%d\n", sum);
}