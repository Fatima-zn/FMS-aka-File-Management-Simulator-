#define RAYGUI_IMPLEMENTATION
#include <raylib.h>
#include <raygui.h>
#include "ms.h"
#include "file_manipulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//Screen Dimensions
#define SCREEN_WIDTH 1300
#define SCREEN_HEIGHT 850
#define SIDE_PANE_WIDTH 350


//Colors to int
#define TRANSPARENT_INT 0x00000000


//Colors
Color backgroundColor = LIGHTGRAY;
Color sidePanelColor = DARKBLUE;
Color buttonColor = DARKBLUE;
Color fileTileColor =  WHITE;
Color hoverColor = (Color){173, 216, 230, 255}; //LIGHTBLUE
Color selectedColor = (Color){135, 206, 250, 255}; //LIGHTSKYBLUE


//File VARs
#define MAX_CHARS 21 
bool isUploaded = false; 
char* filePath = NULL;
//Insertion
int switchCaseVar = 0;// 0: Default (Display Table)| 1: Rename| 2: Insertion| 3: Search| 4: Delete
bool isDrawn = false;
bool elementNotFoundIndicator = false;

bool isMemoryStateDisplayed = false;
int isInserted = -2;
int isCreated = -2;
int updated = 1;

char rslt0[5];
char rslt1[5];


//S T R U C T U R E S
typedef struct{
    Rectangle tile;
    char fileName[51];
    int sizeBloc;
    int sizeRecord;
    int firstAddress;
    int contigue;
    int ordered;
    bool isSelected;
} FileTile;

// F U N C T I O N S
void DrawSidePanel(FILE* ms, Font customFontBold);
void DrawFileTiles(FileTile file[], int nbr_file, int *selectedFileNum, Font customFontBold);
void DisplayFileInfo(FILE* ms, FileTile *files, int *selectedFilesNum, Font customFont, Font customFontBold);
void addFile(FILE* ms, FileTile file[], int *nbr_file, char data[][22], int *letterCount, Rectangle *textBox, bool *mouseOnText, bool *showTextInput, Font customFont);
bool IsAnyKeyPressed();
char* HandleFileDrop(int *fileCounter);
void InsertElement(FILE* ms, char *fileName,char data[][MAX_LINE_SIZE], int *letterCount, Rectangle *textBox, bool *mouseOnText, bool *showTextInput, Font customFont);
void SearchElement(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont);
void DeleteElement(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont);
void RenameFile(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont);
void DisplayMemoryState(FILE* ms, Font customFont, Font customFontBold);
int* getNbrOfElementsInBloc(FILE* ms);
char** getMetaMap(FILE* ms);
int getNbrOfFiles(FILE* ms);
void getFileInformation(FILE*ms, FileTile* files);


int main(){
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "FMS");

    //Open File
    FILE* ms = fopen("ms.bin", "rb+");
    if(ms == NULL){
        printf("Error opening file!\n");
    }



    //Font
    Font customFont = LoadFontEx("Poppins-Regular.ttf", 64, NULL, 0);
    Font customFontBold = LoadFontEx("Poppins-Bold.ttf", 64, NULL, 0);
    GuiSetFont(customFont); //For Components

    
    //Text Field VARs
    char data[4][MAX_CHARS + 1] = {"\0"}; 
    int letterCount[4] = {0}; 
    Rectangle textBox[4] = {
        {SCREEN_WIDTH/2 - 175, 170, 350, 50},
        {SCREEN_WIDTH/2 - 175, 270, 350, 50},
        {SCREEN_WIDTH/2 - 175, 370, 350, 50},
        {SCREEN_WIDTH/2 - 175, 470, 350, 50}
    }; 
    bool mouseOnText[4] = {false};
    bool showTextInput = false;

    //Upload Button pressed
    bool uploadButtonPressed = false;

    //Insertion Fields
    bool InsertionActivated = false;
    char insertData[2][MAX_LINE_SIZE] = {"\0"};
    int letterCounter[2] = {0};
    Rectangle textBoxI[2] = {
        {SCREEN_WIDTH/2 - 175, 170, 350, 50},
        {SCREEN_WIDTH/2 - 375, 270, 800, 50}
    }; 
    bool mouseOn[2] = {false};

    //Search Fields
    bool SearchActivated = false;
    char searchData[5] = {"\0"};
    int searchLetterCount = 0;
    Rectangle searchBox = {SCREEN_WIDTH/2 - 175, 370, 350, 50}; 
    bool mouseOnSearch = false;



    //Delete Fields
    bool DeleteActivated = false;
    char deleteData[5] = {"\0"};
    int deleteLetterCount = 0;
    Rectangle deleteBox = {SCREEN_WIDTH/2 - 175, 370, 350, 50}; 
    bool mouseOnDelete = false;

    //Rename Fields
    bool RenameActivated = false;
    char renameData[21] = {"\0"};
    int renameLetterCount = 0;
    Rectangle renameBox = {SCREEN_WIDTH/2 - 175, 370, 350, 50}; 
    bool mouseOnRename = false;




    //VAR
    int fileCounter = getNbrOfFiles(ms);
    FileTile* files = (FileTile*)malloc(fileCounter * sizeof(FileTile));
    getFileInformation(ms, files);
    

    int selectedFile = -1;


    //Button Style
        GuiSetStyle(BUTTON, BORDER_COLOR_NORMAL, ColorToInt(buttonColor));
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, ColorToInt(buttonColor));

        //Focused State
        GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, ColorToInt(buttonColor));
        GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, ColorToInt(buttonColor));

        //Pressed State
        GuiSetStyle(BUTTON, BORDER_COLOR_PRESSED, ColorToInt(buttonColor));
        GuiSetStyle(BUTTON, BASE_COLOR_PRESSED, ColorToInt(buttonColor));

    //Test Style
        GuiSetStyle(DEFAULT, TEXT_SIZE, 30);
        GuiSetStyle(BUTTON, TEXT_COLOR_NORMAL, ColorToInt(WHITE));
        GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(WHITE));
        GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, ColorToInt(WHITE));
        GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, ColorToInt(LIGHTGRAY)); 
        GuiSetStyle(BUTTON, TEXT_COLOR_PRESSED, ColorToInt(LIGHTGRAY));  



    while(!WindowShouldClose()){
        //UPDATE
        fileCounter = getNbrOfFiles(ms);

        if(showTextInput){
            if (GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, SCREEN_HEIGHT - 100, 350, 50}, "Upload File")){
                uploadButtonPressed = true;
    
                filePath = HandleFileDrop(&fileCounter); 
                if (filePath != NULL){ 
                    printf("File dropped: %s\n", filePath); 
                    isUploaded = true; 
                } 
            }
        }




        //DRAW
        BeginDrawing();
            ClearBackground(backgroundColor);
            


            if(selectedFile == -1){

                if(!showTextInput){
                    if(GuiButton((Rectangle){1100, 50, 150, 50}, "Add File")){
                        showTextInput = true;
                    }
                }

                if(showTextInput){

                    addFile(ms, files, &fileCounter, data, letterCount, textBox, mouseOnText, &showTextInput, customFont);
                    if(uploadButtonPressed){
                        DrawTextEx(customFont, "Drag and Drop your file here then PRESS the button below to upload (.csv only)", (Vector2){210, SCREEN_HEIGHT - 150}, 30, 1, BLACK);
                    }

                }else{
                    DrawTextEx(customFontBold, "File Management Simulator", (Vector2){360, 30}, 42, 1, BLACK);
                
                    DrawSidePanel(ms, customFontBold);

                    if(!isMemoryStateDisplayed){
                        DrawFileTiles(files, fileCounter, &selectedFile, customFontBold);
                    }else{
                        DisplayMemoryState(ms, customFont, customFontBold);
                    }
                    
                }

            } else{
                switch(switchCaseVar){
                    case 1:{
                        RenameActivated = true;
                        RenameFile(ms, files[selectedFile].fileName, renameData, &renameLetterCount, renameBox, &mouseOnRename, &RenameActivated, customFont);
                    } break;
                    case 2:{
                        InsertionActivated = true;
                        InsertElement(ms, files[selectedFile].fileName, insertData, letterCounter, textBoxI, mouseOn, &InsertionActivated, customFont);
                    } break;
                    case 3:{
                        SearchActivated = true;
                        SearchElement(ms, files[selectedFile].fileName, searchData, &searchLetterCount, searchBox, &mouseOnSearch, &SearchActivated, customFont);
                        if(isDrawn){
                            DrawTextEx(customFont, "Search Result", (Vector2){SCREEN_WIDTH/2 - 120, 30}, 32, 1, BLACK);

                            DrawTextEx(customFont, "Bloc Position: ", (Vector2){SCREEN_WIDTH/2 - 160, 100}, 30, 1, BLACK);
                            DrawTextEx(customFont, rslt0, (Vector2){SCREEN_WIDTH/2 + 100, 100}, 30, 1, BLACK);

                            DrawTextEx(customFont, "Element Position: ", (Vector2){SCREEN_WIDTH/2 - 160, 170}, 30, 1, BLACK);
                            DrawTextEx(customFont, rslt1, (Vector2){SCREEN_WIDTH/2 + 100, 170}, 30, 1, BLACK);
                        }
                        if(elementNotFoundIndicator){
                            DrawTextEx(customFont, "Element Not Found!", (Vector2){SCREEN_WIDTH/2 - 120, SCREEN_HEIGHT/2 - 100}, 32, 1, RED);
                        }
                    } break;
                    case 4:{
                        DeleteActivated = true;
                        DeleteElement(ms, files[selectedFile].fileName, deleteData, &deleteLetterCount, deleteBox, &mouseOnDelete, &DeleteActivated, customFont);
                    }break;
                    default: {
                        DisplayFileInfo(ms, files, &selectedFile, customFont, customFontBold);
                    } break;
                }
 
            }

  
        EndDrawing();
    }


    UnloadFont(customFont);
    fclose(ms);
    CloseWindow();
    return 0;
}



void DrawSidePanel(FILE *ms, Font customFontBold){
    DrawRectangle(0, 0, SIDE_PANE_WIDTH, SCREEN_HEIGHT, sidePanelColor);    

    //Save current styles  
        int prevBaseColorNormal = GuiGetStyle(BUTTON, BASE_COLOR_NORMAL);    
    //Set New Styles
        GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, TRANSPARENT_INT); 

    DrawTextEx(customFontBold, "Actions", (Vector2){20, 20}, 42, 1, WHITE);
    if(GuiButton((Rectangle){30, 100, 250, 70}, "Initialise MS")) initMS(ms);
    if(GuiButton((Rectangle){30, 170, 250, 70}, "Display MS State")){
        isMemoryStateDisplayed = true;
    }
    if(GuiButton((Rectangle){30, 240, 250, 70}, "Compact the MS")) Compact(ms);
    if(GuiButton((Rectangle){30, 310, 250, 70}, "Empty MS")) deleteMS(ms);

    //Set Previous Styles
    GuiSetStyle(BUTTON, BASE_COLOR_NORMAL, prevBaseColorNormal);

    if(isMemoryStateDisplayed){
        if(GuiButton((Rectangle){SIDE_PANE_WIDTH + 60, SCREEN_HEIGHT - 150, 200, 70}, "Back")) isMemoryStateDisplayed = false;
    }
}


void DrawFileTiles(FileTile file[], int nbr_file, int *selectedFileNum, Font customFontBold){
    Vector2 mousePosition = GetMousePosition();

    for(int i = 0; i < nbr_file; i++){
        Color tileColor = fileTileColor;

        //ADDED
        file[i].tile = (Rectangle){SIDE_PANE_WIDTH + 20, 130 + (i * 50), 890, 50};

        if(CheckCollisionPointRec(mousePosition, file[i].tile)){
            tileColor = selectedColor;

            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                *selectedFileNum = i;
            }
        }
        
        DrawRectangleRec(file[i].tile, tileColor);


        DrawTextEx(customFontBold, file[i].fileName, (Vector2){file[i].tile.x + 15, file[i].tile.y + 5}, 32, 1, BLACK);
    }
}






void DisplayFileInfo(FILE* ms, FileTile *files, int *selectedFilesNum, Font customFont, Font customFontBold){ 
    //TITLE
    DrawTextEx(customFontBold, "File Information", (Vector2){SCREEN_WIDTH / 2 - MeasureText("File Information", 20) / 2, 50}, 42, 1, BLACK);


    //Actions
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 170, 250, 50}, "Rename")) switchCaseVar = 1;
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 240, 250, 50}, "Add Element")) switchCaseVar = 2;
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 310, 250, 50}, "Search Element")) switchCaseVar = 3;
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 380, 250, 50}, "Delete Element")) switchCaseVar = 4;
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 450, 250, 50}, "Defragment File")) defragmentation(ms, files[*selectedFilesNum].fileName);
    if(GuiButton((Rectangle){SCREEN_WIDTH - 300, 520, 250, 50}, "Delete File")){
        //Delete File
        int temp[2];
        temp[0]=2;
        UpdateMeta(ms, files[*selectedFilesNum].fileName, "", -1, -1, -1, -1, temp, 0, temp, 0, 1);
    }


    //Draw the table 
    #define NUM_ROWS 6
    #define NUM_COLUMNS 2
    #define CELL_WIDTH 350
    #define CELL_HEIGHT 60

    //VARs
    char filename[21];
    char sizeBloc[3];
    char sizeRecord[4];
    char address[3];
    char GlobalOrg[15];
    char InternalOrg[15];

    sprintf(sizeBloc, "%d", files[*selectedFilesNum].sizeBloc);
    sprintf(sizeRecord, "%d", files[*selectedFilesNum].sizeRecord);
    sprintf(address, "%d", files[*selectedFilesNum].firstAddress);
    strcpy(GlobalOrg, files[*selectedFilesNum].contigue == 1? "Contiguous" : "Chained");
    strcpy(InternalOrg, files[*selectedFilesNum].ordered == 1? "Ordered" : "Non-Ordered");
    strcpy(filename, files[*selectedFilesNum].fileName);

    



    const char *list[] = {"File name ", filename,"Size (Blocs) ", sizeBloc,"Size (Records) ", sizeRecord,"First Bloc Address ", address,"Global Organization Mode ", GlobalOrg, "Internal Organization Mode" , InternalOrg};
    int listCounter = 0;
    int heightOffset = 160;
    for (int row = 0; row < NUM_ROWS; row++){ 
        for (int col = 0; col < NUM_COLUMNS; col++){ 
            Rectangle cellRect = {100 + col * CELL_WIDTH, heightOffset + row * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT}; 
            DrawRectangleRec(cellRect, LIGHTGRAY); 
            DrawRectangleLinesEx(cellRect, 2, BLACK);  
            DrawTextEx(listCounter % 2 == 0? customFontBold : customFont, list[listCounter], (Vector2){cellRect.x + 10, cellRect.y + 15}, 30, 1, BLACK);                  
            listCounter++;          
        } 
        heightOffset++;
    }
    





    //Back Button 
    if (GuiButton((Rectangle){50, SCREEN_HEIGHT - 120, 85, 50}, "Back")){ 
        (*selectedFilesNum) = -1; 
        switchCaseVar = 0;
    } 

}


/*

void addFile(FILE* ms, FileTile file[], int *nbr_file, char data[][22], int *letterCount, Rectangle *textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    
    if(*showTextInput){
        //Detects if mouse is on the text field
        for(int i = 0; i < 4; i++){
            mouseOnText[i] = (CheckCollisionPointRec(GetMousePosition(), textBox[i]));

            if(mouseOnText[i]){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (letterCount[i] < MAX_CHARS)){
                        data[i][letterCount[i]] = (char)key;
                        data[i][letterCount[i] + 1] = '\0';
                        letterCount[i]++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    letterCount[i]--;
                    if (letterCount[i] < 0) letterCount[i] = 0;
                    data[i][letterCount[i]] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data[i]) == 0) allFieldsFilled = false;
        }



        if (GuiButton((Rectangle){SCREEN_WIDTH/2 + 50, 550, 300, 60}, "Create")){ 
            printf("nbr_file: %d, allFieldsFilled: %d, isUploaded: %d\n", *nbr_file, allFieldsFilled, isUploaded);
            if(*nbr_file < 15 && allFieldsFilled && isUploaded) {
                //Back part
                isCreated = Create_file(ms, fopen(filePath, "r"), data[0], atoi(data[1]), atoi(data[2]), atoi(data[3]));
                isDrawn = true;



                //UI part
                file[*nbr_file].tile = (Rectangle){SIDE_PANE_WIDTH + 20, 150 + (*nbr_file * 50), 250, 40}; 
                snprintf(file[*nbr_file].fileName, sizeof(file[*nbr_file].fileName), "%s", data[0]); 
                file[*nbr_file].isSelected = false; 
                printf("%s\n", data[0]);
                (*nbr_file)++; 
            }
        }
        if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 360, 550, 330, 60}, "Back")){
            //Reset text input field 
            *showTextInput = false; 
            isDrawn = false;
            for(int i = 0; i < 4; i++){
                letterCount[i] = 0; 
                data[i][22] = '\0'; 
            }
        }
    }


    //Drawing
    if(isDrawn){
        if(isCreated == 0) DrawTextEx(customFont, "File has been created", (Vector2){SCREEN_WIDTH/2 - 140, 30}, 32, 1, DARKGREEN);
        else if(isCreated == -1)DrawTextEx(customFont, "FUll MS", (Vector2){SCREEN_WIDTH/2 - 60, 30}, 32, 1, RED);
    }
    char list[4][28] = {"File name:", "Size (Record/Element):", "Global Organization Mode:", "Internal Organization Mode:"};
    for(int i = 0; i < 4; i++){
        DrawTextEx(customFont, list[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y - 30}, 20, 1, BLACK);

        DrawRectangleRec(textBox[i], LIGHTGRAY);
        if (mouseOnText[i]) DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, BLUE);
        else DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, DARKGRAY);
        //Display current input state
        DrawTextEx(customFont, data[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y + 8}, 32, 1, BLACK);
    }

 
}


*/

char* HandleFileDrop(int *fileCounter){
    char *droppedFilePath = NULL;

    if (IsFileDropped()) {
        FilePathList droppedFiles = LoadDroppedFiles();


        if (droppedFiles.count > 0 && IsFileExtension(droppedFiles.paths[0], ".csv") && *fileCounter < 15){
            droppedFilePath = droppedFiles.paths[0];
        }

        UnloadDroppedFiles(droppedFiles);
    }

    return droppedFilePath;
}



bool IsAnyKeyPressed(){
    bool keyPressed = false;
    int key = GetKeyPressed();

    if ((key >= 32) && (key <= 126)) keyPressed = true;

    return keyPressed;
}




void InsertElement(FILE* ms, char *fileName, char data[][MAX_LINE_SIZE], int *letterCount, Rectangle *textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    if(*showTextInput){
        //Detects if mouse is on the text field
        for(int i = 0; i < 2; i++){
            mouseOnText[i] = (CheckCollisionPointRec(GetMousePosition(), textBox[i]));

            if(mouseOnText[i]){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (letterCount[i] < MAX_LINE_SIZE)){
                        data[i][letterCount[i]] = (char)key;
                        data[i][letterCount[i] + 1] = '\0';
                        letterCount[i]++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    letterCount[i]--;
                    if (letterCount[i] < 0) letterCount[i] = 0;
                    data[i][letterCount[i]] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data[i]) == 0) allFieldsFilled = false;
        }




        if (GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 520, 350, 60}, "Insert")){ 
            //FUNCTION HERE
            if(allFieldsFilled){

                isInserted = insertElement(ms, fileName, atoi(data[0]), data[1]);
                isDrawn = true;
            }

        }
        if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 620, 350, 60}, "Back")){
            //Reset text input field 
            *showTextInput = false; 
            isDrawn = false;
            switchCaseVar = 0;
            *letterCount = 0;
            for(int i = 0; i < 2; i++){
                letterCount[i] = 0; 
                data[i][22] = '\0'; 
            }
        }

    }


    //Drawing
    if(isDrawn){
        switch(isInserted){
            case -1: DrawTextEx(customFont, "FULL MS", (Vector2){SCREEN_WIDTH/2 - 50, 30}, 32, 1, DARKGREEN); break;
            case 0: DrawTextEx(customFont, "Element inserted successfully", (Vector2){SCREEN_WIDTH/2 - 170, 30}, 32, 1, DARKGREEN); break;
            case 1: DrawTextEx(customFont, "Element already exists", (Vector2){SCREEN_WIDTH/2 - 150, 30}, 32, 1, DARKGREEN); break;
        }
    }


    char list[2][28] = {"ID:", "Data:"};
    for(int i = 0; i < 2; i++){
        DrawTextEx(customFont, list[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y - 30}, 20, 1, BLACK);

        DrawRectangleRec(textBox[i], LIGHTGRAY);
        if (mouseOnText[i]) DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, BLUE);
        else DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, DARKGRAY);
        //Display current input state
        DrawTextEx(customFont, data[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y + 8}, 32, 1, BLACK);
    }

 
}





void SearchElement(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;
 


    if(*showTextInput){
        //Detects if mouse is on the text field
            *mouseOnText = (CheckCollisionPointRec(GetMousePosition(), textBox));

            if(*mouseOnText){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (*letterCount < MAX_CHARS)){
                        data[*letterCount] = (char)key;
                        data[*letterCount + 1] = '\0';
                        (*letterCount)++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    (*letterCount)--;
                    if (*letterCount < 0) *letterCount = 0;
                    data[*letterCount] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data) == 0) allFieldsFilled = false;
    }




            //FUNCTION HERE
            if(GuiButton((Rectangle){SCREEN_WIDTH/2 + 50, 520, 300, 60}, "Search")){
                printf("Button pressed\n");
                if(allFieldsFilled){
                    printf("%d\n", allFieldsFilled);
                    searchResult result = searchElementByID(ms, fileName, atoi(data));
                    printf("Bloc position: %d, element position: %d\n", result.blocPosition, result.elementPosition);
                    if(result.isFound){
                        sprintf(rslt0, "%d", result.blocPosition);
                        sprintf(rslt1, "%d", result.elementPosition);
                        isDrawn = true;
                        elementNotFoundIndicator = false;

                    } else{
                        printf("Element not found\n");
                        isDrawn = false;
                        
                        elementNotFoundIndicator = true;
                    }
                }

            }
            if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 360, 520, 300, 60}, "Back")){
                //Reset text input field 
                *showTextInput = false; 
                switchCaseVar = 0;
                *letterCount = 0; 
                elementNotFoundIndicator = false;
                isDrawn = false;
                data = '\0'; 
                *rslt0 = '\0';
                *rslt1 = '\0';
            }




    //Drawing
    DrawTextEx(customFont, "ID: ", (Vector2){(int)textBox.x + 5, (int)textBox.y - 30}, 20, 1, BLACK);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (*mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLUE);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    //Display current input state
    DrawTextEx(customFont, data, (Vector2){(int)textBox.x + 5, (int)textBox.y + 8}, 32, 1, BLACK);
}



void DeleteElement(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    if(*showTextInput){
        //Detects if mouse is on the text field
            *mouseOnText = (CheckCollisionPointRec(GetMousePosition(), textBox));

            if(*mouseOnText){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (*letterCount < MAX_CHARS)){
                        data[*letterCount] = (char)key;
                        data[*letterCount + 1] = '\0';
                        (*letterCount)++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    (*letterCount)--;
                    if (*letterCount < 0) *letterCount = 0;
                    data[*letterCount] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data) == 0) allFieldsFilled = false;
    }



    if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 360, 520, 300, 60}, "Logical Suppression")){
        printf("Logical Suppression\n");

        //FUNCTION HERE
        if(allFieldsFilled){
            LogicalSuppression(ms, fileName, atoi(data));
            isDrawn = true;
        }

    } else if(GuiButton((Rectangle){SCREEN_WIDTH/2 + 50, 520, 300, 60}, "Physical Suppression")){
        printf("Physical Suppression\n");
        isDrawn = true;
                
        //FUNCTION HERE
        if(allFieldsFilled){
            PhysicalSuppression(ms, fileName, atoi(data));
        }

    }
    if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 200, 620, 350, 60}, "Back")){
        //Reset text input field 
        *showTextInput = false; 
        isDrawn = false;
        switchCaseVar = 0;
        *letterCount = 0; 
        data = '\0'; 
    }




    //Drawing
    if(isDrawn) DrawTextEx(customFont, "Element has been deleted successfully", (Vector2){SCREEN_WIDTH/2 - 220, 30}, 32, 1, DARKGREEN);

    DrawTextEx(customFont, "ID: ", (Vector2){(int)textBox.x + 5, (int)textBox.y - 30}, 20, 1, BLACK);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (*mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLUE);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    //Display current input state
    DrawTextEx(customFont, data, (Vector2){(int)textBox.x + 5, (int)textBox.y + 8}, 32, 1, BLACK);

}


/*

void RenameFile(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    if(*showTextInput){
        //Detects if mouse is on the text field
            *mouseOnText = (CheckCollisionPointRec(GetMousePosition(), textBox));

            if(*mouseOnText){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (*letterCount < MAX_CHARS)){
                        data[*letterCount] = (char)key;
                        data[*letterCount + 1] = '\0';
                        (*letterCount)++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    (*letterCount)--;
                    if (*letterCount < 0) *letterCount = 0;
                    data[*letterCount] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data) == 0) allFieldsFilled = false;
    }



        if (GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 520, 350, 60}, "Rename")){ 
            //FUNCTION HERE
            if(allFieldsFilled){
                int temp[2];
                temp[0] = 2;
                //Rename File
                UpdateMeta(ms, fileName, data, -1, -1, -1, -1, temp, 0, temp, 0, 0);
                isDrawn = true;
            }

        }
        if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 620, 350, 60}, "Back")){
            //Reset text input field 
            *showTextInput = false; 
            isDrawn = false;
            switchCaseVar = 0;
            *letterCount = 0; 
            data = '\0'; 
        }




    //Drawing
    if(isDrawn) DrawTextEx(customFont, "File renamed successfully", (Vector2){SCREEN_WIDTH/2 - 150, 30}, 32, 1, DARKGREEN);
    DrawTextEx(customFont, "New Name: ", (Vector2){(int)textBox.x + 5, (int)textBox.y - 30}, 20, 1, BLACK);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (*mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLUE);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    //Display current input state
    DrawTextEx(customFont, data, (Vector2){(int)textBox.x + 5, (int)textBox.y + 8}, 32, 1, BLACK);

}

*/


void DisplayMemoryState(FILE* ms, Font customFont, Font customFontBold){
    AllocationTableBuffer* Buffer = UploadAllocationTableToMC(ms);
    int* nbrOfElements = getNbrOfElementsInBloc(ms);
    char** metaMap = getMetaMap(ms);

    #define COL_NUM 5
    #define ROW_NUM 4
    #define CELL_W 175
    #define CELL_H 125


    int index = 0;
    int heightOffset = 160;
    for (int row = 0; row < ROW_NUM; row++){ 
        for (int col = 0; col < COL_NUM; col++){ 
            Rectangle rect = {400 + col * CELL_W, heightOffset + row * CELL_H, CELL_W, CELL_H}; 
            DrawRectangleRec(rect, Buffer->allocationTable[index].state? RED : GREEN); 
            DrawRectangleLinesEx(rect, 2, BLACK);  

            char blocIndex[21];
            sprintf(blocIndex, "Bloc %d", Buffer->allocationTable[index].nbr_bloc);
            DrawTextEx(customFontBold, blocIndex, (Vector2){rect.x + 10, rect.y + 15}, 30, 1, BLACK); 

            DrawTextEx(customFont, metaMap[index], (Vector2){rect.x + 10, rect.y + 40}, 28, 1, BLACK);  

            char nbr_e[4];
            sprintf(nbr_e, "%d element(s)", nbrOfElements[index]);
            DrawTextEx(customFont, nbr_e, (Vector2){rect.x + 10, rect.y + 70}, 28, 1, BLACK);  
            index++;              
        } 
        heightOffset++;
    }
}




int* getNbrOfElementsInBloc(FILE* ms){
    Bloc Buffer;
    AllocationTableBuffer* buffer = UploadAllocationTableToMC(ms);
    int* nbrOfElements = (int*)malloc(sizeof(int) * NBR_BLOC);
    for(int j = 0; j < NBR_BLOC; j++){
        nbrOfElements[j] = 0;
    }

    fseek(ms, sizeof(Bloc) * TOTAL_NON_DATA_BLOC, SEEK_SET);
    for(int i = 0; i < NBR_BLOC; i++){
        if(buffer->allocationTable[i].state == 1){
            fread(&Buffer, sizeof(Bloc), 1, ms);
            nbrOfElements[i] = Buffer.nbr_element;
            memset(&Buffer, 0, sizeof(Bloc));
        }

    }

    return nbrOfElements;
}


char** getMetaMap(FILE* ms){
    Bloc Buffer;
    char** metaMap = (char**)malloc(NBR_BLOC * sizeof(char*)); 
    for (int i = 0; i < NBR_BLOC; i++) { 
        metaMap[i] = (char*)malloc(21 * sizeof(char)); 
        strcpy(metaMap[i], "\0");
    }


    fseek(ms, sizeof(Bloc), SEEK_SET);
    for(int i = 0; i < TOTAL_NON_DATA_BLOC - 1; i++){
        fread(&Buffer, sizeof(Bloc), 1, ms);

        for(int j = 0; j < FB; j++){
            MetaBuffer* meta = ElementToMetaBufferPart(Buffer.element[j]);
            if(meta->flag == 1){
                if(meta->contigue){
                    int start = meta->first_address;
                    int end = meta->last_address;
                    while(start <= end){
                        strcpy(metaMap[start - TOTAL_NON_DATA_BLOC] , meta->name);
                        start++;
                    }
                } else{
                    Bloc buffer;
                    int start = meta->first_address;
                    while(-1 != start){
                        strcpy(metaMap[start - TOTAL_NON_DATA_BLOC], meta->name);

                        fseek(ms, sizeof(Bloc) * start, SEEK_SET);
                        fread(&buffer, sizeof(Bloc), 1, ms);
                        start = buffer.next;
                        memset(&buffer, 0, sizeof(Bloc));
                    }
                }
            }
        }
    }

    return metaMap;
}


void getFileInformation(FILE*ms, FileTile* files){
    Bloc Buffer;
    int counter = 0;

    fseek(ms, sizeof(Bloc), SEEK_SET);
    for(int i = 0; i < TOTAL_NON_DATA_BLOC - 1; i++){
        fread(&Buffer, sizeof(Bloc), 1, ms);

        for(int j = 0; j < FB; j++){
            MetaBuffer* meta = ElementToMetaBufferPart(Buffer.element[j]);
            if(meta->flag == 1){
                strcpy(files[counter].fileName, meta->name);
                files[counter].isSelected = false;
                files[counter].sizeBloc = meta->bloc;
                files[counter].sizeRecord = meta->record;
                files[counter].contigue = meta->contigue;
                files[counter].ordered = meta->ordered;
                files[counter].firstAddress = meta->first_address;

                counter++;
            }
        }
    }
}





int getNbrOfFiles(FILE* ms){
    Bloc meta;
    int nbr_files = 0;
    fseek(ms, sizeof(Bloc), SEEK_SET);

    for(int i = 0; i < TOTAL_NON_DATA_BLOC - 1; i++){
        fread(&meta, sizeof(Bloc), 1, ms);
        for(int j = 0; j < FB; j++){
            if(meta.element[j].flag == 1) nbr_files++;
        }
    }

    return nbr_files;
}



void addFile(FILE* ms, FileTile file[], int *nbr_file, char data[][22], int *letterCount, Rectangle *textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    
    if(*showTextInput){
        //Detects if mouse is on the text field
        for(int i = 0; i < 4; i++){
            mouseOnText[i] = (CheckCollisionPointRec(GetMousePosition(), textBox[i]));

            if(mouseOnText[i]){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (letterCount[i] < MAX_CHARS)){
                        data[i][letterCount[i]] = (char)key;
                        data[i][letterCount[i] + 1] = '\0';
                        letterCount[i]++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    letterCount[i]--;
                    if (letterCount[i] < 0) letterCount[i] = 0;
                    data[i][letterCount[i]] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data[i]) == 0) allFieldsFilled = false;
        }



        if (GuiButton((Rectangle){SCREEN_WIDTH/2 + 50, 550, 300, 60}, "Create")){ 
            printf("nbr_file: %d, allFieldsFilled: %d, isUploaded: %d\n", *nbr_file, allFieldsFilled, isUploaded);
            if(*nbr_file < 15 && allFieldsFilled && isUploaded) {
                //Back part
                isCreated = Create_file(ms, fopen(filePath, "r"), data[0], atoi(data[1]), atoi(data[2]), atoi(data[3]));
                isDrawn = true;



                //UI part
                file[*nbr_file].tile = (Rectangle){SIDE_PANE_WIDTH + 20, 150 + (*nbr_file * 50), 250, 40}; 
                snprintf(file[*nbr_file].fileName, sizeof(file[*nbr_file].fileName), "%s", data[0]); 
                file[*nbr_file].isSelected = false; 
                printf("%s\n", data[0]);
                (*nbr_file)++; 
            }
        }
        if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 360, 550, 330, 60}, "Back")){
            //Reset text input field 
            *showTextInput = false; 
            isDrawn = false;
            for(int i = 0; i < 4; i++){
                letterCount[i] = 0; 
                data[i][22] = '\0'; 
            }
        }
    }


    //Drawing
    if(isDrawn){
        if(isCreated == 0) DrawTextEx(customFont, "File has been created", (Vector2){SCREEN_WIDTH/2 - 140, 30}, 32, 1, DARKGREEN);
        else if(isCreated == -1)DrawTextEx(customFont, "FUll MS", (Vector2){SCREEN_WIDTH/2 - 60, 30}, 32, 1, RED);
        else if(isCreated == 1) DrawTextEx(customFont, "FILENAME USED", (Vector2){SCREEN_WIDTH/2 - 80, 30}, 32, 1, RED);
    }
    char list[4][28] = {"File name:", "Size (Record/Element):", "Global Organization Mode:", "Internal Organization Mode:"};
    for(int i = 0; i < 4; i++){
        DrawTextEx(customFont, list[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y - 30}, 20, 1, BLACK);

        DrawRectangleRec(textBox[i], LIGHTGRAY);
        if (mouseOnText[i]) DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, BLUE);
        else DrawRectangleLines((int)textBox[i].x, (int)textBox[i].y, (int)textBox[i].width, (int)textBox[i].height, DARKGRAY);
        //Display current input state
        DrawTextEx(customFont, data[i], (Vector2){(int)textBox[i].x + 5, (int)textBox[i].y + 8}, 32, 1, BLACK);
    }

 
}

void RenameFile(FILE* ms, char *fileName, char data[], int *letterCount, Rectangle textBox, bool *mouseOnText, bool *showTextInput, Font customFont){
    bool allFieldsFilled = true;

    if(*showTextInput){
        //Detects if mouse is on the text field
            *mouseOnText = (CheckCollisionPointRec(GetMousePosition(), textBox));

            if(*mouseOnText){
                SetMouseCursor(MOUSE_CURSOR_IBEAM);

                int key = GetCharPressed();

                while(key > 0){
                    if((key >= 32) && (key <= 125) && (*letterCount < MAX_CHARS)){
                        data[*letterCount] = (char)key;
                        data[*letterCount + 1] = '\0';
                        (*letterCount)++;
                    }
                    key = GetCharPressed();
                }
                if(IsKeyPressed(KEY_BACKSPACE)){
                    (*letterCount)--;
                    if (*letterCount < 0) *letterCount = 0;
                    data[*letterCount] = '\0';
                }
            } //else SetMouseCursor(MOUSE_CURSOR_DEFAULT);

            if (strlen(data) == 0) allFieldsFilled = false;
    }



        if (GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 520, 350, 60}, "Rename")){ 
            //FUNCTION HERE
            if(allFieldsFilled){
                int temp[2];
                temp[0] = 2;
                //Rename File
                updated = UpdateMeta(ms, fileName, data, -1, -1, -1, -1, temp, 0, temp, 0, 0);
                isDrawn = true;
            }

        }
        if(GuiButton((Rectangle){SCREEN_WIDTH/2 - 175, 620, 350, 60}, "Back")){
            //Reset text input field 
            *showTextInput = false; 
            isDrawn = false;
            switchCaseVar = 0;
            *letterCount = 0; 
            data = '\0'; 
        }




    //Drawing
    if(isDrawn){ 
        if(updated == 0) DrawTextEx(customFont, "File renamed successfully", (Vector2){SCREEN_WIDTH/2 - 150, 30}, 32, 1, DARKGREEN);
        else if(updated == 1) DrawTextEx(customFont, "FILENAME USED", (Vector2){SCREEN_WIDTH/2 - 80, 30}, 32, 1, RED);
    }
    DrawTextEx(customFont, "New Name: ", (Vector2){(int)textBox.x + 5, (int)textBox.y - 30}, 20, 1, BLACK);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (*mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLUE);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    //Display current input state
    DrawTextEx(customFont, data, (Vector2){(int)textBox.x + 5, (int)textBox.y + 8}, 32, 1, BLACK);

}


