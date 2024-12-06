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

void countMS(char* textGrid, int searchIndex, int columnDirection, int rowDirection, const int numColumns, const int numRows, int* numM, int* numS)
{
	// check our bounds
	if ((columnDirection < 0) && (searchIndex % numColumns == 0))
	{
		return;
	}
	else if ((columnDirection > 0) && ((searchIndex + 1) % numColumns == 0))
	{
		return;
	}

	if ((rowDirection < 0) && (searchIndex - numColumns < 0))
	{
		return;
	}
	else if ((rowDirection > 0) && (searchIndex + numColumns >= numColumns * numRows))
	{
		return;
	}

	int index = searchIndex + columnDirection + (rowDirection * numColumns);
	char value = textGrid[index];
	if (value == 'M')
	{
		(*numM)--;
	}
	else if (value == 'S')
	{
		(*numS)--;
	}
}

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	FILE* filePointer = fopen("puzzle.txt", "r");

	// start by reading the whole file into memory
	int value = 0;
	do	{
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

	// basic algorithm is going to be to look for the center value, then check one diagonal and verify it has one 'S' and one 'M', then the other
	int total = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == 'A')
		{
			int numM = 2, numS = 2;
			countMS(textGrid, searchIndex, -1, -1, numColumns, numRows, &numM, &numS);
			countMS(textGrid, searchIndex,  1,  1, numColumns, numRows, &numM, &numS);

			if (numM == 1 && numS == 1)
			{
				countMS(textGrid, searchIndex, -1,  1, numColumns, numRows, &numM, &numS);
				countMS(textGrid, searchIndex,  1, -1, numColumns, numRows, &numM, &numS);

				if (numM == 0 && numS == 0)
				{
					total++;
				}
			}
		}
	}

	printf("%d\n", total);
}