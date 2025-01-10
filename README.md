
# File Management Simulator Setup Guide

1. **Ensure you have the necessary files**:
   - Place all Raylib-related files in a directory called `raylib` within your project directory.
   - The `raylib` folder should contain the following:
     - `libraylib.a`
     - `libraylibdll.a`
     - `raylib.dll`
     - `raylib.h`
     - `raymath.h`
     - `rlgl.h`

2. **Include Path Update**:
   Make sure your `main.c` file includes the Raylib header from the correct path:

   ```c
   #include "raylib/raylib.h"

3. **Run the following command to compile the main.c**:

   gcc main.c ms.c file_manipulation.c -o main -I./raylib -I. -L./raylib -lraylibdll -lopengl32 -lgdi32 -lwinmm -lstdc++ -lm -lpthread

4. **Run the executable**:
   ./main
