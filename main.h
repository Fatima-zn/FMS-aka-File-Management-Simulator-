// F U N C T I O N S



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
        else if(isCreated == 1) DrawTextEx(customFont, "FILENAME USED", (Vector2){SCREEN_WIDTH/2 - 60, 30}, 32, 1, RED);
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
                int updated = UpdateMeta(ms, fileName, data, -1, -1, -1, -1, temp, 0, temp, 0, 0);
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
        else if(updated == 1) DrawTextEx(customFont, "FILENAME USED", (Vector2){SCREEN_WIDTH/2 - 60, 30}, 32, 1, RED);
    }
    DrawTextEx(customFont, "New Name: ", (Vector2){(int)textBox.x + 5, (int)textBox.y - 30}, 20, 1, BLACK);

    DrawRectangleRec(textBox, LIGHTGRAY);
    if (*mouseOnText) DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, BLUE);
    else DrawRectangleLines((int)textBox.x, (int)textBox.y, (int)textBox.width, (int)textBox.height, DARKGRAY);
    //Display current input state
    DrawTextEx(customFont, data, (Vector2){(int)textBox.x + 5, (int)textBox.y + 8}, 32, 1, BLACK);

}