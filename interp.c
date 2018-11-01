#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define PROMPT ">>>"
#define BUF_SIZE 80
#define EXIT_KEYWORD "(exit)"

typedef struct LLNODE {
    char* symbol;
    char* value;
    struct LLNODE* next;
} Node;

typedef struct TOKENLIST {
    unsigned int num_tokens;
    int* token_starts;
    char* full_string;
    unsigned int str_len;
} TokenList;

Node* create_node(char* symbol, char* value) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "fatal error: malloc failed in create_node()\n");
        exit(-1);
    }
    new_node->symbol = symbol;
    new_node->value = value;
    new_node->next = NULL;
    return new_node;
}

Node* search_list(Node* list, char* symbol) {
    Node* curr = list;
    while (curr != NULL) {
        if (!strcmp(curr->symbol, symbol)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

void install_symbol(Node** list, char* symbol, char* value) {
    Node* existing_value = search_list(*list, symbol);
    if (existing_value == NULL) {
        Node* new_node = create_node(symbol, value);
        new_node->next = *list;
        *list = new_node;
    } else {
        existing_value->value = value;
    }
    return;
}

char* substring(char* str, unsigned int start, unsigned int end) {
    if (str == NULL || strlen(str) < (end - start)) {
        fprintf(stderr, \
                "fatal error: substring bad address: start %d -> end %d in string of length %d\n", \
                start,
                end,
                (int)strlen(str));
        exit(-1);
    }
    char* ss = malloc(sizeof(char) * (end - start + 1));
    if (ss == NULL) {
        fprintf(stderr, "fatal error: malloc failed in substring()\n");
        exit(-1);
    }
    memcpy(ss, (str + start), (sizeof(char) * (end - start)));
    ss[end - start] = '\0';
    return ss;
}

bool substring_match(char* str1, char* str2, unsigned int start, unsigned int end) {
    // str1 must be of length at least end
    // str2 must be of length exactly (end - start)
    // returns 1 for match, 0 for no match
    if (strlen(str1) < end || strlen(str2) != (end - start)) {
        return 0;
    }
    bool match = 1;
    for (int i = 0; i < (end - start); i++) {
        match &= (str1[start + i] == str2[i]);
    }
    return match;
}

TokenList* tokenize(char* str) {
    TokenList* tklist = malloc(sizeof(TokenList));
    if (tklist == NULL) {
        fprintf(stderr, "fatal error: malloc failed in tokenize()\n");
        exit(-1);
    }
    char c;
    unsigned int num_tokens = 1;
    char* full_string = str;
    while((c = *full_string++)) {
        num_tokens += (c == ' ') ? 1 : 0;
    }
    tklist->num_tokens = num_tokens;
    tklist->full_string = str;
    tklist->str_len = strlen(str);
    tklist->token_starts = malloc(sizeof(int) * num_tokens);
    if (tklist->token_starts == NULL) {
        fprintf(stderr, "fatal error: malloc failed in tokenize()\n");
        exit(-1);
    }
    tklist->token_starts[0] = 0;
    num_tokens = 1;
    for (int i = 0; i < tklist->str_len; i++) {
        if (str[i] == ' ') {
            tklist->token_starts[num_tokens] = i;
            num_tokens++;
        }
    }
    return tklist;
}

void print_tokens(TokenList* tklist) {
    if (tklist != NULL) {
        char* temp;
        printf("whole string: %s\n", tklist->full_string);
        printf("number of tokens: %d\n", (int)tklist->num_tokens);
        for (int i = 0; i < tklist->num_tokens; i++) {
            printf("  token %d\n", i);
            printf("    start: %d\n", tklist->token_starts[i]);
            printf("    end: %d\n", (i == tklist->num_tokens - 1) ? tklist->str_len : tklist->token_starts[i + 1]);
            temp = substring(tklist->full_string, \
                             tklist->token_starts[i], \
                             (i == tklist->num_tokens - 1) ? tklist->str_len : tklist->token_starts[i + 1]);
            printf("    substring: %s\n", temp);
            free(temp);
        }
    }
    return;
}

void print_symbol_table(Node* list) {
    Node* curr = list;
    printf("current symbol table:\n");
    while (curr != NULL) {
        printf("  symbol \"%s\" has value \"%s\"\n", curr->symbol, curr->value);
        curr = curr->next;
    }
    return;
}

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    Node* symbol_table = NULL;
    while (!exit) {
        printf("%s ", PROMPT);
        fgets(input, BUF_SIZE, stdin); // YES I KNOW this is unsafe
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n')) {
            input[strlen(input) - 1] = '\0';
        }
        TokenList* tklist = tokenize(input);
        //print_tokens(tklist);
        if (!strcmp(EXIT_KEYWORD, input)) {
            exit = 1;
        } else if (tklist->num_tokens > 0 && \
                   substring_match(input, \
                                   "(SETQ", \
                                   tklist->token_starts[0], \
                                   (tklist->num_tokens > 1) ? tklist->token_starts[1] : tklist->str_len)) {
            if (tklist->num_tokens == 3) {
                // substring allocates the string locations, and those strings
                // are now the symbol table's responsibility to free when necessary
                install_symbol(&symbol_table,
                               substring(input, tklist->token_starts[1], tklist->token_starts[2]), \
                               substring(input, tklist->token_starts[2], tklist->str_len));
            } else {
                fprintf(stderr, "\"SETQ\" takes exactly 2 arguments; %d given\n", tklist->num_tokens);
            } 
        } else if (tklist->num_tokens > 0 && \
                   substring_match(input, \
                                   "(QUOTE", \
                                   tklist->token_starts[0], \
                                   (tklist->num_tokens > 1) ? tklist->token_starts[1] : tklist->str_len)) {
            if (tklist->num_tokens == 2) {
                char* symbol = substring(input, \
                                         tklist->token_starts[1], tklist->str_len);
                Node* lookup = search_list(symbol_table, symbol);
                if (lookup == NULL) {
                    fprintf(stderr, "error: \"%s\" not in symbol table\n", symbol);
                } else {
                    printf("value of %s = %s\n", lookup->symbol, lookup->value);
                }
                free(symbol);
            } else {
                fprintf(stderr, "\"QUOTE\" takes exactly 1 argument; %d given\n", tklist->num_tokens);
            }
        } else {
            printf("%s\n", input);
        }
        //print_symbol_table(symbol_table);
    }
    return 0;
}
