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

struct Bounds
{
	int xMin;
	int xMax;
	int yMin;
	int yMax;
	char* plot; // need to have a copy of just this plot since our bounds can overlap other plots of the same type
};

void floodFill(const char* textGrid, char* floodGrid, char value, int searchIndex, int numColumns, int numRows, int* area, int* perimeter, struct Bounds* bounds)
{
	if (textGrid[searchIndex] == value && floodGrid[searchIndex] != value)
	{
		// set our value
		floodGrid[searchIndex] = value;
		bounds->plot[searchIndex] = value;

		// compute area and perimeter
		(*area)++;

		int currentPerimeter = 4;

		int x = searchIndex % numColumns;
		int y = searchIndex / numColumns;
		if (isValidLocation(x - 1, y, numColumns, numRows) && floodGrid[searchIndex - 1] == value)
		{
			currentPerimeter -= 2;
		}
		if (isValidLocation(x + 1, y, numColumns, numRows) && floodGrid[searchIndex + 1] == value)
		{
			currentPerimeter -= 2;
		}
		if (isValidLocation(x, y - 1, numColumns, numRows) && floodGrid[searchIndex - numColumns] == value)
		{
			currentPerimeter -= 2;
		}
		if (isValidLocation(x, y + 1, numColumns, numRows) && floodGrid[searchIndex + numColumns] == value)
		{
			currentPerimeter -= 2;
		}

		(*perimeter) += currentPerimeter;

		// compute our bounds
		if (x < bounds->xMin) { bounds->xMin = x; }
		if (x > bounds->xMax) { bounds->xMax = x; }
		if (y < bounds->yMin) { bounds->yMin = y; }
		if (y > bounds->yMax) { bounds->yMax = y; }

		// continue to fill
		if (isValidLocation(x - 1, y, numColumns, numRows))
		{
			floodFill(textGrid, floodGrid, value, searchIndex - 1, numColumns, numRows, area, perimeter, bounds);
		}
		if (isValidLocation(x + 1, y, numColumns, numRows))
		{
			floodFill(textGrid, floodGrid, value, searchIndex + 1, numColumns, numRows, area, perimeter, bounds);
		}
		if (isValidLocation(x, y - 1, numColumns, numRows))
		{
			floodFill(textGrid, floodGrid, value, searchIndex - numColumns, numColumns, numRows, area, perimeter, bounds);
		}
		if (isValidLocation(x, y + 1, numColumns, numRows))
		{
			floodFill(textGrid, floodGrid, value, searchIndex + numColumns, numColumns, numRows, area, perimeter, bounds);
		}
	}
}

int findSides(const struct Bounds* bounds, char value, int searchIndex, int numColumns, int numRows)
{
	int numSides = 0;

	// find all horiztonal sides
	for (int y = bounds->yMin; y <= bounds->yMax; y++)
	{
		int topSideStart = -1;
		int bottomSideStart = -1;
		for (int x = bounds->xMin; x <= bounds->xMax; x++)
		{
			if (bounds->plot[y * numColumns + x] == value)
			{
				// check to make sure that we are on a boundary
				if (!isValidLocation(x, y - 1, numColumns, numRows) || bounds->plot[(y - 1) * numColumns + x] != value)
				{
					if (topSideStart == -1)
					{
						topSideStart = x;
					}
				}
				else if (topSideStart >= 0)
				{
					numSides++;
					topSideStart = -1;
				}

				if (!isValidLocation(x, y + 1, numColumns, numRows) || bounds->plot[(y + 1) * numColumns + x] != value)
				{
					if (bottomSideStart == -1)
					{
						bottomSideStart = x;
					}
				}
				else if (bottomSideStart >= 0)
				{
					numSides++;
					bottomSideStart = -1;
				}
			}
			else
			{
				// if we are no longer in our plot, then end any sides we started
				if (topSideStart >= 0)
				{
					numSides++;
					topSideStart = -1;
				}
				if (bottomSideStart >= 0)
				{
					numSides++;
					bottomSideStart = -1;
				}
			}
		}

		// done with this row, so finish any sides we started
		if (topSideStart >= 0)
		{
			numSides++;
			topSideStart = -1;
		}
		if (bottomSideStart >= 0)
		{
			numSides++;
			bottomSideStart = -1;
		}
	}

	// find all vertical sides
	for (int x = bounds->xMin; x <= bounds->xMax; x++)
	{
		int leftSideStart = -1;
		int rightSideStart = -1;
		for (int y = bounds->yMin; y <= bounds->yMax; y++)
		{
			if (bounds->plot[y * numColumns + x] == value)
			{
				// check to make sure that we are on a boundary
				if (!isValidLocation(x - 1, y, numColumns, numRows) || bounds->plot[y * numColumns + x - 1] != value)
				{
					if (leftSideStart == -1)
					{
						leftSideStart = y;
					}
				}
				else if (leftSideStart >= 0)
				{
					numSides++;
					leftSideStart = -1;
				}

				if (!isValidLocation(x + 1, y, numColumns, numRows) || bounds->plot[y * numColumns + x + 1] != value)
				{
					if (rightSideStart == -1)
					{
						rightSideStart = y;
					}
				}
				else if (rightSideStart >= 0)
				{
					numSides++;
					rightSideStart = -1;
				}
			}
			else
			{
				// if we are no longer in our plot, then end any sides we started
				if (leftSideStart >= 0)
				{
					numSides++;
					leftSideStart = -1;
				}
				if (rightSideStart >= 0)
				{
					numSides++;
					rightSideStart = -1;
				}
			}
		}

		// done with this row, so finish any sides we started
		if (leftSideStart >= 0)
		{
			numSides++;
			leftSideStart = -1;
		}
		if (rightSideStart >= 0)
		{
			numSides++;
			rightSideStart = -1;
		}
	}

	return numSides;
}

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	struct Vector plotTypes;
	initVector(&plotTypes);

	FILE* filePointer = fopen("puzzle.txt", "r");

	// start by reading the whole file into memory
	int value = 0;
	do {
		value = fgetc(filePointer);
		addVector(&inputVector, value);

		// create a list of our unique plot types
		if (isalnum(value))
		{
			if (!containsValueInVector(&plotTypes, value))
			{
				addVector(&plotTypes, value);
			}
		}
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

	// have another array that indicates what we've grouped already
	char* floodGrid = malloc(sizeof(char) * numColumns * numRows);
	memset(floodGrid, '.', sizeof(char) * numColumns * numRows);

	char* boundsPlot = malloc(sizeof(char) * numColumns * numRows);
	memset(boundsPlot, '.', sizeof(char) * numColumns * numRows);

	// this is a flood fill problem, so we'll need to spread out and find all the connecting areas of the same type
	long long cost = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (floodGrid[searchIndex] == '.')
		{
			char plotType = textGrid[searchIndex];
			int area = 0, perimeter = 0;

			// figure out the inclusive bounds for our area while we are flood filling
			struct Bounds bounds;
			bounds.xMin = numColumns - 1;
			bounds.xMax = 0;
			bounds.yMin = numRows - 1;
			bounds.yMax = 0;
			bounds.plot = boundsPlot;

			floodFill(textGrid, floodGrid, plotType, searchIndex, numColumns, numRows, &area, &perimeter, &bounds);
			int numSides = findSides(&bounds, plotType, searchIndex, numColumns, numRows);
			cost += area * numSides;

			memset(boundsPlot, '.', sizeof(char) * numColumns * numRows);
		}
	}

	printf("%lld\n", cost);
}