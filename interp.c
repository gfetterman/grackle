#include<stdio.h>
#include<stdbool.h>
#include<string.h>
#include<stdlib.h>

#define PROMPT ">>>"
#define BUF_SIZE 80
#define EXIT_KEYWORD "(exit)"

typedef struct LLNODE {
    unsigned int symbol_number;
    char* symbol;
    char* value;
    struct LLNODE* next;
} Node;

Node* create_node(char* symbol, char* value, unsigned int symbol_number) {
    Node* new_node = malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "fatal error: malloc failed in create_node()\n");
        exit(-1);
    }
    new_node->symbol_number = symbol_number;
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

Node* symbol_from_index(Node* list, unsigned int index) {
    Node* curr = list;
    while (curr != NULL) {
        if (curr->symbol_number == index) {
            break;
        }
        curr = curr->next;
    }
    return curr;
}

unsigned int len(Node* list) {
    unsigned int length = 0;
    while (list != NULL) {
        list = list->next;
        length++;
    }
    return length;
}

unsigned int install_symbol(Node** list, char* symbol, char* value) {
    Node* existing_value = search_list(*list, symbol);
    if (existing_value == NULL) {
        unsigned int num = len(*list);
        Node* new_node = create_node(symbol, value, num);
        new_node->next = *list;
        *list = new_node;
        return num;
    } else {
        existing_value->value = value;
        return existing_value->symbol_number;
    }
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

void print_symbol_table(Node* list) {
    Node* curr = list;
    printf("current symbol table:\n");
    while (curr != NULL) {
        printf("  symbol #%d, \"%s\", has value \"%s\"\n", curr->symbol_number, curr->symbol, curr->value);
        curr = curr->next;
    }
    return;
}

unsigned int pull_token(Node** symbol_table, \
                        char str[], \
                        unsigned int length, \
                        unsigned int* offset) {
    // length must == length of character array str
    // pulls one token out of the front of str
    // if it reaches the end of str, it returns 0
    // if it's '(' or ')', it returns 1 or 2, respectively
    // if it's anything else, it returns an index into
    // the symbol table (here, for the moment, the number
    // table and symbol table have been merged)
    // leading spaces are ignored
    // (it can't handle other whitespace at the moment)
    //printf("string to tokenize: \"%s\", beginning with '%c'\n", str, str[*offset]);
    while (*offset < length && str[*offset] == ' ') {
        (*offset)++;
    }
    if (*offset >= length) { // reached end of input
        return 0;
    } else if (str[*offset] == '(') { // open parenthesis
        (*offset)++;
        return 1;
    } else if (str[*offset] == ')') { // close parenthesis
        (*offset)++;
        return 2;
    } else { // number or atom
        unsigned int token_len = 0;
        while (*offset + token_len < length && \
               str[*offset + token_len] != ' ' && \
               str[*offset + token_len] != '(' && \
               str[*offset + token_len] != ')') {
            token_len++;
        }
        char* token_str = substring(str, *offset, *offset + token_len);
        //printf("symbol found: %s\n", token_str);
        Node* found = search_list(*symbol_table, token_str);
        (*offset) += token_len; // for the next call to pull_token()
        if (found == NULL) {
            unsigned int new_offset = install_symbol(symbol_table, token_str, token_str);
            //printf("new symbol added to symbol table at position %d\n", new_offset);
            //print_symbol_table(*symbol_table);
            // and now it's the symbol table's problem to free the memory
            // allocated for token_str
            return new_offset;
        } else {
            free(token_str);
            return found->symbol_number;
        }
    }
}

void setup_symbol_table(Node** symbol_table) {
    install_symbol(symbol_table, "NULL_SENTINEL", "NULL_SENTINEL");
    install_symbol(symbol_table, "(", "(");
    install_symbol(symbol_table, ")", ")");
    install_symbol(symbol_table, "+", "+");
    install_symbol(symbol_table, "*", "*");
    install_symbol(symbol_table, "-", "-");
    install_symbol(symbol_table, "/", "/");
    install_symbol(symbol_table, "exit", "exit");
    return;
}

void get_input(char* prompt, char buffer[], unsigned int buffer_size) {
    printf("%s ", prompt);
    fgets(buffer, buffer_size, stdin); // yes, this is unsafe
    // drop newline at end of input
    if ((strlen(buffer) > 0) && (buffer[strlen(buffer) - 1] == '\n')) {
        buffer[strlen(buffer) - 1] = '\0';
    }
    return;
}

unsigned int lookahead(unsigned int token_list[], unsigned int num_tokens) {
    unsigned int open_parens = 0;
    unsigned int finger = 0;
    while (finger < num_tokens) {
        switch (token_list[finger]) {
            case 0:
                open_parens = 0;
                break;
            case 1:
                open_parens++;
                break;
            case 2:
                open_parens--;
                break;
            default:
                break;
        }
        finger++;
        if (open_parens == 0) {
            break;
        }
    }
    return finger;
}

unsigned int evaluate(Node** symbol_table, \
                      unsigned int token_list[], \
                      unsigned int num_tokens) {
    unsigned int finger = 0;
    unsigned int result = 0;
    while (finger < num_tokens) {
        printf("case %d\n", token_list[finger]);
        switch (token_list[finger]) {
            case 0: // end of token list
                finger = num_tokens;
                break;
            case 1: // open parenthesis
                finger++;
                break;
            case 2: // close parenthesis
                finger = num_tokens;
                break;
            case 3: {// addition
                unsigned int operand_1, operand_2;
                unsigned int len_1 = lookahead(token_list + finger + 1, num_tokens - finger - 1);
                unsigned int len_2 = lookahead(token_list + finger + 1 + len_1, num_tokens - finger - 1 - len_1);
                if (token_list[finger + 1 + len_1 + len_2] != 2) {
                    printf("error: '+' operator takes exactly 2 operands\n");
                    return 0;
                }
                operand_1 = evaluate(symbol_table, token_list + finger + 1, len_1);
                operand_2 = evaluate(symbol_table, token_list + finger + 1 + len_1, len_2);
                result = operand_1 + operand_2;
                finger = num_tokens;
                break;
            }
            case 4: { // multiplication
                unsigned int operand_1, operand_2;
                unsigned int len_1 = lookahead(token_list + finger + 1, num_tokens - finger - 1);
                unsigned int len_2 = lookahead(token_list + finger + 1 + len_1, num_tokens - finger - 1 - len_1);
                if (token_list[finger + 1 + len_1 + len_2] != 2) {
                    printf("error: '*' operator takes exactly 2 operands\n");
                    return 0;
                }
                operand_1 = evaluate(symbol_table, token_list + finger + 1, len_1);
                operand_2 = evaluate(symbol_table, token_list + finger + 1 + len_1, len_2);
                result = operand_1 * operand_2;
                finger = num_tokens;
                break;
            }
            case 5: { // subtraction
                unsigned int operand_1, operand_2;
                unsigned int len_1 = lookahead(token_list + finger + 1, num_tokens - finger - 1);
                unsigned int len_2 = lookahead(token_list + finger + 1 + len_1, num_tokens - finger - 1 - len_1);
                if (token_list[finger + 1 + len_1 + len_2] != 2) {
                    printf("error: '-' operator takes exactly 2 operands\n");
                    return 0;
                }
                operand_1 = evaluate(symbol_table, token_list + finger + 1, len_1);
                operand_2 = evaluate(symbol_table, token_list + finger + 1 + len_1, len_2);
                result = operand_1 - operand_2;
                finger = num_tokens;
                break;
            }
            case 6: { // (integer) division
                unsigned int operand_1, operand_2;
                unsigned int len_1 = lookahead(token_list + finger + 1, num_tokens - finger - 1);
                unsigned int len_2 = lookahead(token_list + finger + 1 + len_1, num_tokens - finger - 1 - len_1);
                if (token_list[finger + 1 + len_1 + len_2] != 2) {
                    printf("error: '/' operator takes exactly 2 operands\n");
                    return 0;
                }
                operand_1 = evaluate(symbol_table, token_list + finger + 1, len_1);
                operand_2 = evaluate(symbol_table, token_list + finger + 1 + len_1, len_2);
                result = operand_1 / operand_2;
                finger = num_tokens;
                break;
            }
            case 7:
                printf("exiting...\n");
                exit(0);
            default: {
                Node* found = symbol_from_index(*symbol_table, token_list[finger]);
                if (found == NULL) {
                    printf("error: token index \"%d\" unrecognized\n", token_list[finger]);
                } else {
                    result = atoi(found->value);
                }
                finger++;
            }
        }
    }
    return result;
}

int main() {
    bool exit = 0;
    char input[BUF_SIZE];
    Node* symbol_table = NULL;
    setup_symbol_table(&symbol_table);
    unsigned int token_list[20]; // this is a temporary solution
    while (!exit) {
        // clear token list
        for (int i = 0; i < 20; i++) {
            token_list[i] = 0;
        }
        get_input(PROMPT, input, BUF_SIZE);
        unsigned int token;
        unsigned int num_tokens = 0;
        unsigned int offset = 0;
        while ((token = pull_token(&symbol_table, input, strlen(input), &offset))) {
            if (num_tokens >= 20) {
                printf("error: cannot parse more than 20 tokens\n");
                break; // then evaluation will probably fail and throw another error
            }
            token_list[num_tokens] = token;
            num_tokens++;
        }
        printf("result = %u\n", evaluate(&symbol_table, token_list, num_tokens));
    }
    return 0;
}
