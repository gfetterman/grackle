#include<stdio.h>
#include<stdbool.h>
#include<string.h>

#define PROMPT ">>>"
#define BUF_SIZE 80
#define EXIT_KEYWORD "(exit)"

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    while (!exit) {
        printf("%s ", PROMPT);
        fgets(input, BUF_SIZE, stdin); // YES I KNOW this is unsafe
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n')) {
            input[strlen(input) - 1] = '\0';
        }
        if (!strcmp(EXIT_KEYWORD, input)) {
            exit = 1;
        }
    }
    return 0;
}
