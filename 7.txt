//gcc main.c -o homeGame -I"C:/Users/dacoo/raylib/src" -L"C:/Users/dacoo/raylib/src" -lraylib -lopengl32 -lgdi32 -lwinmm -lm -lpthread
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/stat.h>
#include <pthread.h>

#ifdef _WIN32
#include <direct.h>
#define mkdir _mkdir  // Windows-specific mkdir alias
#endif

#define GRID_ROWS 5
#define GRID_COLS 5
#define BUTTON_SIZE 100
#define MAX_SCRIPTS 25
#define MAX_TEXT_LENGTH 512
#define MAX_GRIDS 200
#define GAME_FILES_PATH "gameFiles/"

// Function declarations
void SaveScriptToFile(const char* filename, const char* text);
void LoadScriptFromFile(const char* filename, char* textBuffer, int bufferSize);
bool DoesFileExist(const char* filename);
void* ExecuteCommand(void* arg);
void RunCommandAsync(const char* command);
void OpenScriptInNotepad(const char* filename);
void ExecutePythonScript(const char* filename);
void CompileAndExecuteCFile(const char* filename);
void CreateNewScript(int gridIndex, int scriptIndex);
void CreateDescriptionFile(const char* scriptFilename);
void SwitchGrid(int gridID);
void ToggleScriptMode();
void ToggleEditMode();
void DrawHelpMenu();
void DrawFeedbackPanel();
void InitializeAndLoadExistingScripts();

// Define the structures and variables
typedef struct {
    char text[MAX_TEXT_LENGTH];
    bool isEditing;
    char filename[50];
    bool isCFile;
} Script;

typedef struct {
    Script scripts[MAX_SCRIPTS];
    int gridID;
} ScriptGrid;

ScriptGrid grids[MAX_GRIDS];
int currentGridIndex = 0;
int selectedScriptIndex = -1;
bool isCFile = false;
bool isEditingMode = true;
bool showHelpMenu = false;

// Function definitions
void SaveScriptToFile(const char* filename, const char* text) {
    FILE* file = fopen(filename, "w");
    if (file) {
        fprintf(file, "%s", text);
        fclose(file);
    }
}

void LoadScriptFromFile(const char* filename, char* textBuffer, int bufferSize) {
    FILE* file = fopen(filename, "r");
    if (file) {
        // Read entire file into buffer, not just one line
        size_t bytesRead = fread(textBuffer, 1, bufferSize - 1, file);
        textBuffer[bytesRead] = '\0'; // Ensure null termination
        fclose(file);
    } else {
        // Initialize buffer to empty string if file can't be opened
        textBuffer[0] = '\0';
    }
}

bool DoesFileExist(const char* filename) {
    struct stat buffer;
    return (stat(filename, &buffer) == 0);
}

void* ExecuteCommand(void* arg) {
    char* command = (char*)arg;
    system(command);
    free(command);
    return NULL;
}

void RunCommandAsync(const char* command) {
    pthread_t thread;
    char* commandCopy = strdup(command);
    pthread_create(&thread, NULL, ExecuteCommand, commandCopy);
    pthread_detach(thread);
}

void OpenScriptInNotepad(const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "notepad %s", filename);
    RunCommandAsync(command);
}

void ExecutePythonScript(const char* filename) {
    char command[256];
    snprintf(command, sizeof(command), "python %s", filename);
    RunCommandAsync(command);
}

void CompileAndExecuteCFile(const char* filename) {
    // Compile the C file into an executable named 'output.exe'
    char command[512];
    
    // Create the output file path based on the input file
    char outputFile[256];
    snprintf(outputFile, sizeof(outputFile), "output_%d.exe", currentGridIndex);

    // Format the compile command
    snprintf(command, sizeof(command), "gcc %s -o %s 2> compile_errors.txt", filename, outputFile);

    // Run the compile command synchronously to wait for compilation to complete
    system(command);

    // Check if the compilation was successful before running
    FILE *file = fopen("compile_errors.txt", "r");
    if (file) {
        // Get file size to check if there are any errors
        fseek(file, 0, SEEK_END);
        long fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        
        if (fileSize == 0) {
            // No errors, so run the executable
            snprintf(command, sizeof(command), "%s", outputFile);
            RunCommandAsync(command);
        } else {
            // Display compilation errors to the user
            char errorLine[256];
            fgets(errorLine, sizeof(errorLine), file);
            // Display first error line
            DrawText(errorLine, 10, 560, 20, RED);
            printf("Compilation failed. Check 'compile_errors.txt' for details.\n");
            // Open the error file in notepad
            OpenScriptInNotepad("compile_errors.txt");
        }
        fclose(file);
    }
}

void CreateNewScript(int gridIndex, int scriptIndex) {
    selectedScriptIndex = scriptIndex;
    grids[gridIndex].scripts[scriptIndex].isEditing = true;

    if (isCFile) {
        snprintf(grids[gridIndex].scripts[scriptIndex].filename, sizeof(grids[gridIndex].scripts[scriptIndex].filename), 
                GAME_FILES_PATH "script%d.c", gridIndex * MAX_SCRIPTS + scriptIndex + 1);
    } else {
        snprintf(grids[gridIndex].scripts[scriptIndex].filename, sizeof(grids[gridIndex].scripts[scriptIndex].filename), 
                GAME_FILES_PATH "script%d.py", gridIndex * MAX_SCRIPTS + scriptIndex + 1);
    }
    grids[gridIndex].scripts[scriptIndex].isCFile = isCFile;

    if (!DoesFileExist(grids[gridIndex].scripts[scriptIndex].filename)) {
        // Initialize with appropriate template based on file type
        if (isCFile) {
            SaveScriptToFile(grids[gridIndex].scripts[scriptIndex].filename, 
                "#include <stdio.h>\n\nint main() {\n    printf(\"Hello, World!\\n\");\n    return 0;\n}\n");
        } else {
            SaveScriptToFile(grids[gridIndex].scripts[scriptIndex].filename, 
                "# Python Script Template\nprint(\"Hello, World!\")\n");
        }
    }
    
    // Load the script content into memory
    LoadScriptFromFile(grids[gridIndex].scripts[scriptIndex].filename, 
                      grids[gridIndex].scripts[scriptIndex].text, 
                      MAX_TEXT_LENGTH);
    
    // Open the script in notepad for editing
    OpenScriptInNotepad(grids[gridIndex].scripts[scriptIndex].filename);
}

void CreateDescriptionFile(const char* scriptFilename) {
    char descriptionFilename[256];
    snprintf(descriptionFilename, sizeof(descriptionFilename), "%s_description.txt", scriptFilename);

    if (!DoesFileExist(descriptionFilename)) {
        FILE* file = fopen(descriptionFilename, "w");
        if (file) {
            fprintf(file, "Description for %s\n\nEnter your description here.\n", scriptFilename);
            fclose(file);
        }
    }
    
    // Open the description file in notepad for editing
    OpenScriptInNotepad(descriptionFilename);
}

void SwitchGrid(int gridID) {
    if (gridID >= 0 && gridID < MAX_GRIDS) {
        currentGridIndex = gridID;
        selectedScriptIndex = -1;
    }
}

void ToggleScriptMode() {
    // Toggle between Python mode and C mode
    isCFile = !isCFile;
    
    // Reset the selected script index when changing modes
    selectedScriptIndex = -1;
}

void ToggleEditMode() {
    isEditingMode = !isEditingMode;
}

void DrawHelpMenu() {
    int helpX = 20;
    int helpY = 100;
    DrawRectangle(10, 90, 780, 400, LIGHTGRAY);
    DrawText("HELP MENU", helpX, helpY, 30, DARKGRAY);
    DrawText("1. Grid Navigation: Use LEFT and RIGHT arrow keys to switch grids.", helpX, helpY + 40, 20, DARKGRAY);
    DrawText("2. Python/C Mode Toggle: Press '1' to switch between Python and C modes.", helpX, helpY + 70, 20, DARKGRAY);
    DrawText("3. Edit/Execute Mode Toggle: Press '2' to toggle between Editing and Executing modes.", helpX, helpY + 100, 20, DARKGRAY);
    DrawText("4. Creating Scripts: Click on a grid button to create a new script in the selected mode.", helpX, helpY + 130, 20, DARKGRAY);
    DrawText("5. Editing Scripts: Click on an existing script button to edit the script in Notepad.", helpX, helpY + 160, 20, DARKGRAY);
    DrawText("6. Saving Scripts: Press 'S' to save the currently opened script.", helpX, helpY + 190, 20, DARKGRAY);
    DrawText("7. Executing Scripts: Press 'E' to execute the current script (Python or compile C).", helpX, helpY + 220, 20, DARKGRAY);
    DrawText("8. Creating Descriptions: Right-click a script button to create a description file.", helpX, helpY + 250, 20, DARKGRAY);
    DrawText("9. Existing Scripts: Green buttons indicate scripts that exist in 'gameFiles'.", helpX, helpY + 280, 20, DARKGRAY);
    DrawText("10. Toggle Help Menu: Press 'H' to show or hide this help menu.", helpX, helpY + 310, 20, DARKGRAY);
}

void DrawFeedbackPanel() {
    int panelX = 620;
    int panelY = 10;
    DrawRectangle(panelX - 10, panelY - 10, 190, 150, LIGHTGRAY);
    DrawText("FEEDBACK PANEL", panelX, panelY, 20, DARKGRAY);
    DrawText(TextFormat("Current Grid: %d", currentGridIndex + 1), panelX, panelY + 30, 20, DARKGRAY);
    DrawText(TextFormat("Mode: %s", isCFile ? "C" : "Python"), panelX, panelY + 50, 20, DARKGRAY);
    DrawText(TextFormat("Edit Mode: %s", isEditingMode ? "Editing" : "Executing"), panelX, panelY + 70, 20, DARKGRAY);

    if (selectedScriptIndex != -1) {
        DrawText(TextFormat("Selected Script: %s", grids[currentGridIndex].scripts[selectedScriptIndex].filename), panelX, panelY + 90, 20, DARKGRAY);
        if (DoesFileExist(grids[currentGridIndex].scripts[selectedScriptIndex].filename)) {
            DrawText("File Status: Exists", panelX, panelY + 110, 20, GREEN);
        } else {
            DrawText("File Status: New", panelX, panelY + 110, 20, RED);
        }
    }
}

void InitializeAndLoadExistingScripts() {
    // First make sure the gameFiles directory exists
    #ifdef _WIN32
    _mkdir(GAME_FILES_PATH);
    #else
    mkdir(GAME_FILES_PATH, 0777);
    #endif

    for (int i = 0; i < MAX_GRIDS; i++) {
        grids[i].gridID = i;
        for (int j = 0; j < MAX_SCRIPTS; j++) {
            grids[i].scripts[j].isEditing = false;
            grids[i].scripts[j].text[0] = '\0';
            
            // Check for Python scripts
            char pythonFilename[100];
            snprintf(pythonFilename, sizeof(pythonFilename), GAME_FILES_PATH "script%d.py", i * MAX_SCRIPTS + j + 1);
            
            // Check for C scripts
            char cFilename[100];
            snprintf(cFilename, sizeof(cFilename), GAME_FILES_PATH "script%d.c", i * MAX_SCRIPTS + j + 1);
            
            // Set filename based on what exists
            if (DoesFileExist(pythonFilename)) {
                strcpy(grids[i].scripts[j].filename, pythonFilename);
                grids[i].scripts[j].isCFile = false;
                LoadScriptFromFile(grids[i].scripts[j].filename, grids[i].scripts[j].text, MAX_TEXT_LENGTH);
            } else if (DoesFileExist(cFilename)) {
                strcpy(grids[i].scripts[j].filename, cFilename);
                grids[i].scripts[j].isCFile = true;
                LoadScriptFromFile(grids[i].scripts[j].filename, grids[i].scripts[j].text, MAX_TEXT_LENGTH);
            } else {
                // Set a default empty filename based on current mode
                if (isCFile) {
                    snprintf(grids[i].scripts[j].filename, sizeof(grids[i].scripts[j].filename), 
                            GAME_FILES_PATH "script%d.c", i * MAX_SCRIPTS + j + 1);
                } else {
                    snprintf(grids[i].scripts[j].filename, sizeof(grids[i].scripts[j].filename), 
                            GAME_FILES_PATH "script%d.py", i * MAX_SCRIPTS + j + 1);
                }
                grids[i].scripts[j].isCFile = isCFile;
            }
        }
    }
}

int main() {
    // Set the window to be resizable
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    // Initialize window
    InitWindow(800, 600, "Script Editor - press h for Help Menu");
    MaximizeWindow();  // Maximize window on startup
    SetTargetFPS(60);

    // Ensure the game files directory exists
    #ifdef _WIN32
    _mkdir(GAME_FILES_PATH);
    #else
    mkdir(GAME_FILES_PATH, 0777);
    #endif

    // Initialize grids and scripts
    InitializeAndLoadExistingScripts();

    // Main game loop
    while (!WindowShouldClose()) {
        Vector2 mousePosition = GetMousePosition();
        int screenWidth = GetScreenWidth();
        int screenHeight = GetScreenHeight();

        // Handle grid navigation with arrow keys
        if (IsKeyPressed(KEY_RIGHT)) {
            SwitchGrid((currentGridIndex + 1) % MAX_GRIDS);
        }
        if (IsKeyPressed(KEY_LEFT)) {
            SwitchGrid((currentGridIndex - 1 + MAX_GRIDS) % MAX_GRIDS);
        }

        // Toggle Python/C mode
        if (IsKeyPressed(KEY_ONE)) {
            ToggleScriptMode();  // Switch between Python and C grids
        }

        // Toggle Edit/Execute mode
        if (IsKeyPressed(KEY_TWO)) {
            ToggleEditMode();
        }

        // Toggle Help Menu
        if (IsKeyPressed(KEY_H)) {
            showHelpMenu = !showHelpMenu;
        }

        // Check for button clicks to create or edit scripts
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            int xIndex = mousePosition.x / BUTTON_SIZE;
            int yIndex = mousePosition.y / BUTTON_SIZE;
            int index = yIndex * GRID_COLS + xIndex;

            if (xIndex < GRID_COLS && yIndex < GRID_ROWS) {
                if (index < MAX_SCRIPTS) {
                    selectedScriptIndex = index;
                    
                    // Check if the script exists in the opposite mode
                    char altFilename[100];
                    if (isCFile) {
                        snprintf(altFilename, sizeof(altFilename), GAME_FILES_PATH "script%d.py", 
                                currentGridIndex * MAX_SCRIPTS + index + 1);
                    } else {
                        snprintf(altFilename, sizeof(altFilename), GAME_FILES_PATH "script%d.c", 
                                currentGridIndex * MAX_SCRIPTS + index + 1);
                    }
                    
                    // If the current mode script doesn't exist but the opposite does
                    if (!DoesFileExist(grids[currentGridIndex].scripts[index].filename) && DoesFileExist(altFilename)) {
                        // Update the filename to use the existing script but maintain the current mode setting
                        strcpy(grids[currentGridIndex].scripts[index].filename, altFilename);
                        grids[currentGridIndex].scripts[index].isCFile = !isCFile;
                    }
                    
                    if (!DoesFileExist(grids[currentGridIndex].scripts[index].filename)) {
                        CreateNewScript(currentGridIndex, index);
                    } else if (isEditingMode) {
                        // Only open in Notepad if in editing mode
                        grids[currentGridIndex].scripts[index].isEditing = true;
                        OpenScriptInNotepad(grids[currentGridIndex].scripts[index].filename);
                    } else {
                        // If in execution mode, run the script
                        if (grids[currentGridIndex].scripts[index].isCFile) {
                            CompileAndExecuteCFile(grids[currentGridIndex].scripts[index].filename);
                        } else {
                            ExecutePythonScript(grids[currentGridIndex].scripts[index].filename);
                        }
                    }
                }
            }
        }

        // Handle right-click to create a description file
        if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
            int xIndex = mousePosition.x / BUTTON_SIZE;
            int yIndex = mousePosition.y / BUTTON_SIZE;
            int index = yIndex * GRID_COLS + xIndex;

            if (xIndex < GRID_COLS && yIndex < GRID_ROWS && index < MAX_SCRIPTS) {
                if (DoesFileExist(grids[currentGridIndex].scripts[index].filename)) {
                    CreateDescriptionFile(grids[currentGridIndex].scripts[index].filename);
                } else {
                    // Create the script first if it doesn't exist
                    CreateNewScript(currentGridIndex, index);
                    CreateDescriptionFile(grids[currentGridIndex].scripts[index].filename);
                }
            }
        }

        // Handle keyboard shortcuts for selected script
        if (selectedScriptIndex != -1) {
            Script* currentScript = &grids[currentGridIndex].scripts[selectedScriptIndex];
            
            // Handle reload - refresh script content from file
            if (IsKeyPressed(KEY_R)) {
                if (DoesFileExist(currentScript->filename)) {
                    LoadScriptFromFile(currentScript->filename, currentScript->text, MAX_TEXT_LENGTH);
                }
            }
            
            // Execute script with E key
            if (IsKeyPressed(KEY_E)) {
                if (DoesFileExist(currentScript->filename)) {
                    if (currentScript->isCFile) {
                        CompileAndExecuteCFile(currentScript->filename);
                    } else {
                        ExecutePythonScript(currentScript->filename);
                    }
                }
            }
        }

        // Draw grid and UI
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if (showHelpMenu) {
            DrawHelpMenu();  // Draw help menu if the flag is set
        } else {
            // Draw grid title and ID
            DrawText(TextFormat("Grid ID: %d", currentGridIndex + 1), 10, 10, 20, DARKGRAY);
            
            // Draw script buttons
            for (int y = 0; y < GRID_ROWS; y++) {
                for (int x = 0; x < GRID_COLS; x++) {
                    int index = y * GRID_COLS + x;
                    Rectangle button = { x * BUTTON_SIZE, y * BUTTON_SIZE, BUTTON_SIZE, BUTTON_SIZE };
                    
                    // Default button color is light gray
                    Color buttonColor = LIGHTGRAY; 
                    Color textColor = DARKGRAY;

                    Script* script = &grids[currentGridIndex].scripts[index];
                    
                    // Determine button color based on script status
                    if (DoesFileExist(script->filename)) {
                        if (script->isCFile) {
                            buttonColor = YELLOW;  // Yellow for C scripts
                        } else {
                            buttonColor = PURPLE;  // Purple for Python scripts
                        }
                        
                        // Add different shade for currently selected script
                        if (index == selectedScriptIndex) {
                            buttonColor = ColorBrightness(buttonColor, 0.7f);  // Brighten the color
                            textColor = BLACK;  // Make text more visible
                        }
                    } else if (index == selectedScriptIndex) {
                        buttonColor = ORANGE;  // Orange for selected but non-existent script
                    }

                    // Draw button with the determined color
                    DrawRectangleRec(button, buttonColor);
                    DrawRectangleLinesEx(button, 2, DARKGRAY);

                    // Draw script number and status
                    int scriptNumber = currentGridIndex * MAX_SCRIPTS + index + 1;
                    
                    // Draw script indicator
                    if (DoesFileExist(script->filename)) {
                        char fileTypeIndicator[2];
                        if (script->isCFile) {
                            fileTypeIndicator[0] = 'C';
                        } else {
                            fileTypeIndicator[0] = 'P';  // Python
                        }
                        fileTypeIndicator[1] = '\0';
                        
                        DrawText(fileTypeIndicator, button.x + BUTTON_SIZE - 20, button.y + 5, 20, textColor);
                    }
                    
                    // Draw script number
                    DrawText(TextFormat("%d", scriptNumber), button.x + 5, button.y + 5, 20, textColor);
                    
                    // Draw script name or status
                    if (index == selectedScriptIndex && script->isEditing) {
                        DrawText("Editing...", button.x + 5, button.y + BUTTON_SIZE - 25, 20, textColor);
                    } else if (DoesFileExist(script->filename)) {
                        // Show abbreviated filename (just the number part)
                        char* lastSlash = strrchr(script->filename, '/');
                        if (lastSlash) {
                            DrawText(lastSlash + 1, button.x + 5, button.y + BUTTON_SIZE - 25, 16, textColor);
                        } else {
                            DrawText(script->filename, button.x + 5, button.y + BUTTON_SIZE - 25, 16, textColor);
                        }
                    }
                }
            }

            // Draw feedback panel in the top right corner
            int panelX = screenWidth - 200;
            int panelY = 10;
            DrawRectangle(panelX - 10, panelY - 10, 190, 190, LIGHTGRAY);  // Panel background
            DrawText("FEEDBACK PANEL", panelX, panelY, 20, DARKGRAY);
            DrawText(TextFormat("Current Grid: %d", currentGridIndex + 1), panelX, panelY + 30, 20, DARKGRAY);

            // Draw mode indicator with background color
            Rectangle modeRect = { panelX, panelY + 50, 80, 30 };
            if (isCFile) {
                DrawRectangleRec(modeRect, YELLOW);
                DrawText("C", panelX + 10, panelY + 55, 20, BLACK);
            } else {
                DrawRectangleRec(modeRect, PURPLE);
                DrawText("Python", panelX + 10, panelY + 55, 20, WHITE);
            }

            // Draw edit/execute mode indicator
            Rectangle editRect = { panelX, panelY + 90, 150, 30 };
            if (isEditingMode) {
                DrawRectangleRec(editRect, GREEN);
                DrawText("Editing Mode", panelX + 10, panelY + 95, 20, BLACK);
            } else {
                DrawRectangleRec(editRect, RED);
                DrawText("Execution Mode", panelX + 10, panelY + 95, 20, WHITE);
            }

            // Draw selected script info
            if (selectedScriptIndex != -1) {
                Script* selectedScript = &grids[currentGridIndex].scripts[selectedScriptIndex];
                
                DrawText("Selected Script:", panelX, panelY + 130, 16, DARKGRAY);
                
                // Get script basename
                char* filename = selectedScript->filename;
                char* lastSlash = strrchr(filename, '/');
                char* displayName = lastSlash ? lastSlash + 1 : filename;
                
                DrawText(displayName, panelX, panelY + 150, 16, BLUE);
                
                if (DoesFileExist(selectedScript->filename)) {
                    DrawText("File Status: Exists", panelX, panelY + 170, 16, GREEN);
                } else {
                    DrawText("File Status: Not Created", panelX, panelY + 170, 16, RED);
                }
            }
            
            // Draw keyboard shortcut help at the bottom
            DrawRectangle(0, screenHeight - 30, screenWidth, 30, LIGHTGRAY);
            DrawText("KEYBOARD: [H]elp [1]Toggle Mode [2]Toggle Edit/Execute [E]xecute [R]eload [LEFT/RIGHT] Change Grid", 
                    10, screenHeight - 25, 20, DARKGRAY);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}