#include <stdio.h>

extern void normalize_path(char* path);
/*
int main(int argc, char* argv[]) {
    if (argc != 2) return 0;
    normalize_path(argv[1]);
    puts(argv[1]);
    return 0;
}
*/

void remove_subsequent(char* path) {
    char* carret = path;
    while (*path)
        if (*path == '/' && *(path + 1) == '/') ++path;
        else *(carret++) = *(path++);

    *carret = 0;
}

typedef struct {
    char* str;
    int len;
    int prev;
    int next;
} part;

char is_one_dot(const part* p) {
    return (p->len == 1) && (*(p->str) == '.');
}

char is_two_dots(const part* p) {
    return (p->len == 2) && (*(p->str) == '.') && (*(p->str + 1) == '.');
}

void remove_redundant(char* path) {
    char* output = path;

    char leading = 0;
    char ending = 0;

    if (*path == '/') {
        leading = 1;
        ++path;
    }

    part parts[512]; // MAX PARTS
    int count = 0;
    while (*path) {
        ending = 0;

        parts[count].str = path;
        while (*path && *path != '/')
            ++path;
        parts[count].len = path - parts[count].str;
        parts[count].prev = count - 1;
        parts[count].next = count + 1;

        //printf("%.*s\n", parts[count].len, parts[count].str);
        ++count;

        if (*path == '/') {
            ++path;
            ending = 1;
        }
    }

    int start = 0;
    int current = 0;
    while (current < count) {
        //printf("%d: %.*s\n", current, parts[current].len, parts[current].str);
        int prev = parts[current].prev;
        int next = parts[current].next;
        if (is_one_dot(parts + current)) {
            if (prev != -1) {
                parts[prev].next = next;
                if (next < count)
                    parts[next].prev = prev;
            }
            else {
                start = next;
                if (next < count)
                    parts[next].prev = -1;
            }
        }
        else if (is_two_dots(parts + current)) {
            if (prev != -1 && !is_two_dots(parts + prev)) {
                int prevprev = parts[prev].prev;
                if (prevprev != -1) {
                    parts[prevprev].next = next;
                    if (next < count)
                        parts[next].prev = prevprev;
                }
                else {
                    start = next;
                    if (next < count)
                        parts[next].prev = -1;
                }
            }
        }

        current = next;
    }

    if (leading)
        *(output++) = '/';

    current = start;
    while (current < count) {
        for (int i = 0; i < parts[current].len; ++i)
            *(output++) = *(parts[current].str++);
        *(output++) = '/';
        current = parts[current].next;
    }

    if (ending)
        *output = 0;
    else
        *(output - 1) = 0;
}

void normalize_path(char* path) {
    remove_subsequent(path);
    remove_redundant(path);
}
