#include <linux/limits.h>
#include <unistd.h>

#include <stdio.h>

#include <openssl/sha.h>

#define CHUNCK (1024)

int main()
{
    SHA512_CTX ctx;
    SHA512_Init(&ctx);

    char data[CHUNCK];
    int res = 0;
    while ((res = read(STDIN_FILENO, data, sizeof(data))) > 0)
        SHA512_Update(&ctx, data, res);

    unsigned char result[SHA512_DIGEST_LENGTH];
    SHA512_Final(result, &ctx);

    printf("0x");
    for (size_t i = 0; i < sizeof(result); ++i)
        printf("%02x", result[i]);

    return 0;
}