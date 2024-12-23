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

#define NUM_HASHES_SET (256 * 1024) // let's just have a lot since we don't want to allow collisions
struct HashSet
{
	char* value[NUM_HASHES_SET];
};

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

void initHashSet(struct HashSet* hashSet)
{
	memset(hashSet->value, 0, sizeof(hashSet->value));
}

void addHashSet(struct HashSet* hashSet, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_SET;
	assert(hashSet->value[bucket] == 0);

	size_t length = strlen(str);
	length++; // add null char
	hashSet->value[bucket] = malloc(sizeof(char) * length);
	strcpy(hashSet->value[bucket], str);
}

int hasValueHashSet(const struct HashSet* hashSet, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_SET;
	if (hashSet->value[bucket] == 0 || strcmp(hashSet->value[bucket], str) != 0) // need to verify the string matches
	{
		return 0;
	}
	return 1;
}

#define NUM_HASHES_BUCKET (256)
struct HashBucket
{
	unsigned int hashes[NUM_HASHES_BUCKET];
	char pattern[NUM_HASHES_BUCKET][128];
	long long matches[NUM_HASHES_BUCKET];
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
		memset(hashTable->buckets[bucketIndex].matches, 0, sizeof(hashTable->buckets[bucketIndex].matches));
	}
}

void addHashTable(struct HashTable* hashTable, const char* str, long long matches)
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
			hashTable->buckets[bucket].matches[bucketIndex] = matches;
			added = 1;
			break;
		}
	}
	assert(added);
}

long long hasPatternHashTable(struct HashTable* hashTable, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_BUCKET;

	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashTable->buckets[bucket].pattern[bucketIndex][0] == 0)
		{
			break; // this is empty, so it can't be here
		}
		if (hashTable->buckets[bucket].hashes[bucketIndex] == hash && strcmp(hashTable->buckets[bucket].pattern[bucketIndex], str) == 0) // strcmp might be unnecessary, but to be safe
		{
			return hashTable->buckets[bucket].matches[bucketIndex];
		}
	}

	return 0;
}

void findPattern(const struct HashSet* patternHashSet, int maxPatternLength, const char* pattern, long long* possiblePatterns, struct HashTable* cacheTable)
{
	char scratch[16]; // should be enough for any of our patterns
	memset(scratch, 0, sizeof(scratch));
	for (int patternIndex = 0; patternIndex < maxPatternLength && pattern[patternIndex] != 0; patternIndex++)
	{
		assert(patternIndex < 16);
		scratch[patternIndex] = pattern[patternIndex];
		if (hasValueHashSet(patternHashSet, scratch))
		{
			if (pattern[patternIndex + 1] == 0)
			{
				(*possiblePatterns)++;
			}
			else
			{
				long long numMatches = 0;
				if (cacheTable != NULL)
				{
					numMatches = hasPatternHashTable(cacheTable, &pattern[patternIndex + 1]);
				}
				if (numMatches > 0)
				{
					(*possiblePatterns) += numMatches;
				}
				else
				{
					findPattern(patternHashSet, maxPatternLength, &pattern[patternIndex + 1], possiblePatterns, cacheTable);
				}
			}
		}
	}
}

void findAllPatterns(const struct HashSet* patternHashSet, int maxPatternLength, char* pattern, long long* possiblePatterns, struct HashTable* cacheTable)
{
	// initialize our cache
	int patternLength = (int)strlen(pattern);
	for (int patternIndex = 1; patternIndex < patternLength; patternIndex++) // don't need to cache any single letters
	{
		const char* checkPattern = &pattern[patternLength - patternIndex - 1];
		if (hasPatternHashTable(cacheTable, checkPattern) <= 0) // make sure this isn't already in our cache
		{
			long long cachePatternNum = 0;
			findPattern(patternHashSet, maxPatternLength, checkPattern, &cachePatternNum, cacheTable);
			if (cachePatternNum > 0)
			{
				addHashTable(cacheTable, checkPattern, cachePatternNum);
			}
		}
	}

	long long numTotalPatterns = hasPatternHashTable(cacheTable, pattern);
	printf("%lld - %s\n", numTotalPatterns, pattern);

	// now add the number of possibilities
	(*possiblePatterns) += numTotalPatterns;
}

void main()
{
	FILE* filePointer = fopen("puzzle.txt", "r");

	struct HashSet* patternHashSet;
	patternHashSet = malloc(sizeof(struct HashSet));
	initHashSet(patternHashSet);

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];

	// get our patterns
	fgets(input, INPUT_SIZE, filePointer);
	int advIndex = 0;
	int startString = 0;
	int maxPatternLength = 0;
	while (input[advIndex] != '\n')
	{
		if (input[advIndex] == ',')
		{
			input[advIndex] = 0; // null terminate
			int length = (int)strlen(&input[startString]);
			if (length > maxPatternLength)
			{
				maxPatternLength = length;
			}
			addHashSet(patternHashSet, &input[startString]);

			startString = advIndex;
		}
		else if (!isalpha(input[advIndex]))
		{
			startString = advIndex + 1;
		}
		advIndex++;
	}
	input[advIndex] = 0; // null terminate
	addHashSet(patternHashSet, &input[startString]); // add the last one

	fgets(input, INPUT_SIZE, filePointer); // newline

	// we'll keep a cache of all results so that we can use past calculations to skip future ones
	// we also need to iterate backwards so that we know we can successfully complete the pattern with each entry
	struct HashTable* cacheTable = malloc(sizeof(struct HashTable));
	initHashTable(cacheTable);

	long long possiblePatterns = 0;
	int numPossible = 0;
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		int length = (int)strlen(input);
		if (input[length - 1] == '\n')
		{
			input[length - 1] = 0; // remove the newline
			length--;
		}

		long long beforePossiblePatterns = possiblePatterns;
		findAllPatterns(patternHashSet, maxPatternLength, input, &possiblePatterns, cacheTable);
		if (possiblePatterns != beforePossiblePatterns)
		{
			numPossible++;
		}
	}

	printf("%lld\n", possiblePatterns);
}