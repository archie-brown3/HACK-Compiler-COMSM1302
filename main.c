#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "symboltable.h"
#include "token.h"

// Lexing functions
void lex_file(SymbolTable *labels, FILE *input, FILE *output);
void lex_line(int *rom_address, SymbolTable *labels, const char *line, FILE *output);
int lex_token(Token *dest, const char *line);
void lex_label(const char *line, int rom_address, SymbolTable *labels);

// Parsing functions
void parse_file(const SymbolTable *labels, SymbolTable *variables, FILE *input, FILE *output);
int get_next_instruction(Token *dest[], FILE *input);
void parse_instruction(const SymbolTable *labels, SymbolTable *variables, Token *instruction[], int length,
                       FILE *output);
void parse_a_instruction(const SymbolTable *labels, SymbolTable *variables, Token *instruction[], char *dest);
void parse_c_instruction(Token *instruction[], int length, char *dest_str);
void parse_c_comp(Token *instruction[], int length, char *comp_str);
void parse_c_jump(Token *instruction[], int length, char *jump_str);
void parse_c_dest(Token *instruction[], int length, char *dest_str);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Please supply two arguments: an input .asm file, and an output .hack file.");
        exit(EXIT_FAILURE);;
    }
    char *input_name = argv[1];
    char *output_name = argv[2];

    SymbolTable *labels = malloc_table();
    SymbolTable *variables = malloc_table();

    FILE *input = fopen(input_name, "r");
    if (input == NULL) {
        exit(EXIT_FAILURE);
    }
    FILE *lex_output = fopen("temp.lex", "w");
    if (lex_output == NULL) {        rom_address ++;

        exit(EXIT_FAILURE);
    }
    lex_file(labels, input, lex_output);
    fclose(input);
    fclose(lex_output);

    FILE *lex_input = fopen("temp.lex", "r");
    if (lex_input == NULL) {
        exit(EXIT_FAILURE);
    }
    FILE *output = fopen(output_name, "w");
    if (output == NULL) {
        exit(EXIT_FAILURE);
    }
    parse_file(labels, variables, lex_input, output);
    fclose(lex_input);
    fclose(output);

    free_table(labels);
    free_table(variables);
    return EXIT_SUCCESS;
}

// Outputs a tokenised version of input to output while populating labels.
void lex_file(SymbolTable *labels, FILE *input, FILE *output) {
    char line[MAX_LINE_LENGTH];
    int rom_address = 0;
    while (fgets(line, MAX_LINE_LENGTH, input) != NULL) {
        lex_line(&rom_address, labels, line, output);
    }
}
/**/
// Reads the next line from input, tokenises it, updates the label table, and writes the resulting tokens to output.
// Increments *rom_address if the current line contains an instruction (rather than e.g. labels, comments, etc.)
void lex_line(int *rom_address, SymbolTable *labels, const char *line, FILE *output) {
    int pos = 0;
    bool tokens_on_line = false;

    // In each iteration of this loop, everything up to line[pos] has been lexed.
    while ((line[pos] != '\n') && (line[pos] != '\r')) {
        Token *next = malloc_token();
        
        // If you see whitespace, ignore it.
        if (line[pos] == ' ' || line[pos] == '\t'){
            pos ++;
            continue;
        }
        // If you see a comment, ignore the rest of the line.
        if (line[pos] == '/'){
            if (line[pos + 1] =='/'){
            break;
            }
            if (line[pos + 1] =='*'){
            break;
            }
        }
        // Add labels to the symbol table.
        if (line[pos] == '(') {
            lex_label(line+pos, *rom_address, labels);
            break;
        }

        // Otherwise, we have at least one non-newline token, so lex it.
        tokens_on_line = true;
        pos += lex_token(next, line + pos);
        write_token(next, output);
        free_token(next);
    }

    if (tokens_on_line) {
        // INSERT CODE: If the line you just lexed contained tokens (i.e. if it corresponds to a line of machine code),
        // lex the newline at the end and update rom_address.
        
        // Allocate memory on the heap for a newline token
        Token *newline = malloc_token();
        // Mark the end of the instuction with a NEWLINE token
        newline->type = NEWLINE;

        // Write the newline token to the output file and cleanup memory
        write_token(newline, output);
        free_token(newline);
        (*rom_address)++;
    }
}

// Assuming line contains a label and starts with "(" (so no leading whitespace), extracts the label and adds it to
// the symbol table with ROM address rom_address. The line may end with a comment (e.g. "(LOOP) // Loop here").
void lex_label(const char *line, int rom_address, SymbolTable *labels) {
    // Your code here!
    char literal[MAX_LINE_LENGTH];
    // If the line does not start with '(', return immediately
    if (line[0] != '(') {
        return;
    }
   // Handle comments
   
   int index = 1;
   while (line[index] != ')' && line[index] != '\0') {
    index++;
   }
  for (int count = 1; count < index ; count++){
   // Copy charachters into buffer
    literal[count -1] = line[count];   
   }
   // Null terminate string
   literal[index -1] = '\0';   
  // Store in new ROM address
  add_to_table(labels, literal, rom_address);

}


// Reads the next token from a non-empty, non-label, non-comment line into dest, then returns the number of characters
// in that token.
int lex_token(Token *dest, const char *line) {
 static bool at_previous = false; // Will be set to 1 if the last time this function was called, it lexed an @.
 int length;

 // Newlines
 if (line[0] == '\n') {
  dest->type = NEWLINE;
  length = 1;
 } 
 // Symbols
 else if (line[0] == '@' || line[0] == '+' || line[0] == '-' || 
    line[0] == '&' || line[0] == '|' || line[0] == ';' || 
    line[0] == '!' || line[0] == '=') {
  dest->type = SYMBOL;
  dest->value.char_val = line[0];
  length = 1;
 } 
 // Integer literals
 else if (line[0] >= '0' && line[0] <= '9') {
  dest->type = INTEGER_LITERAL;
  char literal[MAX_LINE_LENGTH];
  for (length = 0; line[length] >= '0' && line[length] <= '9'; length++) {
   literal[length] = line[length];
  }
  literal[length] = '\0';
  dest->value.int_val = strtol(literal, NULL, 10);
 } 
 // Keywords
 else if (line[0] == 'A' || line[0] == 'D' || line[0] == 'M' || 
    line[0] == 'R' || line[0] == 'J' || line[0] == 'S' || 
    line[0] == 'K' || line[0] == 'L' || line[0] == 'T') {
  dest->type = KEYWORD;
  char keyword[MAX_LINE_LENGTH];
  for (length = 0; (line[length] >= 'A' && line[length] <= 'Z') || 
       (line[length] >= '0' && line[length] <= '9'); length++) {
   keyword[length] = line[length];
  }
  keyword[length] = '\0';
  // TODO: Parse keyword string and set dest->value.key_val
 }
 // Identifiers
 else {
  dest->type = IDENTIFIER;
  char identifier[MAX_LINE_LENGTH];
  for (length = 0; (line[length] >= 'a' && line[length] <= 'z') || 
       (line[length] >= 'A' && line[length] <= 'Z') ||
       (line[length] >= '0' && line[length] <= '9') ||
       line[length] == '_' || line[length] == '.' || 
       line[length] == '$' || line[length] == ':'; length++) {
   identifier[length] = line[length];
  }
  identifier[length] = '\0';
  // TODO: Copy identifier to dest->value
 }
 at_previous = (line[0] == '@');
 return length;
}

// Labels should be a pre-populated label symbol table. Input should be a tokenised file. Populates the variables symbol
// table and writes Hack machine code to output.
void parse_file(const SymbolTable *labels, SymbolTable *variables, FILE *input, FILE *output) {
    Token *instruction[MAX_LINE_LENGTH];
    while (1) {
        int length = get_next_instruction(instruction, input);
        if (length == 0) {
            free_token(instruction[0]);
            break;
        }
        parse_instruction(labels, variables, instruction, length, output);
        for(int i=0; i<length; i++) {
            free_token(instruction[i]);
        }
    }
}

// Copies a list of tokens corresponding to the next Hack instruction into dest, omitting the newline. Return number
// of tokens.
int get_next_instruction(Token *dest[], FILE *input) {
    dest[0] = malloc_token();
    bool instruction_exists = read_token(dest[0], input);
    if (!instruction_exists) {
        return 0;
    }
    int length = 1;
    while (1) {
        dest[length] = malloc_token();
        read_token(dest[length], input);
        if (dest[length]->type == NEWLINE) {
            free_token(dest[length]);
            break;
        }
        length++;
    }
    return length;
}

// Parse the given instruction (containing length operands) and write the corresponding Hack code to the output file.
void parse_instruction(const SymbolTable *labels, SymbolTable *variables, Token *instruction[], int length,
                       FILE *output) {
    char hack_instruction[18] = "";
    if (1){ // INSERT CODE: Fill in the right condition here
        parse_a_instruction(labels, variables, instruction, hack_instruction);
    } else {
        parse_c_instruction(instruction, length, hack_instruction);
    }
    strcat(hack_instruction, "\n");
    fputs(hack_instruction, output);
}

// Parse the operand of the given A instruction, updating the variables table accordingly, and put the corresponding
// Hack command into dest.
void parse_a_instruction(const SymbolTable *labels, SymbolTable *variables, Token *instruction[], char *dest) {
    Token *operand = instruction[1];
    int value_to_load;
    if (operand->type == INTEGER_LITERAL) {
        value_to_load = operand->value.int_val;
    } else if (operand->type == IDENTIFIER) {
        // INSERT CODE: If the token is a label, set value_to_load to the correct ROM address. Otherwise, it's a
        // variable; set value_to_load to the correct RAM address, first adding the variable to the symbol table if
        // needed.
    } else {
        // INSERT CODE: If the token is a keyword, set value_to_load to the correct integer literal.
    }
    // Writes the address to dest, padding with zeroes.
    int_to_bin_string(value_to_load, dest);
}

// Parse the operand of the given C instruction and put the corresponding Hack command into dest_str.
void parse_c_instruction(Token *instruction[], int length, char *dest_str) {
    char comp_op[8];
    char dest_op[4];
    char jump_op[4];

    parse_c_comp(instruction, length, comp_op);
    parse_c_dest(instruction, length, dest_op);
    parse_c_jump(instruction, length, jump_op);

    strcat(dest_str, "111");
    strcat(dest_str, comp_op);
    strcat(dest_str, dest_op);
    strcat(dest_str, jump_op);
}

// Given a list of tokens of length [length] forming a C-instruction, copy the jump operand into jump_str.
void parse_c_jump(Token *instruction[], int length, char *jump_str) {
    // INSERT CODE: Compute the C-instruction's jump operand and copy it into jump_str.
}

// Given a list of tokens of length [length] forming a C-instruction in assembly, copy the dest operand into dest_str.
void parse_c_dest(Token *instruction[], int length, char *dest_str) {
    // INSERT CODE: Compute the C-instruction's dest operand and copy it into jump_str.
}

// Given a list of tokens of length [length] forming a C-instruction, copy the comp operand into comp_str.
void parse_c_comp(Token *instruction[], int length, char *comp_str) {
    // Set comp_start to the index of the start of the computation part of the instruction, i.e. after the = if there
    // is one or at the start otherwise.
    int comp_start = 0;
    for(int i=0; i<length; i++) {
        if (instruction[i]->type == SYMBOL && instruction[i]->value.char_val == '=') {
            comp_start = i+1;
            break;
        }
    }
    // Set comp_end to the index of the end of the computation part of the instruction, i.e. before the ";" if there is
    // one or at the end otherwise.
    int comp_end;
    if(length >= 2 && instruction[length-2]->type == SYMBOL && instruction[length-2]->value.char_val == ';') {
        comp_end = length-3;
    } else {
        comp_end = length-1;
    }

    // Number of characters in the computation part of the instruction.
    int comp_length = comp_end - comp_start + 1;

    strcpy(comp_str, "0000000");
    switch(comp_length) {
        case 1: // Must be 0, 1, A, D or M
            switch (instruction[comp_start]->type) {
                case INTEGER_LITERAL:
                    switch (instruction[comp_start]->value.int_val) {
                        case 0:
                            strcpy(comp_str, "0101010");
                            break;
                        case 1:
                            strcpy(comp_str, "0111111");
                            break;
                        default:
                            exit(EXIT_FAILURE);
                    }
                    break;
                case KEYWORD:
                    switch (instruction[comp_start]->value.key_val) {
                        case KW_A:
                            strcpy(comp_str, "0110000");
                            break;
                        case KW_D:
                            strcpy(comp_str, "0001100");
                            break;
                        case KW_M:
                            strcpy(comp_str, "1110000");
                            break;
                        default:
                            exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    exit(EXIT_FAILURE);
            }
            break;
        case 2: // Must start with ! or -
            switch (instruction[comp_start]->value.char_val) {
                case '!':
                    // INSERT CODE: Deal with !A, !D or !M.
                    break;
                case '-':
                    if (instruction[comp_start + 1]->type == INTEGER_LITERAL) { // Must be -1
                        strcpy(comp_str, "0111010");
                        break;
                    }
                    switch (instruction[comp_start + 1]->value.key_val) {
                        case KW_A:
                            strcpy(comp_str, "0110011");
                            break;
                        case KW_D:
                            strcpy(comp_str, "0001111");
                            break;
                        case KW_M:
                            strcpy(comp_str, "1110011");
                            break;
                        default:
                            exit(EXIT_FAILURE);
                    }
                    break;
                default:
                    exit(EXIT_FAILURE);
            }
            break;
        case 3: {
            // Without these as shorthand things get truly horrible
            bool operands_ad = (instruction[comp_start]->value.key_val == KW_A &&
                                instruction[comp_start + 2]->value.key_val == KW_D);
            bool operands_da = (instruction[comp_start]->value.key_val == KW_D &&
                                instruction[comp_start + 2]->value.key_val == KW_A);
            bool operands_dm = (instruction[comp_start]->value.key_val == KW_D &&
                                instruction[comp_start + 2]->value.key_val == KW_M);
            bool operands_md = (instruction[comp_start]->value.key_val == KW_M &&
                                instruction[comp_start + 2]->value.key_val == KW_D);
            switch(instruction[comp_start+1]->value.char_val) {
                case '+':
                    if (instruction[comp_start+2]->type == INTEGER_LITERAL) { // Must be A+1, D+1 or M+1
                        switch(instruction[comp_start]->value.key_val) {
                            case KW_A: strcpy(comp_str, "0110111"); break;
                            case KW_D: strcpy(comp_str, "0011111"); break;
                            case KW_M: strcpy(comp_str, "1110111"); break;
                            default: exit(EXIT_FAILURE);
                        }
                        // Otherwise, the only options are A+D, D+A, M+D or D+M.
                    } else if (operands_ad || operands_da) {
                        strcpy(comp_str, "0000010"); // A+D or D+A
                    } else if (operands_md || operands_dm) {
                        strcpy(comp_str, "1000010"); // M+D or D+M
                    } else {
                        exit(EXIT_FAILURE);
                    } break;
                case '-':
                    // INSERT CODE: Deal with A-1, D-1, M-1, A-D, D-A, M-D or D-M,
                    break;
                case '&': // Must be A&D, D&A, M&D or D&M
                    if (operands_ad || operands_da) {
                        strcpy(comp_str, "0000000"); // A&D
                    } else if (operands_md || operands_dm) {
                        strcpy(comp_str, "1000000"); // D&A
                    } else {
                        exit(EXIT_FAILURE);
                    } break;
                case '|': // Must be A|D, D|A, M|D or D|M
                    if (operands_ad || operands_da) {
                        strcpy(comp_str, "0010101"); // A&D
                    } else if (operands_md || operands_dm) {
                        strcpy(comp_str, "1010101"); // D&A
                    } else {
                        exit(EXIT_FAILURE);
                    } break;
                default: exit(EXIT_FAILURE);
            } break;
        }
        default: exit(EXIT_FAILURE);
    }
}

// Converts the given integer into a 16-bit binary value, storing the result in dest. Pad with zeroes at the left.
void int_to_bin_string(int num, char *dest) {
    for(int i=15; i>=0; i--, num /= 2) {
        int binary_digit = num % 2;
        dest[i] = (char)(binary_digit + '0');
    }
    dest[16] = '\0';
}