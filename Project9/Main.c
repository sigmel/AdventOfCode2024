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
	FILE* filePointer = fopen("puzzle.txt", "r");

	// first, figure out the total size of our disk
	size_t totalSize = 0;
	int maxFileID = 0;
	char value = 0;
	size_t toggle = 0;
	while (1)
	{
		value = fgetc(filePointer);
		if (value == EOF)
		{
			break;
		}

		value -= '0';
		totalSize += value;
		if (toggle % 2 == 0)
		{
			maxFileID++;
			assert(maxFileID > 0 && maxFileID < INT_MAX); // verify this accepts the puzzle input
		}
		toggle++;
	} while (value != EOF);
	
	int* disk = malloc(sizeof(int) * totalSize);

	fseek(filePointer, 0, SEEK_SET);

	// now fill it out
	size_t cursor = 0;
	int fileID = 0;
	toggle = 0;
	while (1)
	{
		value = fgetc(filePointer);
		if (value == EOF)
		{
			break;
		}

		value -= '0';
		if (toggle % 2 == 0)
		{
			for (int fileIndex = 0; fileIndex < value; fileIndex++)
			{
				disk[cursor + fileIndex] = fileID;
			}
			fileID++;
		}
		else
		{
			for (int fileIndex = 0; fileIndex < value; fileIndex++)
			{
				disk[cursor + fileIndex] = -1;
			}
		}
		cursor += value;
		toggle++;
	}

	// now compact using a begin and end pointer
	size_t end = totalSize - 1;
	int currentFileID = maxFileID - 1;
	while (currentFileID > 0) // can't ever move file 0
	{
		// find the size we need
		size_t sizeNeeded = 0;
		while (disk[end] != currentFileID) { end--; }
		while (disk[end] == currentFileID && end >= 0) { sizeNeeded++; end--; }

		// find a spot this could fit in
		size_t begin = 0; // could save off where the first free space starts, but meh
		while (begin < end)
		{
			size_t freeSpace = 0;
			while (disk[begin] != -1) { begin++; }
			size_t freeSpaceStart = begin;
			while (disk[begin] == -1 && begin <= end) { begin++; freeSpace++; }
			if (freeSpace >= sizeNeeded)
			{
				for (int copyIndex = 0; copyIndex < sizeNeeded; copyIndex++)
				{
					disk[freeSpaceStart + copyIndex] = currentFileID;
					disk[end + 1 + copyIndex] = -1;
				}
				break;
			}
		}

		currentFileID--;
	}

	// now compute the checksum
	long long checksum = 0;
	cursor = 0;
	while (cursor < totalSize)
	{
		if (disk[cursor] > 0)
		{
			checksum += cursor * disk[cursor];
		}
		cursor++;
	}
	printf("%lld\n", checksum);
}