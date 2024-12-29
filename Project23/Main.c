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
struct SetBucket
{
	char value[NUM_HASHES_BUCKET][128];
};

#define NUM_HASHES_SET (1024) // let's just have a lot since we don't want to allow collisions
struct HashSet
{
	struct SetBucket buckets[NUM_HASHES_SET];
};

void initHashSet(struct HashSet* hashSet)
{
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_SET; bucketIndex++)
	{
		memset(hashSet->buckets[bucketIndex].value, 0, sizeof(hashSet->buckets[bucketIndex].value));
	}
}

void addHashSet(struct HashSet* hashSet, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_SET;
	int added = 0;
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashSet->buckets[bucket].value[bucketIndex][0] == 0) // we need a valid non-zero pattern, so can use this to see if this is set
		{
			strcpy(hashSet->buckets[bucket].value[bucketIndex], str);
			added = 1;
			break;
		}
	}

	assert(added);
}

int hasValueHashSet(const struct HashSet* hashSet, const char* str)
{
	unsigned int hash = FNV(str);
	int bucket = hash % NUM_HASHES_SET;
	for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
	{
		if (hashSet->buckets[bucket].value[bucketIndex][0] == 0) // we need a valid non-zero pattern, so can use this to see if this is set
		{
			break;
		}
		if (strcmp(hashSet->buckets[bucket].value[bucketIndex], str) == 0)
		{
			return 1;
		}
	}
	return 0;
}

struct HashBucket
{
	unsigned int hashes[NUM_HASHES_BUCKET];
	char pattern[NUM_HASHES_BUCKET][128];
	struct Vector value[NUM_HASHES_BUCKET];
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

void addHashTable(struct HashTable* hashTable, const char* str, char c0, char c1)
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
			if (hashTable->buckets[bucket].value[bucketIndex].data == NULL)
			{
				initVector(&hashTable->buckets[bucket].value[bucketIndex]);
			}
			else if (hashTable->buckets[bucket].value[bucketIndex].data[hashTable->buckets[bucket].value[bucketIndex].count - 1] == 0)
			{
				hashTable->buckets[bucket].value[bucketIndex].data[hashTable->buckets[bucket].value[bucketIndex].count - 1] = ','; // change our null terminator to a comma since we're adding another
			}
			addVector(&hashTable->buckets[bucket].value[bucketIndex], c0);
			addVector(&hashTable->buckets[bucket].value[bucketIndex], c1);
			addVector(&hashTable->buckets[bucket].value[bucketIndex], 0); // null terminate our string
			added = 1;
			break;
		}
	}
	assert(added);
}

struct Vector* hasPatternHashTable(struct HashTable* hashTable, const char* str)
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
			return &hashTable->buckets[bucket].value[bucketIndex];
		}
	}

	return NULL;
}

#define NUM_GROUPS 256
struct ComputerGroup
{
	char computerName[3];
	int numGroups;
	struct Vector group[NUM_GROUPS]; // will contain the names of the computers with no separators
};

#define MAX_COMPUTERS 1024

void initComputerGroup(struct ComputerGroup* group, char* name)
{
	group->computerName[0] = name[0];
	group->computerName[1] = name[1];
	group->computerName[2] = 0;
	group->numGroups = 0;
}

int canAddComputerToGroup(struct Vector* group, char* name, struct HashTable* connectionTable)
{
	// need to make sure that this can connect to all other computers to add
	for (int computerIndex = 0; computerIndex < group->count; computerIndex += 2)
	{
		char c0[3];
		c0[0] = group->data[computerIndex];
		c0[1] = group->data[computerIndex + 1];
		c0[2] = 0;
		struct Vector* c0Connections = hasPatternHashTable(connectionTable, c0);
		if (strstr(c0Connections->data, name) == NULL)
		{
			return 0;
		}
	}
	return 1;
}

void addComputerToComputerGroup(struct ComputerGroup* group, char* name, struct HashTable* connectionTable)
{
	assert(group->numGroups < NUM_GROUPS);
	for (int groupIndex = 0; groupIndex < group->numGroups; groupIndex++)
	{
		if (canAddComputerToGroup(&group->group[groupIndex], name, connectionTable))
		{
			addVector(&group->group[groupIndex], name[0]);
			addVector(&group->group[groupIndex], name[1]);
		}
	}

	// will add each entry to its own group since we can't predict what the groups will be ahead of time
	initVector(&group->group[group->numGroups]);
	addVector(&group->group[group->numGroups], name[0]);
	addVector(&group->group[group->numGroups], name[1]);
	group->numGroups++;
}

void alphaSortComputerNames(char* computerNames, int numNames)
{
	// just bubble sort these, they will be few in number
	for (int i0 = 0; i0 < (numNames - 1) * 3; i0 += 3)
	{
		for (int i1 = i0 + 3; i1 < numNames * 3; i1 += 3)
		{
			if (strcmp(&computerNames[i0], &computerNames[i1]) > 0)
			{
				char temp[2];
				temp[0] = computerNames[i0];
				temp[1] = computerNames[i0 + 1];
				computerNames[i0] = computerNames[i1];
				computerNames[i0 + 1] = computerNames[i1 + 1];
				computerNames[i1] = temp[0];
				computerNames[i1 + 1] = temp[1];
			}
		}
	}
}

// we'll store all the connections between computers in a hash table
// the value will be a char vector of all the computers it can connect to
// I'm hoping that there isn't input where it is the same item but reversed since I'm not checking for that
void main()
{
	struct HashTable* connectionTable = malloc(sizeof(struct HashTable));
	initHashTable(connectionTable);

	FILE* filePointer = fopen("puzzle.txt", "r");

#define INPUT_SIZE 4096
	char input[INPUT_SIZE];
	while (fgets(input, INPUT_SIZE, filePointer))
	{
		char c0[3];
		c0[0] = input[0];
		c0[1] = input[1];
		c0[2] = 0;

		char c1[3];
		c1[0] = input[3];
		c1[1] = input[4];
		c1[2] = 0;

		assert(strcmp(c0, c1) != 0); // make sure we don't have two of the same computer (probably not happening, but just to be sure)

		// add each connection possibility
		addHashTable(connectionTable, c0, input[3], input[4]);
		addHashTable(connectionTable, c1, input[0], input[1]);
	}

	// now find all the groups
	// this will go through each computer, so it is definitely doing redundant work,
	// but I'm hoping it isn't bad enough that I have to optimize it
	int numComputers = 0;
	struct ComputerGroup* connectionGroups = malloc(sizeof(struct ComputerGroup) * MAX_COMPUTERS);

	for (int tableIndex = 0; tableIndex < NUM_HASHES_TABLE; tableIndex++)
	{
		for (int bucketIndex = 0; bucketIndex < NUM_HASHES_BUCKET; bucketIndex++)
		{
			if (connectionTable->buckets[tableIndex].pattern[bucketIndex][0] == 0)
			{
				// we've reached the end of this bucket
				break;
			}

			assert(numComputers < MAX_COMPUTERS);
			initComputerGroup(&connectionGroups[numComputers], connectionTable->buckets[tableIndex].pattern[bucketIndex]);

			// now make any eligible groups
			char* computerName = connectionTable->buckets[tableIndex].pattern[bucketIndex];
			struct Vector* computerVector = &connectionTable->buckets[tableIndex].value[bucketIndex];
			for (int c0Index = 0; c0Index < computerVector->count - 3; c0Index += 3)
			{
				char c0[3];
				c0[0] = computerVector->data[c0Index];
				c0[1] = computerVector->data[c0Index + 1];
				c0[2] = 0;

				addComputerToComputerGroup(&connectionGroups[numComputers], c0, connectionTable);
			}
			numComputers++;
		}
	}

	// find the largest group
	int largestNum = 0;
	char* largestName = NULL;
	struct Vector* largest = NULL;
	for (int computerIndex = 0; computerIndex < numComputers; computerIndex++)
	{
		struct ComputerGroup* group = &connectionGroups[computerIndex];
		for (int groupIndex = 0; groupIndex < group->numGroups; groupIndex++)
		{
			struct Vector* groupVector = &group->group[groupIndex];
			if (groupVector->count > largestNum)
			{
				largestNum = groupVector->count;
				largestName = group->computerName;
				largest = groupVector;
			}
		}
	}

	// now format it correctly
	char computerNames[MAX_COMPUTERS * 3];
	computerNames[0] = largestName[0];
	computerNames[1] = largestName[1];
	computerNames[2] = 0;
	int nameIndex = 3;
	for (int computerIndex = 0; computerIndex < largest->count; computerIndex += 2)
	{
		computerNames[nameIndex] = largest->data[computerIndex];
		computerNames[nameIndex + 1] = largest->data[computerIndex + 1];
		computerNames[nameIndex + 2] = 0;
		nameIndex += 3;
	}
	alphaSortComputerNames(computerNames, (largestNum / 2) + 1);

	// print out all triads
	assert(nameIndex < 256);
	char key[256] = { 0 };
	for (int charIndex = 0; charIndex < nameIndex - 1; charIndex++)
	{
		if (computerNames[charIndex] == 0)
		{
			key[charIndex] = ',';
		}
		else
		{
			key[charIndex] = computerNames[charIndex];
		}
	}
	printf("%s\n", key);
}