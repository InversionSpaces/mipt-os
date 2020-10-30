#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <limits.h>
#include <unistd.h>

int main()
{
    char* script_path = 0;
    char* query_string = 0;

    char request_string[LINE_MAX + PATH_MAX] = "";
    char prot_version[LINE_MAX] = "";
    char host_name[HOST_NAME_MAX] = "";

    scanf("%*s");
    scanf("%s", request_string);

    char* question_mark = strchr(request_string, '?');
    script_path = request_string + 1; // WTF?
    if (question_mark == NULL)
        query_string = "";
    else {
        *question_mark = 0;
        query_string = question_mark + 1;
    }

    scanf("%s", prot_version);
    scanf("%*s");
    scanf("%s", host_name);

    if (access(script_path, F_OK) == -1) {
        printf("%s 404 ERROR\n\n", prot_version);
        return 0;
    }

    if (access(script_path, X_OK) == -1) {
        printf("%s 403 ERROR\n\n", prot_version);
        return 0;
    }

    char* script_name = strrchr(script_path, '/');
    if (script_name == NULL)
        script_name = script_path;
    else
        script_name += 1;

    if (setenv("HTTP_HOST", host_name, 1) == -1 ||
        setenv("QUERY_STRING", query_string, 1) == -1 ||
        setenv("REQUEST_METHOD", "GET", 1) == -1 ||
        setenv("SCRIPT_NAME", script_name, 1) == -1 ||
        setenv("SERVER_PROTOCOL", prot_version, 1) == -1)
        return 2;

    printf("%s 200 OK\n", prot_version);
    fflush(stdout);

    execl(script_path, script_path, NULL);
    return 3;
}
