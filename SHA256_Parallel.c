#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include <stdint.h>

#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n) ((x) >> (n))
#define CH(x, y, z) (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define SIGMA0(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define SIGMA1(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define DELTA0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define DELTA1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))

static const uint32_t K[] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5,
    0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3,
    0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
    0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7,
    0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13,
    0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3,
    0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5,
    0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
    0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

void sha256_transform(uint32_t *state, const uint32_t *block)
{
    uint32_t W[64];
    uint32_t a, b, c, d, e, f, g, h, T1, T2;
    int i;

    for (i = 0; i < 16; i++)
    {
        W[i] = block[i];
    }
    for (i = 16; i < 64; i++)
    {
        W[i] = DELTA1(W[i - 2]) + W[i - 7] + DELTA0(W[i - 15]) + W[i - 16];
    }

    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    f = state[5];
    g = state[6];
    h = state[7];

#pragma omp parallel for private(T1, T2)
    for (i = 0; i < 64; i++)
    {
        T1 = h + SIGMA1(e) + CH(e, f, g) + K[i] + W[i];
        T2 = SIGMA0(a) + MAJ(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + T1;
        d = c;
        c = b;
        b = a;
        a = T1 + T2;
    }

    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    state[5] += f;
    state[6] += g;
    state[7] += h;
}

void sha256_hash(const unsigned char *message, size_t len, unsigned char *digest)
{
    uint32_t state[8] = {
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
        0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
    uint32_t block[16];
    size_t i, j;

    for (i = 0; i + 64 <= len; i += 64)
    {
        for (j = 0; j < 16; j++)
        {
            block[j] = (uint32_t)(message[i + j * 4 + 0]) << 24 |
                       (uint32_t)(message[i + j * 4 + 1]) << 16 |
                       (uint32_t)(message[i + j * 4 + 2]) << 8 |
                       (uint32_t)(message[i + j * 4 + 3]) << 0;
        }
        sha256_transform(state, block);
    }
    memset(block, 0, sizeof(block));
    for (j = 0; i + j < len; j++)
    {
        block[j / 4] |= (uint32_t)(message[i + j]) << (24 - j % 4 * 8);
    }
    block[j / 4] |= (uint32_t)0x80 << (24 - j % 4 * 8);
    if (j >= 56)
    {
        sha256_transform(state, block);
        memset(block, 0, sizeof(block));
    }
    block[15] = (uint32_t)len << 3;
    sha256_transform(state, block);

    for (i = 0; i < 8; i++)
    {
        digest[i * 4 + 0] = (unsigned char)(state[i] >> 24);
        digest[i * 4 + 1] = (unsigned char)(state[i] >> 16);
        digest[i * 4 + 2] = (unsigned char)(state[i] >> 8);
        digest[i * 4 + 3] = (unsigned char)(state[i] >> 0);
    }
}
int main()
{
    const char *message = "Okay";
    unsigned char digest[32];
    double start_time, end_time;

    start_time = omp_get_wtime();
    sha256_hash((unsigned char *)message, strlen(message), digest);

    int i;
    printf("The plaintext is: %s",message);
    printf("\n");
    printf("The hashed digest is: ");
    for (i = 0; i < 32; i++)
    {
        printf("%02x", digest[i]);
    }
    printf("\n");

    end_time = omp_get_wtime();

    printf("Time taken: %f seconds\n", end_time - start_time);

    return 0;
}
