#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_CHAR_MAP 256
#define MAX_FILENAME 100
#define MAX_LINE_LENGTH 1024

// Function prototypes
void load_char_map(void);
void encode_file(void);
void decode_file(void);
int find_char_index(char c);

// Character mapping array that will be loaded from file
char char_map[MAX_CHAR_MAP];
int char_map_size = 0;
int is_map_loaded = 0;

int main() {
    int choice = 0;
    
    while (1) {
        printf("\nC Source Code Encoder/Decoder\n");
        printf("1. Load character mapping from file\n");
        printf("2. Encode C file to numeric format\n");
        printf("3. Decode numeric file to C source code\n");
        printf("4. Exit\n");
        printf("Enter your choice: ");
        
        if (scanf("%d", &choice) != 1) {
            // Clear the input buffer if invalid input
            while (getchar() != '\n');
            printf("Invalid input. Please enter a number.\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                load_char_map();
                break;
            case 2:
                if (!is_map_loaded) {
                    printf("Please load a character map first (option 1).\n");
                } else {
                    encode_file();
                }
                break;
            case 3:
                if (!is_map_loaded) {
                    printf("Please load a character map first (option 1).\n");
                } else {
                    decode_file();
                }
                break;
            case 4:
                printf("Exiting program. Goodbye!\n");
                return 0;
            default:
                printf("Invalid choice. Please try again.\n");
                break;
        }
    }
    
    return 0;
}

// Load the character mapping from a user-specified file
void load_char_map(void) {
    char filename[MAX_FILENAME];
    FILE *file;
    char line[MAX_LINE_LENGTH];
    
    // Clear input buffer
    while (getchar() != '\n');
    
    printf("Enter the file number containing the character mapping (will load from [number].txt): ");
    int file_number;
    scanf("%d", &file_number);
    snprintf(filename, MAX_FILENAME, "%d.txt", file_number);
    
    file = fopen(filename, "r");
    if (!file) {
        printf("Error: Could not open mapping file %s\n", filename);
        return;
    }
    
    // Reset the character map
    memset(char_map, 0, MAX_CHAR_MAP);
    char_map_size = 0;
    
    // Read each line from the file
    // Format expected: index<tab>character
    int line_num = 0;
    while (fgets(line, MAX_LINE_LENGTH, file) && char_map_size < MAX_CHAR_MAP) {
        line_num++;
        // Remove newline character if present
        line[strcspn(line, "\n")] = 0;
        
        // Parse the line: index<tab>character
        int index;
        char *tab_pos = strchr(line, '\t');
        if (!tab_pos) {
            printf("Warning: Line %d is not in the expected format (index<tab>character), skipping\n", line_num);
            continue;
        }
        
        // Convert the index part to an integer
        *tab_pos = '\0'; // Temporarily terminate string at tab position
        if (sscanf(line, "%d", &index) != 1) {
            printf("Warning: Line %d has an invalid index, skipping\n", line_num);
            continue;
        }
        
        // Process the character part
        char *char_part = tab_pos + 1;
        
        // Handle special cases for whitespace characters
        if (strcmp(char_part, "Space") == 0) {
            if (index > char_map_size) char_map_size = index;
            char_map[index - 1] = ' ';
        } else if (strcmp(char_part, "Tab") == 0) {
            if (index > char_map_size) char_map_size = index;
            char_map[index - 1] = '\t';
        } else if (strlen(char_part) == 1) {
            // Regular single character
            if (index > char_map_size) char_map_size = index;
            char_map[index - 1] = char_part[0];
        } else if (char_part[0] == '\\' && strlen(char_part) == 2) {
            // Escape sequences like \n, \t, etc.
            if (index > char_map_size) char_map_size = index;
            switch (char_part[1]) {
                case 'n': char_map[index - 1] = '\n'; break;
                case 't': char_map[index - 1] = '\t'; break;
                case 'r': char_map[index - 1] = '\r'; break;
                case '0': char_map[index - 1] = '\0'; break;
                case '\\': char_map[index - 1] = '\\'; break;
                case '\'': char_map[index - 1] = '\''; break;
                case '\"': char_map[index - 1] = '\"'; break;
                default: 
                    printf("Warning: Unknown escape sequence %s, ignoring\n", char_part);
                    continue;
            }
        } else if (strlen(char_part) > 0) {
            // More complex representation - just take the first character
            printf("Warning: Character part '%s' contains multiple characters, using first one: '%c'\n", 
                   char_part, char_part[0]);
            if (index > char_map_size) char_map_size = index;
            char_map[index - 1] = char_part[0];
        }
    }
    
    fclose(file);
    is_map_loaded = 1;
    printf("Character mapping loaded successfully with %d characters.\n", char_map_size);
    
    // Print out the mapping for verification
    printf("Loaded character map:\n");
    for (int i = 0; i < char_map_size; i++) {
        if (isprint(char_map[i])) {
            printf("%d: '%c'\n", i + 1, char_map[i]);
        } else {
            printf("%d: '\\x%02x'\n", i + 1, (unsigned char)char_map[i]);
        }
    }
    
    // Clear input buffer
    while (getchar() != '\n');
}

// Find the index of a character in the char_map array
int find_char_index(char c) {
    for (int i = 0; i < char_map_size; i++) {
        if (char_map[i] == c) {
            return i + 1; // +1 because our list is 1-indexed
        }
    }
    return -1; // Character not found
}

// Encode a C source file to a numeric format
void encode_file(void) {
    char input_filename[MAX_FILENAME];
    char output_filename[MAX_FILENAME];
    FILE *input_file, *output_file;
    int c;
    
    // Clear input buffer
    while (getchar() != '\n');
    
    printf("Enter the name of the C source file to encode: ");
    fgets(input_filename, MAX_FILENAME, stdin);
    input_filename[strcspn(input_filename, "\n")] = 0; // Remove newline
    
    printf("Enter the output file number (will be saved as [number].txt): ");
    int file_number;
    scanf("%d", &file_number);
    snprintf(output_filename, MAX_FILENAME, "%d.txt", file_number);
    
    input_file = fopen(input_filename, "r");
    if (!input_file) {
        printf("Error: Could not open input file %s\n", input_filename);
        return;
    }
    
    output_file = fopen(output_filename, "w");
    if (!output_file) {
        printf("Error: Could not open output file %s\n", output_filename);
        fclose(input_file);
        return;
    }
    
    printf("Encoding file %s to %s...\n", input_filename, output_filename);
    
    // Read each character from the input file
    while ((c = fgetc(input_file)) != EOF) {
        int index = find_char_index((char)c);
        if (index != -1) {
            fprintf(output_file, "%d ", index);
        } else {
            // For characters not in our mapping, we could use a special code or skip
            // Here we'll use 0 to represent unmapped characters
            fprintf(output_file, "0 ");
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    printf("Encoding complete.\n");
    
    // Clear input buffer
    while (getchar() != '\n');
}

// Decode a numeric file back to C source code
void decode_file(void) {
    char input_filename[MAX_FILENAME];
    char output_filename[MAX_FILENAME];
    FILE *input_file, *output_file;
    int index;
    
    // Clear input buffer
    while (getchar() != '\n');
    
    printf("Enter the input file number to decode (e.g., for 5.txt, enter 5): ");
    int file_number;
    scanf("%d", &file_number);
    snprintf(input_filename, MAX_FILENAME, "%d.txt", file_number);
    
    // Clear input buffer
    while (getchar() != '\n');
    
    printf("Enter the output file number for the decoded C source (will be saved as [number].txt): ");
    scanf("%d", &file_number);
    snprintf(output_filename, MAX_FILENAME, "%d.txt", file_number);
    
    input_file = fopen(input_filename, "r");
    if (!input_file) {
        printf("Error: Could not open input file %s\n", input_filename);
        return;
    }
    
    output_file = fopen(output_filename, "w");
    if (!output_file) {
        printf("Error: Could not open output file %s\n", output_filename);
        fclose(input_file);
        return;
    }
    
    printf("Decoding file %s to %s...\n", input_filename, output_filename);
    
    // Read each number from the input file
    while (fscanf(input_file, "%d", &index) == 1) {
        if (index > 0 && index <= char_map_size) {
            fputc(char_map[index - 1], output_file); // -1 because our list is 1-indexed
        } else if (index == 0) {
            // Handle unmapped characters (we used 0 as a special code)
            fputc('?', output_file); // Replace with some placeholder
        }
    }
    
    fclose(input_file);
    fclose(output_file);
    printf("Decoding complete.\n");
    
    // Clear input buffer
    while (getchar() != '\n');
}