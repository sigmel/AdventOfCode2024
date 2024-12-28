#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define VECTOR_DATA long long
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
	long long value[NUM_HASHES_BUCKET];
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
		memset(hashTable->buckets[bucketIndex].value, 0, sizeof(hashTable->buckets[bucketIndex].value));
	}
}

void addHashTable(struct HashTable* hashTable, const char* str, long long value)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	int added = 0;
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0 || // we need a valid non-zero pattern, so can use this to see if this is set
			(hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0))
		{
			assert(strlen(str) < 127);
			hashTable->buckets[bucket].hashes[bucketIndex] = hash;
			strcpy(hashTable->buckets[bucket].pattern[bucketIndex], str);
			hashTable->buckets[bucket].value[bucketIndex] = value;
			added = 1;
			break;
		}
	}
	assert(added);
}

void hasPatternHashTable(struct HashTable* hashTable, const char* str, long long* value)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	*value = 0;

	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0)
		{
			break; // this is empty, so it can't be here
		}
		if (hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0) // strcmp might be unnecessary, but to be safe
		{
			*value = hashTable->buckets[bucket].value[bucketIndex];
			return;
		}
	}
}

long long mix(long long secret, long long value)
{
	// To mix a value into the secret number, calculate the bitwise XOR of the given value and the secret number.
	// Then, the secret number becomes the result of that operation.
	// (If the secret number is 42 and you were to mix 15 into the secret number, the secret number would become 37.)
	return secret ^ value;
}

long long prune(long long number)
{
	// To prune the secret number, calculate the value of the secret number modulo 16777216.
	// Then, the secret number becomes the result of that operation.
	// (If the secret number is 100000000 and you were to prune the secret number, the secret number would become 16113920.)
	return number % 16777216;
}

// We'll just store all our changes in a hash table by change sequences and keep track of all the possibilities
// can just keep track of the highest value to determine the answer
void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	struct HashTable* bananaTable;
	bananaTable = malloc(sizeof(struct HashTable));
	initHashTable(bananaTable);

	long long total = 0;

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		// really just need a circular buffer but this will suffice
		struct Vector changeVector;
		initVector(&changeVector);

		struct HashTable* sequenceTable;
		sequenceTable = malloc(sizeof(struct HashTable));
		initHashTable(sequenceTable);

		long long secret = atoll(input);

		const int iterationNum = 2000;

		addVector(&changeVector, secret % 10);

		for (int iterateIndex = 0; iterateIndex < iterationNum; iterateIndex++)
		{
			// Calculate the result of multiplying the secret number by 64. Then, mix this result into the secret number. Finally, prune the secret number.
			long long result = secret * 64;
			secret = mix(secret, result);
			secret = prune(secret);

			// Calculate the result of dividing the secret number by 32. Round the result down to the nearest integer.Then, mix this result into the secret number.Finally, prune the secret number.
			result = secret / 32;
			secret = mix(secret, result);
			secret = prune(secret);

			// Calculate the result of multiplying the secret number by 2048. Then, mix this result into the secret number.Finally, prune the secret number.
			result = secret * 2048;
			secret = mix(secret, result);
			secret = prune(secret);

			long long numBananas = secret % 10;
			addVector(&changeVector, numBananas);

			if (changeVector.count > 4)
			{
				long long change3 = changeVector.data[iterateIndex + 1] - changeVector.data[iterateIndex];
				long long change2 = changeVector.data[iterateIndex] - changeVector.data[iterateIndex - 1];
				long long change1 = changeVector.data[iterateIndex - 1] - changeVector.data[iterateIndex - 2];
				long long change0 = changeVector.data[iterateIndex - 2] - changeVector.data[iterateIndex - 3];

				char key[128];
				sprintf(key, "%lld,%lld,%lld,%lld", change0, change1, change2, change3);

				long long value = 0;
				hasPatternHashTable(sequenceTable, key, &value);

				if (value == 0)
				{
					addHashTable(sequenceTable, key, numBananas);
				}
			}
		}

		// combine our results to find the total
		for (int tableIndex = 0; tableIndex < NUM_HASHES_TABLE; tableIndex++)
		{
			for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
			{
				if (sequenceTable->buckets[tableIndex].pattern[bucketIndex][0] == 0)
				{
					// we've reached the end of this bucket
					break;
				}

				long long value = 0;
				hasPatternHashTable(bananaTable, sequenceTable->buckets[tableIndex].pattern[bucketIndex], &value);

				long long combined = value + sequenceTable->buckets[tableIndex].value[bucketIndex];
				addHashTable(bananaTable, sequenceTable->buckets[tableIndex].pattern[bucketIndex], combined);
				if (combined > total)
				{
					total = combined;
				}
			}
		}

		free(sequenceTable);
		freeVector(&changeVector);
	}
	fclose(filePointer);

	printf("%lld\n", total);
}