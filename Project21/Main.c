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

// using FNV from https://stackoverflow.com/questions/7666509/hash-function-for-string
unsigned int FNV(const char* key)
{
	// See: https://github.com/aappleby/smhasher/blob/master/src/Hashes.cpp
	unsigned int h = 0x12345678L /* seed*/ ^ 2166136261UL;
	const char* data = (const char*)key;
	for (int i = 0; data[i] != '\0'; i++)
	{
		h ^= data[i];
		h *= 16777619;
	}
	return h;
}

#define NUM_HASHES_BUCKET (256)
struct HashBucket
{
	unsigned int hashes[NUM_HASHES_BUCKET];
	char pattern[NUM_HASHES_BUCKET][128];
	long long numMoves[NUM_HASHES_BUCKET];
};

#define NUM_HASHES_TABLE (1024)
struct HashTable
{
	struct HashBucket buckets[NUM_HASHES_TABLE];
};

void initHashTable(struct HashTable* hashTable)
{
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_TABLE; bucketIndex++)
	{
		memset(hashTable->buckets[bucketIndex].hashes, 0, sizeof(hashTable->buckets[bucketIndex].hashes));
		memset(hashTable->buckets[bucketIndex].pattern, 0, sizeof(hashTable->buckets[bucketIndex].pattern));
		memset(hashTable->buckets[bucketIndex].numMoves, 0, sizeof(hashTable->buckets[bucketIndex].numMoves));
	}
}

void addHashTable(struct HashTable* hashTable, const char* str, long long numMoves)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	int added = 0;
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0) // we need a valid non-zero pattern, so can use this to see if this is set
		{
			assert(strlen(str) < 127);
			hashTable->buckets[bucket].hashes[bucketIndex] = hash;
			strcpy(hashTable->buckets[bucket].pattern[bucketIndex], str);
			hashTable->buckets[bucket].numMoves[bucketIndex] = numMoves;
			added = 1;
			break;
		}
	}
	assert(added);
}

void hasPatternHashTable(struct HashTable* hashTable, const char* str, long long* numMoves)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	*numMoves = 0;

	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0)
		{
			break; // this is empty, so it can't be here
		}
		if (hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0) // strcmp might be unnecessary, but to be safe
		{
			*numMoves = hashTable->buckets[bucket].numMoves[bucketIndex];
			return;
		}
	}
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
	static const int keys[NUM_KEYS] = { '7', '8', '9', '4', '5', '6', '1', '2', '3', 'X', '0', 'A' };
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

#define MAX_SINGLE_MOVES 8
void moveHorizontal(int* x, int endX, int dX, int* numMoves, int* moves)
{
	while (*x != endX)
	{
		if (dX < 0) { moves[*numMoves] = '<'; (*numMoves)++; }
		else { moves[*numMoves] = '>'; (*numMoves)++; }
		*x += dX;
	}
	assert(*numMoves <= MAX_SINGLE_MOVES);
}

void moveVertical(int* y, int endY, int dY, int* numMoves, int* moves)
{
	while (*y != endY)
	{
		if (dY < 0) { moves[*numMoves] = '^'; (*numMoves)++; }
		else { moves[*numMoves] = 'v'; (*numMoves)++; }
		*y += dY;
	}
	assert(*numMoves <= MAX_SINGLE_MOVES);
}

// the basic approach is that we want to move left to right on the dir pad, because we end on A, so getting there from < sucks
void numericKeypad(struct NumericKeypad* numKeypad, int destination, int* numMoves, int* moves)
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
			moveHorizontal(&x, endX, dX, numMoves, moves);
			moveVertical(&y, endY, dY, numMoves, moves);
		}
		else
		{
			// otherwise go the other way
			moveVertical(&y, endY, dY, numMoves, moves);
			moveHorizontal(&x, endX, dX, numMoves, moves);
		}
	}

	// handle any down moves assuming we don't go through the illegal space
	if (dY > 0)
	{
		if (x != 0 || endY != numKeypad->numRows - 1) // make sure we won't hit the blank spot going down
		{
			moveVertical(&y, endY, dY, numMoves, moves);
			moveHorizontal(&x, endX, dX, numMoves, moves);
		}
		else
		{
			moveHorizontal(&x, endX, dX, numMoves, moves);
			moveVertical(&y, endY, dY, numMoves, moves);
		}
	}

	// deal with any leftover moves
	moveVertical(&y, endY, dY, numMoves, moves);
	moveHorizontal(&x, endX, dX, numMoves, moves);

	moves[*numMoves] = 'A'; (*numMoves)++;
	assert(*numMoves <= MAX_SINGLE_MOVES);

	numKeypad->current = end;
}

static int sKeyLookup[128];

void directionalKeypad(struct DirectionalKeypad* dirKeypads, int number, int destination, int depth, struct HashTable* cacheTable, long long* numMoves)
{
	int moveList[MAX_SINGLE_MOVES]; // should be plenty for a single move

	int end = sKeyLookup[destination];

	int cached = 1;

	char key[128] = { 0 };
	key[0] = (char)destination;
	for (int keyIndex = 1; keyIndex < depth + 2; keyIndex++)
	{
		key[keyIndex] = (char)dirKeypads[depth].keys[dirKeypads[depth - keyIndex + 1].current];
	}
	key[depth + 2] = number;
	long long moves = 0;
	hasPatternHashTable(cacheTable, key, &moves);

	if (moves <= 0)
	{
		cached = 0;

		int x = dirKeypads[depth].current % dirKeypads[depth].numColumns;
		int y = dirKeypads[depth].current / dirKeypads[depth].numColumns;
		int endX = end % dirKeypads[depth].numColumns;
		int endY = end / dirKeypads[depth].numColumns;
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

		int numCalculatedMoves = 0;

		// try to go left first, but we can't do that if we would cross the blank spot
		if (dX < 0)
		{
			if (y != 0 || endX != 0) // make sure we won't hit the blank spot going left
			{
				moveHorizontal(&x, endX, dX, &numCalculatedMoves, moveList);
				moveVertical(&y, endY, dY, &numCalculatedMoves, moveList);
			}
			else
			{
				// otherwise go the other way
				moveVertical(&y, endY, dY, &numCalculatedMoves, moveList);
				moveHorizontal(&x, endX, dX, &numCalculatedMoves, moveList);
			}
		}

		// handle any up moves assuming we don't go through the illegal space
		else if (dY < 0)
		{
			if (x != 0 || endY != 0) // make sure we won't hit the blank spot going down
			{
				moveVertical(&y, endY, dY, &numCalculatedMoves, moveList);
				moveHorizontal(&x, endX, dX, &numCalculatedMoves, moveList);
			}
			else
			{
				moveHorizontal(&x, endX, dX, &numCalculatedMoves, moveList);
				moveVertical(&y, endY, dY, &numCalculatedMoves, moveList);
			}
		}

		// deal with any leftover moves
		moveVertical(&y, endY, dY, &numCalculatedMoves, moveList);
		moveHorizontal(&x, endX, dX, &numCalculatedMoves, moveList);

		moveList[numCalculatedMoves] = 'A'; numCalculatedMoves++;
		assert(numCalculatedMoves <= MAX_SINGLE_MOVES);

		moves = numCalculatedMoves;
	}
	else
	{
		cached = 1;
	}

	dirKeypads[depth].current = end;
	if (depth > 0)
	{
		if (!cached)
		{
			long long cachedMoves = 0;
			for (int moveIndex = 0; moveIndex < moves; moveIndex++)
			{
				directionalKeypad(dirKeypads, number, moveList[moveIndex], depth - 1, cacheTable, &cachedMoves);
			}
			addHashTable(cacheTable, key, cachedMoves);
			*numMoves += cachedMoves;
		}
		else
		{
			*numMoves += moves;
		}
	}
	else
	{
		*numMoves += moves;
		if (!cached)
		{
			addHashTable(cacheTable, key, moves);
		}
	}
}

// we can utilize caching to speed this up similar to how we did before since the movement calculations will be consistent
// to make that work, we'll determine all the moves for a single digit at a time so that we can better reuse calculations
void main()
{
	// directional key lookup
	sKeyLookup['X'] = 0;
	sKeyLookup['^'] = 1;
	sKeyLookup['A'] = 2;
	sKeyLookup['<'] = 3;
	sKeyLookup['v'] = 4;
	sKeyLookup['>'] = 5;

	struct Vector inputVector;
	initVector(&inputVector);

	struct NumericKeypad numKeypad;
	initNumericKeypad(&numKeypad);

#define NUM_DIR_KEYPADS 25
	struct DirectionalKeypad dirKeypads[NUM_DIR_KEYPADS];
	for (int dirIndex = 0; dirIndex < NUM_DIR_KEYPADS; dirIndex++)
	{
		initDirectionKeypad(&dirKeypads[dirIndex]);
	}

	struct HashTable* cacheTable = malloc(sizeof(struct HashTable));
	initHashTable(cacheTable);

	FILE* filePointer = fopen("puzzle.txt", "r");

	long long totalComplexity = 0;

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		clearVector(&inputVector);

		size_t length = strlen(input);
		if (input[length - 1] == '\n')
		{
			input[length - 1] = 0;
			length--;
		}
		for (int charIndex = 0; charIndex < (int)length; charIndex++)
		{
			addVector(&inputVector, input[charIndex]);
		}

		// get the numeric code
		int numericCode = atoi(input);

		long long numMoves = 0;

		// determine our moves
		for (int inputIndex = 0; inputIndex < inputVector.count; inputIndex++)
		{
			int numPadMoves = 0;
			int moveList[MAX_SINGLE_MOVES];
			int initialKeySpot = numKeypad.keys[numKeypad.current];
			// first figure out what our moves need to be for the numeric keypad
			numericKeypad(&numKeypad, inputVector.data[inputIndex], &numPadMoves, moveList);

			// we can process these one move at a time
			for (int moveIndex = 0; moveIndex < numPadMoves; moveIndex++)
			{
				directionalKeypad(dirKeypads, initialKeySpot, moveList[moveIndex], NUM_DIR_KEYPADS - 1, cacheTable, &numMoves);
			}
		}

		// and determine the complexity
		long long complexity = numericCode * numMoves;
		totalComplexity += complexity;
	}

	printf("%lld\n", totalComplexity);
}