#define RAYGUI_IMPLEMENTATION
#include <raylib.h>
#include <raygui.h>


// Define colors as integers 
#define RAYWHITE_INT 0xFFFFFFFF 
#define DARKGRAY_INT 0xA9A9A9FF 
#define RED_INT 0xFF0000FF
#define DARKBLUE_INT 0x00008BFF


typedef enum{
    TAB_ONE,
    TAB_TWO
}Tab;


int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "raylib example - custom font");

    //Load custom font
    Font customFont = LoadFont("custom_font.ttf");
    GuiSetFont(customFont);

    //Set text size for buttons (default is 10)
    GuiSetStyle(DEFAULT, TEXT_SIZE, 30);

    Tab currentTab = TAB_ONE;

/*
    GuiSetStyle(BUTTON, TEXT_COLOR_FOCUSED, RAYWHITE_INT);
    GuiSetStyle(BUTTON, BASE_COLOR_FOCUSED, DARKBLUE_INT);
    GuiSetStyle(BUTTON, BORDER_COLOR_FOCUSED, RED_INT);
*/


    while (!WindowShouldClose()) {
        //Update
        if (currentTab == TAB_ONE) {
            if (GuiButton((Rectangle){ screenWidth/2 - 100, screenHeight/2 - 90, 200, 50 }, "Go to Tab 2")){
                currentTab = TAB_TWO;
            }
        } else if(currentTab == TAB_TWO){
            if (GuiButton((Rectangle){ screenWidth/2 - 100, screenHeight/2 - 90, 200, 50 }, "Go to Tab 1")){
                currentTab = TAB_ONE;
            }
        }
        

        //Draw
        BeginDrawing();
        ClearBackground(RAYWHITE);

        if(currentTab == TAB_ONE){
            DrawTextEx(customFont, "This is Tab One", (Vector2){ screenWidth/2 - 85, screenHeight/2 + 30 }, customFont.baseSize, 2, DARKBLUE);
        } else if(currentTab == TAB_TWO){
            DrawTextEx(customFont, "This is Tab Two", (Vector2){ screenWidth/2 - 85, screenHeight/2 + 30 }, customFont.baseSize, 2, RED);
        }
        




        EndDrawing();
    }

    // Unload custom font
    UnloadFont(customFont);

    CloseWindow();

    return 0;
}


