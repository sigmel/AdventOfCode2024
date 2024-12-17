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

void getBoxSides(const char* textGrid, int nextIndex, int* leftBox, int* rightBox)
{
	// find both sides of the box
	if (textGrid[nextIndex] == '[')
	{
		*leftBox = nextIndex;
	}
	else
	{
		*leftBox = nextIndex - 1;
	}
	*rightBox = *leftBox + 1;
}

int canPushBox(const char* textGrid, int nextIndex, int dX, int dY, int numColumns)
{
	// check if we can push
	int canPush = 0;
	int pushIndex = nextIndex + (dY * numColumns) + dX;
	if (textGrid[pushIndex] == '.')
	{
		canPush = 1;
	}
	else if (textGrid[pushIndex] == '#')
	{
	}
	else if (textGrid[pushIndex] == '[' || textGrid[pushIndex] == ']')
	{
		int leftBox, rightBox;
		getBoxSides(textGrid, pushIndex, &leftBox, &rightBox);

		// check if we can push
		// need to check both sides for up/down, but only the leading edge for left/right
		int canPushLeft = 1;
		int canPushRight = 1;
		if (dY != 0)
		{
			canPushLeft = canPushBox(textGrid, leftBox, dX, dY, numColumns);
			canPushRight = canPushBox(textGrid, rightBox, dX, dY, numColumns);
		}
		else if (dX < 0)
		{
			canPushLeft = canPushBox(textGrid, leftBox, dX, dY, numColumns);
		}
		else if (dX > 0)
		{
			canPushRight = canPushBox(textGrid, rightBox, dX, dY, numColumns);
		}
		if (canPushLeft && canPushRight)
		{
			canPush = 1;
		}
	}
	return canPush;
}

void pushBox(char* textGrid, int pushIndex, int dX, int dY, int numColumns)
{
	int delta = dY * numColumns + dX;

	// push any boxes
	// need to handle up/down, which can push two, but left/right can only push one (and need to ignore itself)
	int leftBox, rightBox;
	getBoxSides(textGrid, pushIndex, &leftBox, &rightBox);
	if (dY != 0)
	{
		if (textGrid[leftBox + delta] == '[' || textGrid[leftBox + delta] == ']')
		{
			pushBox(textGrid, leftBox + delta, dX, dY, numColumns);
		}
		if (textGrid[rightBox + delta] == '[' || textGrid[rightBox + delta] == ']')
		{
			pushBox(textGrid, rightBox + delta, dX, dY, numColumns);
		}
	}
	else if (dX < 0)
	{
		if (textGrid[leftBox + delta] == '[' || textGrid[leftBox + delta] == ']')
		{
			pushBox(textGrid, leftBox + delta, dX, dY, numColumns);
		}
	}
	else if (dX > 0)
	{
		if (textGrid[rightBox + delta] == '[' || textGrid[rightBox + delta] == ']')
		{
			pushBox(textGrid, rightBox + delta, dX, dY, numColumns);
		}
	}
	textGrid[leftBox] = '.';
	textGrid[rightBox] = '.';
	textGrid[leftBox + delta] = '[';
	textGrid[rightBox + delta] = ']';
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

	// double our warehouse width
	numColumns *= 2;

	// create and fill our "2D" array
	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	int robotIndex = 0;
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex++)
	{
		if (inputVector.data[srcIndex] == '#')
		{
			textGrid[dstIndex++] = '#';
			textGrid[dstIndex++] = '#';
		}
		else if (inputVector.data[srcIndex] == '.')
		{
			textGrid[dstIndex++] = '.';
			textGrid[dstIndex++] = '.';
		}
		else if (inputVector.data[srcIndex] == '@')
		{
			robotIndex = dstIndex;
			textGrid[dstIndex++] = '.';
			textGrid[dstIndex++] = '.';
		}
		else if (inputVector.data[srcIndex] == 'O')
		{
			textGrid[dstIndex++] = '[';
			textGrid[dstIndex++] = ']';
		}
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
		else if (textGrid[nextIndex] == '[' || textGrid[nextIndex] == ']') // box
		{
			int canPush = canPushBox(textGrid, robotIndex, dX, dY, numColumns);
			if (canPush)
			{
				pushBox(textGrid, nextIndex, dX, dY, numColumns);
				robotIndex = nextIndex;
			}
		}
	}

	// find our sum
	int sum = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == '[')
		{
			int x = searchIndex % numColumns;
			int y = searchIndex / numColumns;
			sum += 100 * y + x;
		}
	}

	printf("%d\n", sum);
}