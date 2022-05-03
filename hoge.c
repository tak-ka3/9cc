#include <stdio.h>
#include <stdarg.h>
#include <string.h>

// 可変長引数の使い方
int fuga(char * fmt, ...) {
    va_list args;
    va_start(args, fmt);

    printf("all=%s\n", fmt);
    char* fmt2 = va_arg(args, char*);
    printf("arg2: %s\n", fmt2);

    return 0;
}

int main() {
    fuga("hello world", "second");
    char *num = strstr("+-*/()", "+/");
    printf("num=%s\n", num);
    return 0;
}