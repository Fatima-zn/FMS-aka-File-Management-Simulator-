#include <ms.h>
#include "file_manipulation.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


//INIT_MS
void initMS(FILE* ms){
    #define MS_SIZE 23
    #define MAX_LINE_SIZE 101
    #define FB 10
    #define ROW_FOR_BLOC 5
    #define NBR_BLOC 20
    #define TOTAL_NON_DATA_BLOC 3

    fclose(ms);
    ms = fopen("ms.bin", "wb");
    initAllocationTable(ms);

    fclose(ms);

    ms = fopen("ms.bin", "rb+");
}



//Initialization of Allocation Table
void initAllocationTable(FILE* ms) {
    rewind(ms);
    Bloc Buffer;
    char temp[5];
    int a = (MAX_LINE_SIZE - 1) / 4;
    int start = TOTAL_NON_DATA_BLOC, end = start + a;


    //Initialize all elements of Buffer
    for (int i = ROW_FOR_BLOC; i < FB; i++) {
        Buffer.element[i].ID = 0;
        Buffer.element[i].flag = 0;
        Buffer.element[i].row[0] = '\0';
    }
    Buffer.nbr_element = NBR_BLOC;
    Buffer.last = -1;
    Buffer.next = -1;


    for (int i = 0; i < ROW_FOR_BLOC; i++) {
        Buffer.element[i].ID = start;
        Buffer.element[i].flag = -1;


        Buffer.element[i].row[0] = '\0';
        for(int j = start; j <= end; j++){
            snprintf(temp, 5, "%03d0", j);
            strncat(Buffer.element[i].row, temp, 4);
            temp[0] = '\0';
        }
        start += a;
        end += a;
    }


    fwrite(&Buffer, sizeof(Buffer), 1, ms);

    rewind(ms);
}


void deleteMS(FILE* ms){
    fclose(ms);
    
    ms = fopen("ms.bin", "wb");
    initAllocationTable(ms);
    fclose(ms);
    
    ms = fopen("ms.bin", "rb+");
}


void TablefromMCtoMS(AllocationTableBuffer* MC_Table, FILE* ms) {
    rewind(ms);
    Bloc Buffer;
    char temp[5];
    int a = (MAX_LINE_SIZE - 1) / 4;
    int start, end = a ;

    Buffer.nbr_element = NBR_BLOC;
    Buffer.last = -1;
    Buffer.next = -1;


    for (int i = 0; i < ROW_FOR_BLOC; i++) {
        start = MC_Table->allocationTable[i*a].nbr_bloc;
        Buffer.element[i].ID = start;
        Buffer.element[i].flag = -1;
        Buffer.element[i].row[0] = '\0';
        for(int j = i*a; j < (i+1)*a; j++){
            snprintf(temp, 5, "%03d%d",MC_Table->allocationTable[j].nbr_bloc, MC_Table->allocationTable[j].state);
            strncat(Buffer.element[i].row, temp, 4);
            temp[0] = '\0';
        }
        end += a;
        
    }


    fwrite(&Buffer, sizeof(Buffer), 1, ms);
    rewind(ms);
}



 AllocationTableBuffer* UploadAllocationTableToMC(FILE* ms){
    Bloc BlocBuffer;
    static AllocationTableBuffer Buffer;
    char temp[4];
    int allocationIndex = 0;

    rewind(ms);
    fread(&BlocBuffer, sizeof(BlocBuffer), 1, ms);

    for(int i = 0; i < ROW_FOR_BLOC; i++){
        int j = 0;
        while(j < (int)strlen(BlocBuffer.element[i].row)){
            strncpy(temp, &BlocBuffer.element[i].row[j], 3);
            temp[3] = '\0';
            
            Buffer.allocationTable[allocationIndex].nbr_bloc = atoi(temp);
            if(BlocBuffer.element[i].row[j + 3] == '1') Buffer.allocationTable[allocationIndex].state = 1;
            else if(BlocBuffer.element[i].row[j + 3] == '0') Buffer.allocationTable[allocationIndex].state = 0;


            allocationIndex++;

            j += 4;
        }
    }

    rewind(ms);
    return &Buffer;
}

        
void MAJTable(FILE* ms, int contigue, int size, int Bloc_list[], int x){
    int M = TOTAL_NON_DATA_BLOC;
    AllocationTableBuffer* buffer = UploadAllocationTableToMC(ms);
    if(!contigue){
        int j;
        for(int i = 0; i < size; i++){
            j = Bloc_list[i] - M;
            buffer->allocationTable[j].state = x;
        }    
    }else{
        int b = Bloc_list[0]- M;
        int B = Bloc_list[1] - M;
        for(int i = b; i <= B; i++){
            buffer->allocationTable[i].state = x;
        }  
    }
    
    TablefromMCtoMS(buffer,ms);
}




void Compact(FILE* ms){
    rewind(ms);
    AllocationTableBuffer* Buffer = UploadAllocationTableToMC(ms);
    Bloc BlocBuffer;
    FILE* temp = fopen("temp.bin", "wb+");
    if(temp == NULL){
        printf("Error opening temporary file\n");
        return;
    }
    rewind(temp);
    rewind(ms);
    for(int j = 0; j < TOTAL_NON_DATA_BLOC; j++){
        fread(&BlocBuffer, sizeof(Bloc), 1, ms);
        fwrite(&BlocBuffer, sizeof(Bloc), 1, temp);
    }

    int occupied_counter = 0;
    int empty_counter = 0;
    int occupied_check_index = 0;
    int x = 1;
    int list[20];
    memset(&list,0,20*sizeof(int));
    for(int i = TOTAL_NON_DATA_BLOC; i < MS_SIZE; i++){
        if(!Buffer->allocationTable[i - TOTAL_NON_DATA_BLOC].state){
           empty_counter++;
           if( i + 1 != MS_SIZE && Buffer->allocationTable[i + 1 - TOTAL_NON_DATA_BLOC].state){
                x = 0;
                break;
           }
        }
        else occupied_counter++;
    }

    if(x) return;
    
    empty_counter = 0;
    occupied_counter = 0;

    for(int i = TOTAL_NON_DATA_BLOC;  i < MS_SIZE; i++){
        if(Buffer->allocationTable[i - TOTAL_NON_DATA_BLOC].state){
            occupied_counter++;
        }else{
            empty_counter ++;
            
            if(occupied_counter > 0){
                for(int j = i - occupied_counter - TOTAL_NON_DATA_BLOC; j < i - TOTAL_NON_DATA_BLOC; j++){
                    list[j] = -empty_counter + 1;
                }
                occupied_check_index += occupied_counter;
                Bloc* BufferArray = (Bloc*)malloc(occupied_counter * sizeof(Bloc));
                fseek(ms, sizeof(Bloc) * (i - occupied_counter), SEEK_SET);
                fread(BufferArray, sizeof(Bloc), occupied_counter, ms);
                fwrite(BufferArray, sizeof(Bloc), occupied_counter, temp);
                occupied_counter = 0;
                free(BufferArray);
            }
        }
    }
    //Handle the case where the last blocks are occupied
    if(occupied_counter > 0){
        for(int j = MS_SIZE - occupied_counter - TOTAL_NON_DATA_BLOC; j < MS_SIZE - TOTAL_NON_DATA_BLOC; j++){
                    list[j] = -occupied_counter;
                }
        occupied_check_index+=occupied_counter;
        Bloc* BufferArray = (Bloc*)malloc(occupied_counter * sizeof(Bloc));
        fseek(ms, sizeof(Bloc) * (MS_SIZE - occupied_counter), SEEK_SET);
        fread(BufferArray, sizeof(Bloc), occupied_counter, ms);
        fwrite(BufferArray, sizeof(Bloc), occupied_counter, temp);
        free(BufferArray);
    }

    //Update Allocation Table
    int blocList[] = {TOTAL_NON_DATA_BLOC, TOTAL_NON_DATA_BLOC + occupied_check_index - 1};
    MAJTable(temp, 1, 2, blocList, 1);
 
    int emptyList[] = {TOTAL_NON_DATA_BLOC + occupied_check_index, MS_SIZE - 1};
    MAJTable(temp, 1, 2, emptyList, 0);
    Bloc Buffer0;
    fseek(temp, sizeof(Bloc), SEEK_SET);
    fread(&Buffer0, sizeof(Bloc), 1, temp);
    int max = Buffer0.last;
    MetaBuffer* meta;
    Element metaElement;
    fseek(temp, sizeof(Bloc), SEEK_SET);
    for(int i = 1; i < TOTAL_NON_DATA_BLOC; i++){   
        for(int j = 0; j < FB; j++){
            if(Buffer0.element[j].flag == 1){
                meta = ElementToMetaBufferPart(Buffer0.element[j]);
                meta->first_address += list[meta->first_address - TOTAL_NON_DATA_BLOC]; 
                meta->last_address +=list[meta->last_address - TOTAL_NON_DATA_BLOC]; 
                metaElement = *MetaBufferToElement(*meta);
                Buffer0.element[j] = metaElement;
            }
            if(j + FB * i == max) break;
        }
        fseek(temp, i * sizeof(Bloc), SEEK_SET);
        fwrite(&Buffer0,sizeof(Bloc), 1, temp);
        if(FB * (i+1) == max) break;
        fread(&Buffer0, sizeof(Bloc), 1, temp);
    }


    fclose(temp);
    fclose(ms);


    remove("ms.bin");
    rename("temp.bin", "ms.bin");

    
    ms =  fopen("ms.bin", "rb+");

}












int* Check_available(FILE* ms, int n, int contigue){
    AllocationTableBuffer* buffer = UploadAllocationTableToMC(ms);
    int c = 0;
    int *blocs = (int*)malloc(n * sizeof(int));
    if(!contigue){
        for(int i = 0; i < NBR_BLOC ; i++){
            if(!buffer->allocationTable[i].state){
                blocs[c] = buffer->allocationTable[i].nbr_bloc;
                c += 1;
                if(c == n) return blocs;
            }
        }
        return NULL;
    }   
    for(int i = 0; i < NBR_BLOC ; i++){
        if(buffer->allocationTable[i].state) c = 0;
        else{
            blocs[c] = buffer->allocationTable[i].nbr_bloc;
            c += 1;
            
        }
        if(c == n) return blocs;
        
    }
    return NULL;
}      
 

int* CheckMSavailability(FILE* ms, int n, int contigue){
    int* p = Check_available(ms, n, contigue);
    if(p || !contigue) return p;
    Compact(ms);
    p = Check_available(ms, n, contigue);
    if(p) return p;
    return NULL;
} 





//TESTING FUNCTIONS
void print_MS(FILE* ms){
    rewind(ms);

    Bloc Buffer;
    int bloc_count = TOTAL_NON_DATA_BLOC;

        fseek(ms, sizeof(Bloc) * TOTAL_NON_DATA_BLOC, SEEK_SET);

        while (fread(&Buffer, sizeof(Buffer), 1, ms) == 1){

            printf("Block %d : \n", bloc_count);
            for (int i = 0; i < FB; i++) {
                printf("ID : %d\t, Row : %s\t, flag : %d\n", Buffer.element[i].ID, Buffer.element[i].row, Buffer.element[i].flag);
            }
            printf("Nbr_element : %d\t\t\t", Buffer.nbr_element);
            printf("Last : %d\t\t", Buffer.last);
            printf("Next : %d\n", Buffer.next);
            printf("\n");

            bloc_count++;
        }
    rewind(ms);
}

void addBloc(FILE* ms, int addressOfBloc){
   rewind(ms);
   fseek(ms, sizeof(Bloc) * (TOTAL_NON_DATA_BLOC + (addressOfBloc - TOTAL_NON_DATA_BLOC)), SEEK_SET);
   Bloc Buffer;

    //Bloc writing
    for(int i = 1; i < FB; i++){
        Buffer.element[i].ID = 0;
        strcpy(Buffer.element[i].row, "Empty");
        Buffer.element[i].flag = 0; 
        Buffer.nbr_element = 1;
        Buffer.last = -1;
        Buffer.next = -1;
    }
    Buffer.element[0].ID = 0;
    strcpy(Buffer.element[0].row, "Some data here...");
    Buffer.element[0].flag = 1; 
    Buffer.nbr_element = 1;
    Buffer.last = -1;
    Buffer.next = -1;

    fwrite(&Buffer, sizeof(Buffer), 1, ms);
    rewind(ms);
}

void someUpdate(FILE* ms, int addressOfBloc, int type){
    AllocationTableBuffer* buffer = UploadAllocationTableToMC(ms);
    buffer->allocationTable[1].state = true;
    int tmpList[2] = {addressOfBloc, addressOfBloc};
    MAJTable(ms, true, 1, tmpList, type);
    rewind(ms);
    Bloc Buffer;
    fread(&Buffer, sizeof(Bloc), 1, ms);
    rewind(ms);
}



//MAIN
/*
int main(){
    FILE *MS = fopen("ms.bin", "rb+");
    if(MS == NULL){
        printf("Error opening file ms.bin\n");
        return 1;
    }


     
    deleteMS(MS);
    rewind(MS);
    addBloc(MS, 4);
    someUpdate(MS, 4);
    addBloc(MS, 7);
    someUpdate(MS, 7);
    addBloc(MS, 15);
    someUpdate(MS, 15);
    addBloc(MS, 21);
    someUpdate(MS, 21);
    addBloc(MS, 22);
    someUpdate(MS, 22);

  
    Compactage(MS);



    rewind(MS);
    print_MS(MS);
    rewind(MS);

 





    AllocationTableBuffer* buffer = UploadAllocationTableToMC(MS);

    for(int i = 0; i < NBR_BLOC; i++){
        printf("nbr_bloc : %d\t\t\t", buffer->allocationTable[i].nbr_bloc);
        printf("State : %d\n", buffer->allocationTable[i].state);
    }
 
 



    fclose(MS);
    return 0;
}
*/





