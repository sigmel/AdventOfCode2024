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
	if (dst->size < src->count)
	{
		free(dst->data);
		dst->data = malloc(sizeof(VECTOR_DATA) * src->size);
		dst->size = src->size;
	}
	memcpy(dst->data, src->data, sizeof(VECTOR_DATA) * src->count);
	dst->count = src->count;
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
	//memset(vector->data, 0, sizeof(VECTOR_DATA) * vector->size);
}

#define NUM_KEYS 12
struct NumericKeypad
{
	int keys[NUM_KEYS];
	int numColumns;
	int numRows;
	int current;
};

void initNumericKeypad(struct NumericKeypad* numKeypad)
{
	static const int keys[NUM_KEYS] = { 7, 8, 9, 4, 5, 6, 1, 2, 3, 'X', 0, 'A' };
	memcpy(numKeypad->keys, keys, sizeof(keys));
	numKeypad->numColumns = 3;
	numKeypad->numRows = 4;
	numKeypad->current = 11;
}

#define NUM_DIRECTIONS 6
struct DirectionalKeypad
{
	int keys[NUM_DIRECTIONS];
	int numColumns;
	int numRows;
	int current;
};

void initDirectionKeypad(struct DirectionalKeypad* dirKeypad)
{
	static const int keys[NUM_DIRECTIONS] = { 'X', '^', 'A', '<', 'v', '>' };
	memcpy(dirKeypad->keys, keys, sizeof(keys));
	dirKeypad->numColumns = 3;
	dirKeypad->numRows = 2;
	dirKeypad->current = 2;
}

void moveHorizontal(int* x, int endX, int dX, struct Vector* moveVector)
{
	while (*x != endX)
	{
		if (dX < 0) { addVector(moveVector, '<'); }
		else { addVector(moveVector, '>'); }
		*x += dX;
	}
}

void moveVertical(int* y, int endY, int dY, struct Vector* moveVector)
{
	while (*y != endY)
	{
		if (dY < 0) { addVector(moveVector, '^'); }
		else { addVector(moveVector, 'v'); }
		*y += dY;
	}
}

// the basic approach is that we want to move left to right on the dir pad, because we end on A, so getting there from < sucks
void numericKeypad(struct NumericKeypad* numKeypad, int destination, struct Vector* moveVector)
{
	int end = 0;
	for (int keyIndex = 0; keyIndex < NUM_KEYS; keyIndex++)
	{
		if (numKeypad->keys[keyIndex] == destination)
		{
			end = keyIndex;
			break;
		}
	}

	int x = numKeypad->current % numKeypad->numColumns;
	int y = numKeypad->current / numKeypad->numColumns;
	int endX = end % numKeypad->numColumns;
	int endY = end / numKeypad->numColumns;
	int dX = endX - x;
	if (dX != 0)
	{
		dX = dX / abs(dX); // normalize it
	}
	int dY = endY - y;
	if (dY != 0)
	{
		dY = dY / abs(dY);
	}

	// try to go left first, but we can't do that if we would cross the blank spot
	if (dX < 0)
	{
		if (y != numKeypad->numRows - 1 || endX != 0) // make sure we won't hit the blank spot going left
		{
			moveHorizontal(&x, endX, dX, moveVector);
			moveVertical(&y, endY, dY, moveVector);
		}
		else
		{
			// otherwise go the other way
			moveVertical(&y, endY, dY, moveVector);
			moveHorizontal(&x, endX, dX, moveVector);
		}
	}

	// handle any down moves assuming we don't go through the illegal space
	if (dY > 0)
	{
		if (x != 0 || endY != numKeypad->numRows - 1) // make sure we won't hit the blank spot going down
		{
			moveVertical(&y, endY, dY, moveVector);
			moveHorizontal(&x, endX, dX, moveVector);
		}
		else
		{
			moveHorizontal(&x, endX, dX, moveVector);
			moveVertical(&y, endY, dY, moveVector);
		}
	}

	// deal with any leftover moves
	moveVertical(&y, endY, dY, moveVector);
	moveHorizontal(&x, endX, dX, moveVector);

	addVector(moveVector, 'A');
	numKeypad->current = end;
}

void directionalKeypad(struct DirectionalKeypad* dirKeypad, struct Vector* moveVector)
{
	struct Vector targetVectors;
	initVector(&targetVectors);
	copyVector(&targetVectors, moveVector);

	clearVector(moveVector);

	for (int targetIndex = 0; targetIndex < targetVectors.count; targetIndex++)
	{
		int destination = targetVectors.data[targetIndex];

		int end = 0;
		for (int keyIndex = 0; keyIndex < NUM_KEYS; keyIndex++)
		{
			if (dirKeypad->keys[keyIndex] == destination)
			{
				end = keyIndex;
				break;
			}
		}

		int x = dirKeypad->current % dirKeypad->numColumns;
		int y = dirKeypad->current / dirKeypad->numColumns;
		int endX = end % dirKeypad->numColumns;
		int endY = end / dirKeypad->numColumns;
		int dX = endX - x;
		if (dX != 0)
		{
			dX = dX / abs(dX); // normalize it
		}
		int dY = endY - y;
		if (dY != 0)
		{
			dY = dY / abs(dY);
		}

		// try to go left first, but we can't do that if we would cross the blank spot
		if (dX < 0)
		{
			if (y != 0 || endX != 0) // make sure we won't hit the blank spot going left
			{
				moveHorizontal(&x, endX, dX, moveVector);
				moveVertical(&y, endY, dY, moveVector);
			}
			else
			{
				// otherwise go the other way
				moveVertical(&y, endY, dY, moveVector);
				moveHorizontal(&x, endX, dX, moveVector);
			}
		}

		// handle any up moves assuming we don't go through the illegal space
		if (dY > 0)
		{
			if (x != 0 || endY != 0) // make sure we won't hit the blank spot going down
			{
				moveVertical(&y, endY, dY, moveVector);
				moveHorizontal(&x, endX, dX, moveVector);
			}
			else
			{
				moveHorizontal(&x, endX, dX, moveVector);
				moveVertical(&y, endY, dY, moveVector);
			}
		}

		// deal with any leftover moves
		moveVertical(&y, endY, dY, moveVector);
		moveHorizontal(&x, endX, dX, moveVector);

		addVector(moveVector, 'A');
		dirKeypad->current = end;
	}

	freeVector(&targetVectors);
}

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	struct Vector moveVector;
	initVector(&moveVector);

	struct NumericKeypad numKeypad;
	initNumericKeypad(&numKeypad);

	struct DirectionalKeypad dirKeypad0;
	initDirectionKeypad(&dirKeypad0);

	struct DirectionalKeypad dirKeypad1;
	initDirectionKeypad(&dirKeypad1);

	FILE* filePointer = fopen("puzzle.txt", "r");

	int totalComplexity = 0;

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		clearVector(&inputVector);
		clearVector(&moveVector);

		size_t length = strlen(input);
		if (input[length - 1] == '\n')
		{
			input[length - 1] = 0;
			length--;
		}
		for (int charIndex = 0; charIndex < (int)length; charIndex++)
		{
			if (isdigit(input[charIndex]))
			{
				addVector(&inputVector, input[charIndex] - '0');
			}
			else
			{
				addVector(&inputVector, input[charIndex]);
			}
		}

		// get the numeric code
		int numericCode = atoi(input);

		//printf("%dA\n", numericCode);

		// determine our moves
		for (int inputIndex = 0; inputIndex < inputVector.count; inputIndex++)
		{
			// first figure out what our moves need to be for the numeric keypad
			numericKeypad(&numKeypad, inputVector.data[inputIndex], &moveVector);
		}

		/*
		for (int moveIndex = 0; moveIndex < moveVector.count; moveIndex++)
		{
			printf("%c", moveVector.data[moveIndex]);
		}
		printf("\n");
		*/

		// now convert that into a direction keypad
		directionalKeypad(&dirKeypad0, &moveVector);

		/*
		for (int moveIndex = 0; moveIndex < moveVector.count; moveIndex++)
		{
			printf("%c", moveVector.data[moveIndex]);
		}
		printf("\n");
		*/

		// and again
		directionalKeypad(&dirKeypad1, &moveVector);

		/*
		for (int moveIndex = 0; moveIndex < moveVector.count; moveIndex++)
		{
			printf("%c", moveVector.data[moveIndex]);
		}
		printf("\n");
		*/

		// and determine the complexity
		int complexity = numericCode * moveVector.count;
		totalComplexity += complexity;
	}

	printf("%d\n", totalComplexity);
}