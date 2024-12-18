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

// adapted this from Wikipedia: https://en.wikipedia.org/wiki/A*_search_algorithm
int aStar(const VECTOR_DATA* textGrid, int numColumns, int numRows)
{
	// find our start and end spots
	int start = 0, end = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (textGrid[searchIndex] == 'S')
		{
			start = searchIndex;
		}
		else if (textGrid[searchIndex] == 'E')
		{
			end = searchIndex;
		}
	}

	struct Vector openSet;
	initVector(&openSet);
	addVector(&openSet, start);
	
	struct Vector openDirection;
	initVector(&openDirection);
	addVector(&openDirection, 0);

	int* cameFrom = malloc(sizeof(int) * numColumns * numRows);
	memset(cameFrom, 0, sizeof(int) * numColumns * numRows);

	int* gScore = malloc(sizeof(int) * numColumns * numRows);
	int* fScore = malloc(sizeof(int) * numColumns * numRows);
	for (int setIndex = 0; setIndex < numColumns * numRows; setIndex++)
	{
		gScore[setIndex] = INT_MAX;
		fScore[setIndex] = INT_MAX;
	}
	gScore[start] = 0;
	fScore[start] = 0;

	while (openSet.count > 0)
	{
		int currentIndex = 0;
		int current = openSet.data[currentIndex];
		int lastDirection = openDirection.data[currentIndex];
		int lowestFScore = fScore[current];
		for (int openSetIndex = 1; openSetIndex < openSet.count; openSetIndex++)
		{
			int checkCurrent = openSet.data[openSetIndex];
			int fScoreValue = fScore[checkCurrent];
			if (fScoreValue < lowestFScore)
			{
				current = checkCurrent;
				lowestFScore = fScoreValue;
				currentIndex = openSetIndex;
				lastDirection = openDirection.data[openSetIndex];
			}
		}
		if (current == end)
		{
			return gScore[current];
		}

		removeElementAtVector(&openSet, currentIndex);
		removeElementAtVector(&openDirection, currentIndex);

		// 0 = East, 1 = South, 2 = West, 3 = North (this also matches the directions)
		for (int neighborCount = 0; neighborCount < 4; neighborCount++)
		{
			int dX = 0, dY = 0;
			if (neighborCount == 0)
			{
				dX = 1;
			}
			else if (neighborCount == 1)
			{
				dY = 1;
			}
			else if (neighborCount == 2)
			{
				dX = -1;
			}
			else if (neighborCount == 3)
			{
				dY = -1;
			}
			int neighbor = current + (dY * numColumns + dX);

			int tentativeGScore = gScore[current];
			if (textGrid[neighbor] == '#')
			{
				tentativeGScore = INT_MAX;
			}
			else if (neighborCount != lastDirection)
			{
				tentativeGScore += 1001; // add an additional 1 for the movement forward
				if (abs(lastDirection - neighborCount) == 2)
				{
					tentativeGScore += 1000; // it takes another 1000 to rotate 180
				}
			}
			else
			{
				tentativeGScore += 1;
			}

			if (tentativeGScore < gScore[neighbor])
			{
				cameFrom[neighbor] = current;
				gScore[neighbor] = tentativeGScore;

				// for heuristic, we'll take a simple Manhattan distance approach
				int endX = end % numColumns;
				int endY = end / numColumns;
				int neighborX = neighbor % numColumns;
				int neighborY = neighbor / numColumns;
				int heuristic = abs(endX - neighborX) + abs(endY - neighborY);
				fScore[neighbor] = tentativeGScore + heuristic;
				if (!containsValueInVector(&openSet, neighbor))
				{
					addVector(&openSet, neighbor);
					addVector(&openDirection, neighborCount);
				}
			}
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

	int score = aStar(textGrid, numColumns, numRows);
	printf("%d\n", score);
}