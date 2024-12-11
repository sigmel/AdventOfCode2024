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

	struct Vector antennaTypes;
	initVector(&antennaTypes);

	FILE* filePointer = fopen("puzzle.txt", "r");

	// start by reading the whole file into memory
	int value = 0;
	do {
		value = fgetc(filePointer);
		addVector(&inputVector, value);

		// create a list of our unique antenna types
		if (isalnum(value))
		{
			int found = 0;
			for (int antennaIndex = 0; antennaIndex < antennaTypes.count; antennaIndex++)
			{
				if (antennaTypes.data[antennaIndex] == value)
				{
					found = 1;
					break;
				}
			}
			if (!found)
			{
				addVector(&antennaTypes, value);
			}
		}
	} while (value != EOF);
	inputVector.data[inputVector.count - 1] = '\n'; // make the EOF a newline as well to keep it consistent

	fclose(filePointer);

	// determine the number of columns
	int numColumns = 0;
	while (inputVector.data[numColumns] != '\n') numColumns++;

	int numRows = inputVector.count / (numColumns + 1); // add 1 since it'll include the newlines currently

	// we store the x,y locations of antennas as char, so make sure they meet our limits
	assert(numColumns < 128);
	assert(numRows < 128);

	// create and fill our "2D" array
	char* textGrid = malloc(sizeof(char) * numColumns * numRows);
	for (int srcIndex = 0, dstIndex = 0; srcIndex < inputVector.count; srcIndex += numColumns + 1, dstIndex += numColumns)
	{
		memcpy(&textGrid[dstIndex], &inputVector.data[srcIndex], sizeof(char) * numColumns);
	}

	struct Vector* antennaLocations = malloc(sizeof(struct Vector) * antennaTypes.count);
	for (int antennaIndex = 0; antennaIndex < antennaTypes.count; antennaIndex++)
	{
		initVector(&antennaLocations[antennaIndex]);
	}
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (isalnum(textGrid[searchIndex]))
		{
			for (int antennaIndex = 0; antennaIndex < antennaTypes.count; antennaIndex++)
			{
				if (antennaTypes.data[antennaIndex] == textGrid[searchIndex])
				{
					// since our vectors are char, convert the coords to x,y
					addVector(&antennaLocations[antennaIndex], searchIndex % numColumns);
					addVector(&antennaLocations[antennaIndex], searchIndex / numColumns);
				}
			}
		}
	}

	// for each antenna type, we want to measure the distance between it and all other antennas of that type
	// then we'll find our "antinodes" which appear to be that distance on either side
	// finally need to check that these are valid locations on the map
	// to keep track of all the unique locatoins of antinodes, we can write them to a new grid and then count the locations
	char* antinodeGrid = malloc(sizeof(char) * numColumns * numRows);
	memset(antinodeGrid, '.', sizeof(char) * numColumns * numRows);

	for (int antennaIndex = 0; antennaIndex < antennaTypes.count; antennaIndex++)
	{
		for (int antennaLocationIndex = 0; antennaLocationIndex < antennaLocations[antennaIndex].count - 2; antennaLocationIndex += 2)
		{
			int c0 = antennaLocations[antennaIndex].data[antennaLocationIndex];
			int r0 = antennaLocations[antennaIndex].data[antennaLocationIndex + 1];

			for (int antennaCompareIndex = antennaLocationIndex + 2; antennaCompareIndex < antennaLocations[antennaIndex].count; antennaCompareIndex += 2)
			{
				int c1 = antennaLocations[antennaIndex].data[antennaCompareIndex];
				int r1 = antennaLocations[antennaIndex].data[antennaCompareIndex + 1];

				// set the harmonics at the same location as the antennas
				antinodeGrid[r0 * numRows + c0] = '#';
				antinodeGrid[r1 * numRows + c1] = '#';

				int dc = c1 - c0;
				int dr = r1 - r0;

				int ac0 = c0 - dc;
				int ar0 = r0 - dr;

				int ac1 = c1 + dc;
				int ar1 = r1 + dr;

				while (isValidLocation(ac0, ar0, numColumns, numRows))
				{
					antinodeGrid[ar0 * numRows + ac0] = '#';
					ac0 -= dc;
					ar0 -= dr;
				}

				while (isValidLocation(ac1, ar1, numColumns, numRows))
				{
					antinodeGrid[ar1 * numRows + ac1] = '#';
					ac1 += dc;
					ar1 += dr;
				}
			}
		}
	}

	int total = 0;
	for (int searchIndex = 0; searchIndex < numColumns * numRows; searchIndex++)
	{
		if (antinodeGrid[searchIndex] == '#')
		{
			total++;
		}
	}

	printf("%d\n", total);
}