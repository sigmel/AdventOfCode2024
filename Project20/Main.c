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

int isValidLocation(int x, int y, int numColumns, int numRows)
{
	if (x < 0) { return 0; }
	if (y < 0) { return 0; }
	if (x >= numColumns) { return 0; }
	if (y >= numRows) { return 0; }
	return 1;
}

#define EAST 0
#define SOUTH 1
#define WEST 2
#define NORTH 3

// adapted this from Wikipedia: https://en.wikipedia.org/wiki/A*_search_algorithm
int aStar(const VECTOR_DATA* textGrid, int numColumns, int numRows, int start, int end, int* cameFrom, int* gScore, int* fScore, struct Vector* shortestPath)
{
	for (int setIndex = 0; setIndex < numColumns * numRows; setIndex++)
	{
		gScore[setIndex] = INT_MAX;
		fScore[setIndex] = INT_MAX;
	}

	memset(cameFrom, 0, sizeof(int) * numColumns * numRows);
	gScore[start] = 0;
	fScore[start] = 0;

	struct Vector openSet;
	initVector(&openSet);
	addVector(&openSet, start);

	int endGScore = INT_MAX;

	while (openSet.count > 0)
	{
		int currentIndex = 0;
		int current = openSet.data[currentIndex];
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
			}
		}

		removeElementAtVector(&openSet, currentIndex);

		if (current == end)
		{
			if (shortestPath != NULL)
			{
				int path = end;
				while (path != start)
				{
					addVector(shortestPath, path);
					path = cameFrom[path];
				}
				addVector(shortestPath, path);
			}
			return gScore[current];
		}

		for (int directionIndex = 0; directionIndex < 4; directionIndex++)
		{
			int dX = 0, dY = 0;
			if (directionIndex == EAST)
			{
				dX = 1;
			}
			else if (directionIndex == SOUTH)
			{
				dY = 1;
			}
			else if (directionIndex == WEST)
			{
				dX = -1;
			}
			else if (directionIndex == NORTH)
			{
				dY = -1;
			}
			int neighbor = current + (dY * numColumns + dX);

			int tentativeGScore = gScore[current];
			if (textGrid[neighbor] == '#')
			{
				continue; // can't go this way
			}
			else
			{
				tentativeGScore += 1;
			}

			if (tentativeGScore <= gScore[neighbor] && tentativeGScore <= endGScore) // no need to continue if this will be more than a shorter finished path
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
				{
					addVector(&openSet, neighbor);
				}
			}
		}
	}

	return INT_MAX;
}

int race(const VECTOR_DATA* textGrid, int numColumns, int numRows)
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

	int* cameFrom = malloc(sizeof(int) * numColumns * numRows);
	int* gScore = malloc(sizeof(int) * numColumns * numRows);
	int* fScore = malloc(sizeof(int) * numColumns * numRows);

	struct Vector shortestPath;
	initVector(&shortestPath);
	int initialScore = aStar(textGrid, numColumns, numRows, start, end, cameFrom, gScore, fScore, &shortestPath);

	/*
	VECTOR_DATA* debugGrid = malloc(sizeof(VECTOR_DATA) * numColumns * numRows);
	memcpy(debugGrid, textGrid, sizeof(VECTOR_DATA) * numColumns * numRows);
	for (int debugIndex = 0; debugIndex < shortestPath.count; debugIndex++)
	{
		debugGrid[shortestPath.data[debugIndex]] = 'O';
	}
	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < numColumns; x++)
		{
			printf("%c", debugGrid[y * numColumns + x]);
		}
		printf("\n");
	}
	*/

	VECTOR_DATA* testGrid = malloc(sizeof(VECTOR_DATA) * numColumns * numRows);
	for (int setIndex = 0; setIndex < numColumns * numRows; setIndex++)
	{
		testGrid[setIndex] = INT_MAX;
	}
	for (int pathIndex = 0; pathIndex < shortestPath.count; pathIndex++)
	{
		testGrid[shortestPath.data[pathIndex]] = pathIndex;
	}

	const int threshold = 100;
	const int cheatAmount = 20;

	struct Vector cheatSpace;
	initVector(&cheatSpace);

	// create our circle of offsets
	for (int cheatY = cheatAmount; cheatY >= -cheatAmount; cheatY--)
	{
		for (int cheatX = -(cheatAmount - abs(cheatY)); cheatX <= cheatAmount - abs(cheatY); cheatX++)
		{
			addVector(&cheatSpace, cheatX);
			addVector(&cheatSpace, cheatY);
		}
	}

	int numCheats = 0;

	// There is only a single path for these "shortest" solutions, so if we wind up on one again,
	// we can just pick back up from there. Also, since we need to find everything, we can basically just iterate
	// over a circular grid at each location and see what the end result would be.
	for (int pathIndex = 2; pathIndex < shortestPath.count; pathIndex++) // can skip the end space and the one right before that
	{
		int position = shortestPath.data[pathIndex];
		int x = position % numColumns;
		int y = position / numColumns;

		for (int cheatIndex = 0; cheatIndex < cheatSpace.count; cheatIndex += 2)
		{
			int dX = cheatSpace.data[cheatIndex];
			int dY = cheatSpace.data[cheatIndex + 1];
			int testPosition = position + (dY * numColumns + dX);

			// make sure we end on an empty space
			if (isValidLocation(x + dX, y + dY, numColumns, numRows) && textGrid[testPosition] != '#')
			{
				// see what our cost is from here (cost of cheat path + cost of getting here + cost remaining)
				int cheatCost = abs(dX) + abs(dY);
				int currentCost = (shortestPath.count - 1) - pathIndex;
				int remainingCost = testGrid[testPosition];
				int cheatScore = cheatCost + currentCost + remainingCost;
				if (cheatScore + threshold <= initialScore)
				{
					//printf("Saved %d picoseconds going %d, %d at %d, %d\n", initialScore - cheatScore, dX, dY, x, y);
					numCheats++;
				}
			}
		}
	}

	return numCheats;
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

	int score = race(textGrid, numColumns, numRows);
	printf("%d\n", score);
}