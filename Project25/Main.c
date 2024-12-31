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

void initLockKey(char pattern[5][5], struct Vector* lockKey, int* lockKeyNum)
{
	initVector(&lockKey[*lockKeyNum]);
	for (int x = 0; x < 5; x++)
	{
		int numHashes = 0;
		for (int y = 0; y < 5; y++)
		{
			if (pattern[y][x] == '#')
			{
				numHashes++;
			}
		}
		addVector(&lockKey[*lockKeyNum], numHashes);
	}
	(*lockKeyNum)++;
}

void main()
{
#define MAX_LOCKS 256
	struct Vector locks[MAX_LOCKS];
	int numLocks = 0;

	struct Vector keys[MAX_LOCKS];
	int numKeys = 0;

	FILE* filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	int key = 0;
	int lock = 0;
	char pattern[5][5];
	int patternNum = 0;
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		if (key == 0 && lock == 0)
		{
			int numHashes = 0;
			while (input[numHashes] == '#') { numHashes++; }
			if (numHashes == 5)
			{
				lock = 1;
			}
			else
			{
				key = 1;
			}
		}
		else if (input[0] == '\n')
		{
			assert(lock || key);
			if (lock)
			{
				initLockKey(pattern, locks, &numLocks);
			}
			else if (key)
			{
				initLockKey(pattern, keys, &numKeys);
			}

			patternNum = 0;
			lock = 0;
			key = 0;
		}
		else if (patternNum < 5)
		{
			memcpy(&pattern[patternNum], input, sizeof(char) * 5);
			patternNum++;
		}
	}
	// add the final one
	assert(lock || key);
	if (lock)
	{
		initLockKey(pattern, locks, &numLocks);
	}
	else if (key)
	{
		initLockKey(pattern, keys, &numKeys);
	}

	// test the keys against all locks
	int total = 0;
	for (int lockIndex = 0; lockIndex < numLocks; lockIndex++)
	{
		struct Vector* lock = &locks[lockIndex];
		for (int keyIndex = 0; keyIndex < numKeys; keyIndex++)
		{
			int works = 1;
			struct Vector* key = &keys[keyIndex];
			for (int columnIndex = 0; columnIndex < 5; columnIndex++)
			{
				if (lock->data[columnIndex] + key->data[columnIndex] > 5)
				{
					works = 0;
					break;
				}
			}
			if (works)
			{
				total++;
			}
		}
	}

	printf("%d\n", total);
}