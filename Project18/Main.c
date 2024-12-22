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
int aStar(const char* textGrid, int numColumns, int numRows)
{
	// find our start and end spots
	int start = 0, end = (numColumns * numRows) - 1;
	struct Vector openSet;
	initVector(&openSet);
	addVector(&openSet, start);

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
		if (current == end)
		{
			char* pathGrid = malloc(sizeof(char) * numColumns * numRows);
			memcpy(pathGrid, textGrid, sizeof(char) * numColumns * numRows);
			int numSpots = 0;
			while (current != start)
			{
				pathGrid[current] = 'O';
				current = cameFrom[current];
				numSpots++;
			}
			pathGrid[current] = 'O';

			for (int y = 0; y < numRows; y++)
			{
				for (int x = 0; x < numColumns; x++)
				{
					printf("%c", pathGrid[y * numColumns + x]);
				}
				printf("\n");
			}

			return numSpots;
		}
		removeElementAtVector(&openSet, currentIndex);
		// 0 = East, 1 = South, 2 = West, 3 = North (this also matches the directions)
		for (int neighborCount = 0; neighborCount < 4; neighborCount++)
		{
			int x = current % numColumns;
			int y = current / numColumns;
			if (neighborCount == EAST)
			{
				x += 1;
			}
			else if (neighborCount == SOUTH)
			{
				y += 1;
			}
			else if (neighborCount == WEST)
			{
				x -= 1;
			}
			else if (neighborCount == NORTH)
			{
				y -= 1;
			}
			int neighbor = y * numColumns + x;
			int tentativeGScore = gScore[current];
			if (!isValidLocation(x, y, numColumns, numRows))
			{
				continue;
			}
			else if (textGrid[neighbor] == '#')
			{
				continue;
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
				int heuristic = abs(endX - x) + abs(endY - y);
				fScore[neighbor] = tentativeGScore + heuristic;
				if (!containsValueInVector(&openSet, neighbor))
				{
					addVector(&openSet, neighbor);
				}
			}
		}
	}
	return INT_MAX;
}

void main()
{
	struct Vector inputVector;
	initVector(&inputVector);

	FILE* filePointer = fopen("puzzle.txt", "r");

	// start by reading the whole file into memory
#define INPUT_SIZE 256
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int advIndex = 0;
		int value = atoi(&input[advIndex]);
		addVector(&inputVector, value);
		while (isdigit(input[advIndex])) { advIndex++; }
		while (!isdigit(input[advIndex])) { advIndex++; }
		value = atoi(&input[advIndex]);
		addVector(&inputVector, value);
	}

	fclose(filePointer);

	int numColumns = 71;
	int numRows = numColumns;

	int numToSimulate = 1024;

	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	memset(textGrid, '.', sizeof(char) * numColumns * numRows);
	for (int moveIndex = 0; moveIndex < numToSimulate; moveIndex++)
	{
		int x = inputVector.data[moveIndex * 2];
		int y = inputVector.data[moveIndex * 2 + 1];
		textGrid[y * numColumns + x] = '#';
	}

	for (int y = 0; y < numRows; y++)
	{
		for (int x = 0; x < numColumns; x++)
		{
			printf("%c", textGrid[y * numColumns + x]);
		}
		printf("\n");
	}
	printf("\n");

	int steps = aStar(textGrid, numColumns, numRows);
	printf("%d\n", steps);
}