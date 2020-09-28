#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

static const int BUFFER_SIZE = 1024;

int main(int argc, char* argv[]) {
    if (argc != 4) return 3;

    const int infd = open(argv[1], O_RDONLY);
    if (infd == -1) return 1;

    const int numbersfd = open(argv[2], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (numbersfd == -1) return 2;

    const int otherfd = open(argv[3], O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
    if (otherfd == -1) return 2;

    char buffer[BUFFER_SIZE];
    char numbers[BUFFER_SIZE];
    char other[BUFFER_SIZE];

    int readed = 0;
    while ((readed = read(infd, buffer, sizeof(buffer))) > 0) {
        int numbers_count = 0;
        int other_count = 0;

        for (int i = 0; i < readed; ++i)
            if (isdigit(buffer[i]))
                numbers[numbers_count++] = buffer[i];
            else
                other[other_count++] = buffer[i];

        int writed = 0;
        while ((writed = write(numbersfd, numbers + writed, numbers_count - writed)) > 0) {}
        if (writed == -1) return 3;

        writed = 0;
        while ((writed = write(otherfd, other + writed, other_count - writed)) > 0) {}
        if (writed == -1) return 3;
    }

    close(infd);
    close(numbersfd);
    close(otherfd);

    if (readed == -1) return 3;

    return 0;
}
