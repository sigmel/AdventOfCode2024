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

int getFreePath(struct Vector* freePaths)
{
	assert(freePaths->count > 0);
	freePaths->count--;
	return freePaths->data[freePaths->count];
}

// adapted this from Wikipedia: https://en.wikipedia.org/wiki/A*_search_algorithm
// to find all paths, we need to take into consideration the direction we came from
// also, we need to keep track of the path we've taken so far so that we can mark all
// the visited tiles once we reach the end
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

	int** cameFrom = malloc(sizeof(int*) * 4);
	int** gScore = malloc(sizeof(int*) * 4);
	int** fScore = malloc(sizeof(int*) * 4);
	for (int directionIndex = 0; directionIndex < 4; directionIndex++)
	{
		cameFrom[directionIndex] = malloc(sizeof(int) * numColumns * numRows);
		gScore[directionIndex] = malloc(sizeof(int) * numColumns * numRows);
		fScore[directionIndex] = malloc(sizeof(int) * numColumns * numRows);
		for (int setIndex = 0; setIndex < numColumns * numRows; setIndex++)
		{
			gScore[directionIndex][setIndex] = INT_MAX;
			fScore[directionIndex][setIndex] = INT_MAX;
		}

		memset(cameFrom[directionIndex], 0, sizeof(int) * numColumns * numRows);
		gScore[directionIndex][start] = 0;
		fScore[directionIndex][start] = 0;
	}

#define MAX_POSSIBLE_PATHS 1000000
	struct Vector* possiblePaths;
	possiblePaths = malloc(sizeof(struct Vector) * MAX_POSSIBLE_PATHS);
	for (int pathIndex = 0; pathIndex < MAX_POSSIBLE_PATHS; pathIndex++)
	{
		initVector(&possiblePaths[pathIndex]);
	}

	struct Vector freePaths;
	initVectorWithSize(&freePaths, MAX_POSSIBLE_PATHS);
	for (int pathIndex = MAX_POSSIBLE_PATHS - 1; pathIndex >= 0; pathIndex--)
	{
		addVector(&freePaths, pathIndex);
	}

	struct Vector openSet;
	initVector(&openSet);
	addVector(&openSet, start);

	struct Vector openDirection;
	initVector(&openDirection);
	addVector(&openDirection, 0);

	struct Vector openPaths;
	initVector(&openPaths);

	int freePathIndex = getFreePath(&freePaths);
	addVector(&possiblePaths[freePathIndex], start);
	addVector(&openPaths, freePathIndex);

	int* touchedTiles = malloc(sizeof(int) * numColumns * numRows);
	memset(touchedTiles, 0, sizeof(int) * numColumns * numRows);

	int endGScore = INT_MAX;

	while (openSet.count > 0)
	{
		int currentIndex = 0;
		int current = openSet.data[currentIndex];
		int lastDirection = openDirection.data[currentIndex];
		int currentPath = openPaths.data[currentIndex];
		int lowestFScore = fScore[lastDirection][current];
		for (int openSetIndex = 1; openSetIndex < openSet.count; openSetIndex++)
		{
			int checkCurrent = openSet.data[openSetIndex];
			int checkDirection = openDirection.data[openSetIndex];
			int fScoreValue = fScore[checkDirection][checkCurrent];
			if (fScoreValue < lowestFScore)
			{
				current = checkCurrent;
				lowestFScore = fScoreValue;
				currentIndex = openSetIndex;
				lastDirection = checkDirection;
				currentPath = openPaths.data[openSetIndex];
			}
		}

		removeElementAtVector(&openSet, currentIndex);
		removeElementAtVector(&openDirection, currentIndex);
		removeElementAtVector(&openPaths, currentIndex);

		if (current == end)
		{
			// verify that this is one of the shortest paths
			assert(endGScore == INT_MAX || endGScore == gScore[lastDirection][end]);
			endGScore = gScore[lastDirection][end];

			for (int pathIndex = 0; pathIndex < possiblePaths[currentPath].count; pathIndex++)
			{
				touchedTiles[possiblePaths[currentPath].data[pathIndex]] = 1;
			}

			continue; // go back and find the next eligible path
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

			int tentativeGScore = gScore[lastDirection][current];
			if (textGrid[neighbor] == '#')
			{
				continue; // can't go this way
			}
			else if (directionIndex != lastDirection)
			{
				tentativeGScore += 1001; // add an additional 1 for the movement forward
				if (abs(lastDirection - directionIndex) == 2)
				{
					continue; // no need to check the direction we came from
				}
			}
			else
			{
				tentativeGScore += 1;
			}

			if (tentativeGScore <= gScore[directionIndex][neighbor] && tentativeGScore <= endGScore) // no need to continue if this will be more than a shorter finished path
			{
				cameFrom[directionIndex][neighbor] = current;
				gScore[directionIndex][neighbor] = tentativeGScore;
				int path = getFreePath(&freePaths);
				copyVector(&possiblePaths[path], &possiblePaths[currentPath]);
				addVector(&possiblePaths[path], neighbor);

				// for heuristic, we'll take a simple Manhattan distance approach
				int endX = end % numColumns;
				int endY = end / numColumns;
				int neighborX = neighbor % numColumns;
				int neighborY = neighbor / numColumns;
				int heuristic = abs(endX - neighborX) + abs(endY - neighborY);
				fScore[directionIndex][neighbor] = tentativeGScore + heuristic;
				{
					addVector(&openSet, neighbor);
					addVector(&openDirection, directionIndex);
					addVector(&openPaths, path);
				}
			}
		}

		addVector(&freePaths, currentPath); // always return our current path
	}

	// checked everything, so now figure out how many tiles we have touched
	int numTouchedTiles = 0;
	for (int tileIndex = 0; tileIndex < numColumns * numRows; tileIndex++)
	{
		if (touchedTiles[tileIndex] != 0)
		{
			numTouchedTiles++;
		}
	}

	// debug print
	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < numColumns; x++)
		{
			if (touchedTiles[y * numColumns + x] != 0)
			{
				printf("O");
			}
			else
			{
				printf("%c", textGrid[y * numColumns + x]);
			}
		}
		printf("\n");
	}

	return numTouchedTiles;
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