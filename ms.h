#ifndef MS_H
#define MS_H

#include <stdio.h>


#define MS_SIZE 23
#define MAX_LINE_SIZE 101
#define FB 10
#define ROW_FOR_BLOC 5
#define NBR_BLOC 20
#define TOTAL_NON_DATA_BLOC 3





//S T R U C T U R E S
typedef struct record{
    int ID;
    char row[MAX_LINE_SIZE];
    int flag;
}Element;
typedef struct Bloc{
    Element element[FB];
    int nbr_element;
    int last;
    int next;
}Bloc;
typedef struct AllocationTable{
    int nbr_bloc;
    int state;
}AllocationTable;
typedef struct AllocationTableBuffer{
    AllocationTable allocationTable[NBR_BLOC];
} AllocationTableBuffer;





// F U N C T I O N S
void initMS(FILE* ms);
void initAllocationTable(FILE* ms);
AllocationTableBuffer* UploadAllocationTableToMC(FILE* ms);
void deleteMS(FILE* ms);
void TablefromMCtoMS(AllocationTableBuffer* MC_Table, FILE* ms);
void MAJTable(FILE* ms, int contigue, int size, int Bloc_list[], int x);
void Compact(FILE* ms);
int* Check_available(FILE* ms, int n, int contigue);
int* CheckMSavailability(FILE* ms, int n, int contigue);
void print_MS(FILE* ms);
void addBloc(FILE* ms, int addressOfBloc);
void someUpdate(FILE* ms, int addressOfBloc, int type);







#endif