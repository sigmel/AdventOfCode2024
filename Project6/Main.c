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

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	FILE* filePointer = fopen("puzzle.txt", "r");

	// start by reading the whole file into memory
	int value = 0;
	do {
		value = fgetc(filePointer);
		addVector(&inputVector, value);
	} while (value != EOF);
	inputVector.data[inputVector.count - 1] = '\n'; // make the EOF a newline as well to keep it consistent

	fclose(filePointer);

	// determine the number of columns
	int numColumns = 0;
	while (inputVector.data[numColumns] != '\n') numColumns++;

	int numRows = inputVector.count / (numColumns + 1); // add 1 since it'll include the newlines currently

	// create and fill our "2D" array
	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex += numColumns + 1, dstIndex += numColumns)
	{
		memcpy(&textGrid[dstIndex], &inputVector.data[srcIndex], sizeof(char) * numColumns);
	}


	// find our guard
	int searchIndex = 0;
	while (1)
	{
		if (textGrid[searchIndex] == '^')
		{
			break;
		}

		searchIndex++;
	}
	
	int sum = 0;
	int columnDirection = 0, rowDirection = -1;
	while (1)
	{
		// mark our visited location
		if (textGrid[searchIndex] != 'X')
		{
			textGrid[searchIndex] = 'X';
			sum++;
		}

		// check to see if we are about to walk off grid
		if ((columnDirection < 0) && (searchIndex % numColumns == 0))
		{
			break;
		}
		else if ((columnDirection > 0) && ((searchIndex + 1) % numColumns == 0))
		{
			break;
		}

		if ((rowDirection < 0) && (searchIndex - numColumns < 0))
		{
			break;
		}
		else if ((rowDirection > 0) && (searchIndex + numColumns >= numColumns * numRows))
		{
			break;
		}

		// check to see if we are about to hit something
		if (textGrid[searchIndex + rowDirection * numColumns + columnDirection] == '#')
		{
			if (columnDirection == 0 && rowDirection == -1)
			{
				columnDirection = 1;
				rowDirection = 0;
			}
			else if (columnDirection == 1 && rowDirection == 0)
			{
				columnDirection = 0;
				rowDirection = 1;
			}
			else if (columnDirection == 0 && rowDirection == 1)
			{
				columnDirection = -1;
				rowDirection = 0;
			}
			else if (columnDirection == -1 && rowDirection == 0)
			{
				columnDirection = 0;
				rowDirection = -1;
			}

		}

		// move forward
		searchIndex = searchIndex + rowDirection * numColumns + columnDirection;
	}

	printf("%d\n", sum);
}