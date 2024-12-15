#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VECTOR_DATA int
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

int isValidLocation(int x, int y, int numColumns, int numRows)
{
	if (x < 0) { return 0; }
	if (y < 0) { return 0; }
	if (x >= numColumns) { return 0; }
	if (y >= numRows) { return 0; }
	return 1;
}

void walkTrail(VECTOR_DATA* textGrid, int searchIndex, int numColumns, int numRows, int currentValue, struct Vector* foundSummit)
{
	currentValue += 1;
	int x = searchIndex % numColumns;
	int y = searchIndex / numColumns;
	if (isValidLocation(x - 1, y, numColumns, numRows) && textGrid[searchIndex - 1] - '0' == currentValue)
	{
		int nextSearchIndex = searchIndex - 1;
		if (currentValue == 9)
		{
			if (!containsValueInVector(foundSummit, nextSearchIndex))
			{
				addVector(foundSummit, nextSearchIndex);
			}
		}
		else
		{
			walkTrail(textGrid, nextSearchIndex, numColumns, numRows, currentValue, foundSummit);
		}
	}
	if (isValidLocation(x + 1, y, numColumns, numRows) && textGrid[searchIndex + 1] - '0' == currentValue)
	{
		int nextSearchIndex = searchIndex + 1;
		if (currentValue == 9)
		{
			if (!containsValueInVector(foundSummit, nextSearchIndex))
			{
				addVector(foundSummit, nextSearchIndex);
			}
		}
		else
		{
			walkTrail(textGrid, nextSearchIndex, numColumns, numRows, currentValue, foundSummit);
		}
	}
	if (isValidLocation(x, y - 1, numColumns, numRows) && textGrid[searchIndex - numColumns] - '0' == currentValue)
	{
		int nextSearchIndex = searchIndex - numColumns;
		if (currentValue == 9)
		{
			if (!containsValueInVector(foundSummit, nextSearchIndex))
			{
				addVector(foundSummit, nextSearchIndex);
			}
		}
		else
		{
			walkTrail(textGrid, nextSearchIndex, numColumns, numRows, currentValue, foundSummit);
		}
	}
	if (isValidLocation(x, y + 1, numColumns, numRows) && textGrid[searchIndex + numColumns] - '0' == currentValue)
	{
		int nextSearchIndex = searchIndex + numColumns;
		if (currentValue == 9)
		{
			if (!containsValueInVector(foundSummit, nextSearchIndex))
			{
				addVector(foundSummit, nextSearchIndex);
			}
		}
		else
		{
			walkTrail(textGrid, nextSearchIndex, numColumns, numRows, currentValue, foundSummit);
		}
	}
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
	VECTOR_DATA* textGrid = malloc(sizeof(VECTOR_DATA) * numColumns * numRows);
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex += numColumns + 1, dstIndex += numColumns)
	{
		memcpy(&textGrid[dstIndex], &inputVector.data[srcIndex], sizeof(VECTOR_DATA) * numColumns);
	}

	struct Vector trailVector;
	initVector(&trailVector);

	int sum = 0;

	// we'll use a stack to search all possible trailheads to see if they can reach a summit (still bruteforcing)
	// can do recursion since we have the constraint that it is always increasing, so we have definite limits that won't exceed ten levels
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == '0')
		{
			clearVector(&trailVector);

			walkTrail(textGrid, searchIndex, numColumns, numRows, 0, &trailVector);
			sum += trailVector.count;
		}
	}

	printf("%d\n", sum);
}