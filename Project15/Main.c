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

int isValidLocation(int x, int y, int numColumns, int numRows)
{
	if (x < 0) { return 0; }
	if (y < 0) { return 0; }
	if (x >= numColumns) { return 0; }
	if (y >= numRows) { return 0; }
	return 1;
}

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	FILE* filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 256
	char input[INPUT_SIZE];

	int numColumns = 0;
	int numRows = 0;

	while (1)
	{
		fgets(input, INPUT_SIZE, filePointer);
		if (input[0] == '\n')
		{
			// we hit the separator between the map and the moves, so let's prepare for the next part
			break;
		}

		int length = (int)strlen(input);
		if (numColumns == 0)
		{
			numColumns = length - 1;
		}
		numRows++;

		for (int stringIndex = 0; stringIndex < length; stringIndex++)
		{
			addVector(&inputVector, input[stringIndex]);
		}
	}

	// create and fill our "2D" array
	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex += numColumns + 1, dstIndex += numColumns)
	{
		memcpy(&textGrid[dstIndex], &inputVector.data[srcIndex], sizeof(char) * numColumns);
	}

	// get our moves
	struct Vector moveVector;
	initVector(&moveVector);

	while (1)
	{
		char value = fgetc(filePointer);
		if (value == EOF)
		{
			break;
		}

		if (value != '\n')
		{
			addVector(&moveVector, value);
		}
	}
	fclose(filePointer);

	// find our robot
	int robotIndex = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == '@')
		{
			robotIndex = searchIndex;
			textGrid[searchIndex] = '.'; // mark as empty
			break;
		}
	}

	// now move it accordingly
	for (int moveIndex = 0; moveIndex < moveVector.count; moveIndex++)
	{
		char move = moveVector.data[moveIndex];

		int dX = 0;
		int dY = 0;
		if (move == '^')
		{
			dY = -1;
		}
		else if (move == 'v')
		{
			dY = 1;
		}
		else if (move == '<')
		{
			dX = -1;
		}
		else if (move == '>')
		{
			dX = 1;
		}

		int nextIndex = robotIndex + (dY * numColumns) + dX;
		if (textGrid[nextIndex] == '.') // empty space
		{
			robotIndex = nextIndex;
		}
		else if (textGrid[nextIndex] == '#') // wall
		{
			// do nothing
		}
		else if (textGrid[nextIndex] == 'O') // box
		{
			// check if we can push
			int canPush = 0;
			int pushIndex = nextIndex;
			while (1)
			{
				pushIndex += (dY * numColumns) + dX;
				if (textGrid[pushIndex] == '.')
				{
					canPush = 1;
					break;
				}
				else if (textGrid[pushIndex] == '#')
				{
					break;
				}
			}

			if (canPush)
			{
				while (1)
				{
					int pusheeIndex = pushIndex - ((dY * numColumns) + dX);
					if (pusheeIndex == robotIndex)
					{
						// reached our robot so we are done pushing
						robotIndex = nextIndex;
						break;
					}
					textGrid[pushIndex] = textGrid[pusheeIndex];
					textGrid[pusheeIndex] = '.';
					pushIndex = pusheeIndex;
				}
			}
		}
	}

	// find our sum
	int sum = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == 'O')
		{
			int x = searchIndex % numColumns;
			int y = searchIndex / numColumns;
			sum += 100 * y + x;
		}
	}

	printf("%d\n", sum);
}