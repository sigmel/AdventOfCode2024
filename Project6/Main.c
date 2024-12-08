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
	char* masterTextGrid = malloc(sizeof(char) * numColumns * numRows);
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex += numColumns + 1, dstIndex += numColumns)
	{
		memcpy(&masterTextGrid[dstIndex], &inputVector.data[srcIndex], sizeof(char) * numColumns);
	}


	// find our guard
	int masterSearchIndex = 0;
	while (1)
	{
		if (masterTextGrid[masterSearchIndex] == '^')
		{
			break;
		}

		masterSearchIndex++;
	}
	
	int sum = 0;

	// I'm not sure the best way to solve this, so going to brute force it again.
	// We can determine if a guard is in a loop if they enter the same cell going the same direction,
	// so we'll need to build a history then if they enter somewhere they have been before,
	// check to see if they are going the same direction as before, and if so we know we're in an infinite loop.
	struct Vector* history = malloc(sizeof(struct Vector) * numColumns * numRows);
	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	for (int historyIndex = 0; historyIndex < numColumns * numRows; historyIndex++)
	{
		initVector(&history[historyIndex]);
	}

	for (int obstacleIndex = 0; obstacleIndex < numColumns * numRows; obstacleIndex++)
	{
		// make sure we're trying a valid spot
		if (masterTextGrid[obstacleIndex] != '^' && masterTextGrid[obstacleIndex] != '#')
		{
			// reset our map
			memcpy(textGrid, masterTextGrid, sizeof(char) * numColumns * numRows);

			// reset any existing history
			for (int historyIndex = 0; historyIndex < numColumns * numRows; historyIndex++)
			{
				clearVector(&history[historyIndex]);
			}

			// set our new obstacle
			textGrid[obstacleIndex] = '#';

			int searchIndex = masterSearchIndex;

			int columnDirection = 0, rowDirection = -1;
			while (1)
			{
				int looped = 0;
				// check if we have a history here
				for (int historyIndex = 0; historyIndex < history[searchIndex].count; historyIndex += 2)
				{
					if (history[searchIndex].data[historyIndex] == columnDirection && history[searchIndex].data[historyIndex + 1] == rowDirection)
					{
						// we've hit a loop!
						sum++;
						looped = 1;
						break;
					}
				}
				if (looped)
				{
					break;
				}

				// mark our visited location
				addVector(&history[searchIndex], columnDirection);
				addVector(&history[searchIndex], rowDirection);

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
				int passable = 0;
				while (!passable) // need to keep checking to make sure we can actually move forward in this direction
				{
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
					else
					{
						passable = 1;
					}
				}

				// move forward
				searchIndex = searchIndex + rowDirection * numColumns + columnDirection;
			}
		}
	}

	printf("%d\n", sum);
}