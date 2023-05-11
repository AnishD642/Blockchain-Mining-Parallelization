#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

// Define the difficulty level of the mining algorithm
#define DIFFICULTY_LEVEL 4

// Define the maximum nonce value
#define MAX_NONCE 101

// Define the block structure
typedef struct
{
    uint32_t index;
    uint32_t timestamp;
    char data[256];
    uint32_t previous_hash;
    uint32_t hash;
} Block;

// Define a function to calculate the hash of a block
uint32_t calculate_hash(Block *block)
{
    char block_data[1024];
    sprintf(block_data, "%d%d%s%d", block->index, block->timestamp, block->data, block->previous_hash);

    uint32_t hash = 0;
    for (int i = 0; i < strlen(block_data); i++)
    {
        hash += block_data[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
}

int main()
{
    // Create a new block
    Block block;
    block.index = 1;
    block.timestamp = 123456789;
    strcpy(block.data, "Hello, world!");
    block.previous_hash = 0;

    // Set up the mining loop
    uint32_t nonce = 0;
    uint32_t hash = 0;
    int mining_complete = 0;

    clock_t start_time = clock();

    for (nonce = 0; nonce < MAX_NONCE; nonce++)
    {
        if (nonce == mining_complete)
        {
            continue;
        }
        block.hash = calculate_hash(&block);
        if (block.hash % (1 << (32 - DIFFICULTY_LEVEL)) != 0)
        {
            {
                printf("Block mined: %d\n", nonce);
                mining_complete = 1;
            }
        }
    }

    clock_t end_time = clock();
    double elapsed_time = (double)(end_time - start_time) / CLOCKS_PER_SEC;

    printf("Block Mining time taken: %f seconds\n", elapsed_time);

    return 0;
}