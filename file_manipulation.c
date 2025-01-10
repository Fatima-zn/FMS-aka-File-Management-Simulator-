#include "ms.h"
#include "file_manipulation.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>




MetaBuffer* ElementToMetaBufferPart(Element meta){
	MetaBuffer* Buffer = (MetaBuffer*)malloc(sizeof(MetaBuffer));;
	Buffer->bloc = meta.ID;
	sscanf(meta.row, "%03d,%03d,%03d,%d,%d,%20[^\n]", &Buffer->record, &Buffer->first_address, &Buffer->last_address, &Buffer->contigue, &Buffer->ordered, Buffer->name);
	Buffer->flag = meta.flag;
	return Buffer;
}

Element* MetaBufferToElement(MetaBuffer Buffer){
	Element* meta = (Element*)malloc(sizeof(Element));
	meta->ID = Buffer.bloc;
	meta->flag = Buffer.flag;
	sprintf(meta->row, "%03d,%03d,%03d,%d,%d,%s", Buffer.record, Buffer.first_address, Buffer.last_address, Buffer.contigue, Buffer.ordered, Buffer.name);
	return meta;
}

MetaBuffer* Get_MetaData(FILE* ms, char name[]){
    rewind(ms);
    fseek(ms, sizeof(Bloc), SEEK_SET);
    Bloc Buffer;
    MetaBuffer* metaBuffer;
    char* meta_name;
    for(int j = 1; j < TOTAL_NON_DATA_BLOC ; j++){
        fread(&Buffer, sizeof(Bloc), 1, ms);
        for(int i = 0; i < FB; i++){
            meta_name = Buffer.element[i].row + 16;
            if(!strcmp(meta_name, name)){
                metaBuffer = ElementToMetaBufferPart(Buffer.element[i]);
                
                metaBuffer->Address_bloc = j;
                metaBuffer->Address_record = i;
                return metaBuffer;
            }
        }
    }
    return NULL;
}


int* Get_ChainneeAddresses(FILE* ms, int first_address){
    Bloc Buffer;
    int* Addresses = (int*)malloc(NBR_BLOC * sizeof(int));
    fseek(ms, first_address * sizeof(Bloc), SEEK_SET);
    Addresses[0] = first_address;
    int i = 0;
    fread(&Buffer, sizeof(Bloc), 1, ms);
    while(Buffer.next != -1){
        i++;
        Addresses[i] = Buffer.next;
        fseek(ms, Addresses[i] * sizeof(Bloc), SEEK_SET);
        fread(&Buffer, sizeof(Bloc), 1, ms);
    }
    return Addresses;
} 


int UpdateMeta(FILE* ms, char curr_name[], char new_name[],int bloc,int record, int first_address, int last_address, int Add_Addresses[],int added_a, int delete_Addresses[],int deleted_a,  int delete) {
	MetaBuffer* ptr_meta =  Get_MetaData(ms, curr_name);
	Element* meta;
	if(delete == 1 || bloc == 0 || record == 0) {
		int* address_list;
		ptr_meta->flag = 0;
        Bloc Buffer;
		if(!ptr_meta->contigue) address_list = Get_ChainneeAddresses(ms, ptr_meta->first_address);
		else {
			address_list = (int*)malloc(2 * sizeof(int));
			address_list[0] = ptr_meta->first_address;
			address_list[1] = ptr_meta->first_address + ptr_meta->bloc - 1;
		}
		MAJTable(ms,ptr_meta->contigue, ptr_meta->bloc, address_list,0);
		free(address_list);
        fseek(ms,ptr_meta->Address_bloc * sizeof(Bloc),SEEK_SET);
        fread(&Buffer,sizeof(Bloc), 1, ms);
        Buffer.element[ptr_meta->Address_record].flag = 0;
        fseek(ms,ptr_meta->Address_bloc * sizeof(Bloc),SEEK_SET);
        fwrite(&Buffer,sizeof(Bloc), 1, ms);
	} else {
		if(bloc != -1) {
            if(CheckMSavailability(ms, bloc, ptr_meta->contigue)){
                ptr_meta->bloc = bloc;
                MAJTable(ms,0,deleted_a,delete_Addresses,0);
                MAJTable(ms,0,added_a,Add_Addresses,1);
            }else return -1;
		}

		if(record != -1) ptr_meta->record = record;
		if(first_address != -1) ptr_meta->first_address = first_address;
		if(last_address != -1) ptr_meta->last_address = last_address;
		if(strcmp(new_name, "") != 0){
            if(Get_MetaData(ms, new_name) == NULL) strcpy(ptr_meta->name, new_name);
            else return 1;
        }
	}
	meta = MetaBufferToElement(*ptr_meta);
	fseek(ms,sizeof(Bloc) * ptr_meta->Address_bloc + sizeof(Element) * ptr_meta->Address_record, SEEK_SET);
	fwrite(meta, sizeof(Element), 1, ms);
	free(meta);
	free(ptr_meta);
    return 0;
}








void defragmentation_Meta(FILE* ms){
    int bloc = TOTAL_NON_DATA_BLOC - 1;
    int last_address= TOTAL_NON_DATA_BLOC - 1;
    int start = 1;
    Bloc Buffer;
    int total_recordCounter = 0;
    int ElementCounter = 0; 
    Bloc Buffer0 = {}; 
    int writeIndex = start; 

    for (int i = 0; i < bloc; i++) {
        fseek(ms, sizeof(Bloc) * (start + i), SEEK_SET);
        fread(&Buffer, sizeof(Bloc), 1, ms);
        for (int j = 0; j < FB; j++) {
            if (Buffer.element[j].flag) {
                Buffer0.element[ElementCounter].ID = Buffer.element[j].ID;
                strcpy(Buffer0.element[ElementCounter].row, Buffer.element[j].row);
                Buffer0.element[ElementCounter].flag = 1;
                ElementCounter++;
                total_recordCounter++;
                if (ElementCounter == FB) {
                    Buffer0.nbr_element = FB;
                    Buffer0.next = -1;
                    Buffer0.last = -1;
                    fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
                    fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                    writeIndex++;
                    ElementCounter = 0; 
                    memset(&Buffer0, 0, sizeof(Buffer0)); //To Clear Buffer0
                }
            }
        }
    }

    //Write remaining elements in Buffer0 if not empty
    if (ElementCounter > 0){
        Buffer0.nbr_element = ElementCounter;
        Buffer0.next = -1;
        Buffer0.last = -1;
        fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
        writeIndex++;
    }
    
    memset(&Buffer0, 0, sizeof(Buffer0));
    Buffer0.next = -1;
    Buffer0.last = -1;
    fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
    for(int i = writeIndex; i <= last_address;i++) fwrite(&Buffer0, sizeof(Bloc), 1, ms);



    fseek(ms, sizeof(Bloc), SEEK_SET);    
    fread(&Buffer, sizeof(Bloc), 1, ms);
    Buffer.last = total_recordCounter;
    fseek(ms, sizeof(Bloc), SEEK_SET);
    fwrite(&Buffer, sizeof(Bloc), 1, ms);
}



void Create_MetaData(FILE* ms, char name[], int bloc, int ne, int first, int last, int contigue, int ordered){
    Bloc Buffer;
    rewind(ms);
    fseek(ms, sizeof(Bloc), SEEK_SET);    
    fread(&Buffer, sizeof(Bloc), 1, ms);
    int i = Buffer.last;
    
    fseek(ms, sizeof(Bloc), SEEK_SET);
    if( i == FB * (TOTAL_NON_DATA_BLOC - 1)){    
        defragmentation_Meta(ms);
        fseek(ms, sizeof(Bloc), SEEK_SET);    
        fread(&Buffer, sizeof(Bloc), 1, ms);
        i = Buffer.last;
    }
    if(Buffer.last >= FB*(TOTAL_NON_DATA_BLOC-1)){ 
        Buffer.last = 0;
        Buffer.nbr_element = 0;
    }
    i = Buffer.last;
    Buffer.last++;
    fwrite(&Buffer, sizeof(Bloc), 1, ms);
    int n_bloc = (int)ceil((double) Buffer.last/FB) ;
    int n_record = (Buffer.last-1) % FB;
    fseek(ms, n_bloc*sizeof(Bloc) + n_record*sizeof(Element), SEEK_SET);
    Buffer.element[0].ID = bloc;
    Buffer.element[0].flag = 1;
    sprintf(Buffer.element[0].row, "%03d,%03d,%03d,%d,%d,%s", ne, first, last, contigue, ordered, name); 
    fwrite(&Buffer, sizeof(Element), 1, ms);
    rewind(ms);
}


void ChargeFile(FILE* ms, FILE* inputFile, int size, int nbr_e, int* addressList, int contigue){
    rewind(ms);
    int IDCounter = 0;
    char data[MAX_LINE_SIZE];

    fseek(ms, sizeof(Bloc) * addressList[0], SEEK_SET);

    if(contigue){
        Bloc* BufferArray = (Bloc*)malloc(sizeof(Bloc)* size);
        for(int i = 0; i < size; i++){
            BufferArray[i].nbr_element = 0;
            while(IDCounter < nbr_e && BufferArray[i].nbr_element < FB){
                if (fgets(data, sizeof(data), inputFile) == NULL) break;
                BufferArray[i].element[BufferArray[i].nbr_element].ID = IDCounter++;
                strcpy(BufferArray[i].element[BufferArray[i].nbr_element].row, data);
                BufferArray[i].element[BufferArray[i].nbr_element].flag = 1;
                BufferArray[i].nbr_element++;
            }
            BufferArray[i].last = -1;
            BufferArray[i].next = -1;

        }
        fwrite(BufferArray, sizeof(Bloc), size, ms);

        free(BufferArray);

        //Update Allocation Table
        int temp[2] = {addressList[0], addressList[size - 1]};
        MAJTable(ms, 1, size, temp, 1);


    } else{
        Bloc* BufferArray = (Bloc*)malloc(sizeof(Bloc) * size); 
        for (int i = 0; i < size; i++){ 
            BufferArray[i].nbr_element = 0; 
            while (IDCounter < nbr_e && BufferArray[i].nbr_element < FB){ 
                if (fgets(data, sizeof(data), inputFile) == NULL) break;
                BufferArray[i].element[BufferArray[i].nbr_element].ID = IDCounter++; 
                strcpy(BufferArray[i].element[BufferArray[i].nbr_element].row, data); 
                BufferArray[i].element[BufferArray[i].nbr_element].flag = 1; 
                BufferArray[i].nbr_element++; 
            } 
            BufferArray[i].last = (i > 0) ? addressList[i - 1] : -1; 
            BufferArray[i].next = (i + 1 < size) ? addressList[i + 1] : -1;  
             
        

            fseek(ms, sizeof(Bloc) * addressList[i], SEEK_SET);
            fwrite(&BufferArray[i], sizeof(Bloc), 1, ms);
        }
        MAJTable(ms, 0, size, addressList, 1); 
        free(BufferArray);
    }
    
    free(addressList);
}


int Create_file(FILE* ms,FILE* inputFile, char name[], int ne, int contigue, int ordered){ //contigue(1: contigue, 0: chainee) ordered(1: ordered, 0: not)
    int size = (int)ceil((double) ne/FB); //in terms of Bloc
    int* l = CheckMSavailability(ms, size, contigue);
    MetaBuffer* ptr = Get_MetaData(ms, name);
    if(l){
        if(ptr == NULL){
            Create_MetaData(ms, name, size, ne,l[0],l[size-1], contigue, ordered);
            ChargeFile(ms, inputFile, size, ne, l, contigue);
            return 0;
        }else{
            return 1;
        }
    }else{
        return -1;
    }
}




//Binary search within elements of a bloc
int binarySearchElements(Element* elements, int nbr_element, int id){ 
    int left = 0; 
    int right = nbr_element - 1; 
    while (left <= right){ 
        int mid = left + (right - left) / 2; 
        if (elements[mid].ID == id){
            if(elements[mid].flag){
                return mid;
            }
        }
        else if (elements[mid].ID < id) left = mid + 1;
             else right = mid - 1;
     } 
             
    return -1; 
} 


//Recherche Dicotomique  | Contiguous Case
searchResult binarySearchContiguous(FILE* ms, int id, int firstBlock, int lastBlock){
    rewind(ms);
    searchResult result = {.blocPosition = -1, .elementPosition = -1, .isFound = 0};
    int left = firstBlock; 
    int right = lastBlock; 
    Bloc buffer; 
    while (left <= right){ 
        int mid = left + (right - left) / 2;
        fseek(ms, sizeof(Bloc) * mid, SEEK_SET); 
        fread(&buffer, sizeof(Bloc), 1, ms); 
        int index = binarySearchElements(buffer.element, buffer.nbr_element, id); 
        if (index != -1){
            result.blocPosition = mid;
            result.elementPosition = index;
            result.isFound = 1;
            return result;
        }
        if (buffer.element[0].ID > id){ 
            right = mid - 1; 
        } else{ 
            left = mid + 1;
        } 
    } 

    rewind(ms);
    return result;
}



//Recherche Dicotomique | Chained Case
searchResult binarySearchChained(FILE* ms, int id, int firstBlock){
    rewind(ms);
    searchResult result = {.blocPosition = -1, .elementPosition = -1, .isFound = 0};
    Bloc buffer; 
    
    //Get the number of blocks in the chain 
    int size = 0; 
    int currentBlock = firstBlock; 
    while (currentBlock != -1){ 
        size++; fseek(ms, sizeof(Bloc) * currentBlock, SEEK_SET); 
        fread(&buffer, sizeof(Bloc), 1, ms); 
        currentBlock = buffer.next; 
    } 
    int left = 0; 
    int right = size - 1; 

    while (left <= right){ 
        int mid = left + (right - left) / 2; 
        
        //Traverse to the mid block 
        currentBlock = firstBlock; 
        for (int i = 0; i < mid; i++){ 
            fseek(ms, sizeof(Bloc) * currentBlock, SEEK_SET); 
            fread(&buffer, sizeof(Bloc), 1, ms); 
            currentBlock = buffer.next; 
        } 
        
        fseek(ms, sizeof(Bloc) * currentBlock, SEEK_SET); 
        fread(&buffer, sizeof(Bloc), 1, ms); 

        int index = binarySearchElements(buffer.element, buffer.nbr_element, id); 
        if (index != -1){
            result.blocPosition = currentBlock;
            result.elementPosition = index;
            result.isFound = 1;
            return result;
        }
        if (buffer.element[0].ID > id) right = mid - 1;
        else left = mid + 1;
    } 

    rewind(ms);
    return result; 
}




//Linear search | Contiguous Organization 
searchResult linearSearchContiguous(FILE* ms, int id, int firstBlock, int lastBlock){ 
    searchResult result = {.blocPosition = -1, .elementPosition = -1, .isFound = 0}; 
    Bloc buffer; 

    for (int i = firstBlock; i <= lastBlock; i++){ 
        fseek(ms, sizeof(Bloc) * i, SEEK_SET); 
        fread(&buffer, sizeof(Bloc), 1, ms); 
        for (int j = 0; j < buffer.nbr_element; j++){ 
            if (buffer.element[j].ID == id && buffer.element[j].flag){ 
                result.blocPosition = i; 
                result.elementPosition = j; 
                result.isFound = 1; 

                return result; 
            } 
        } 
    } 
    
    return result;
}




//Linear search | Chained Organization 
searchResult linearSearchChained(FILE* ms, int id, int firstBlock){ 
    searchResult result = {.blocPosition = -1, .elementPosition = -1, .isFound = 0};
    Bloc buffer; 
    int currentBlock = firstBlock; 

    while (currentBlock != -1){ 
        fseek(ms, sizeof(Bloc) * currentBlock, SEEK_SET); 
        fread(&buffer, sizeof(Bloc), 1, ms); 
        for (int j = 0; j < buffer.nbr_element; j++){ 
            if (buffer.element[j].ID == id && buffer.element[j].flag){ 
                result.blocPosition = currentBlock; 
                result.elementPosition = j;
                result.isFound = 1; 
                
                return result;
            } 
        } 

        currentBlock = buffer.next; 
    } 

    return result;
}




//General Function
searchResult searchElementByID(FILE* ms, char fileName[], int id){ 
    //GET METADATA
    MetaBuffer* metaBuffer = Get_MetaData(ms, fileName);

    //SEARCH PROCESS
    if (metaBuffer->ordered){ 
        if (metaBuffer->contigue){ 
            return binarySearchContiguous(ms, id, metaBuffer->first_address, metaBuffer->last_address); 
        } else{ 
            return binarySearchChained(ms, id, metaBuffer->first_address); 
        } 
    } else{ 
        if (metaBuffer->contigue){ 
            return linearSearchContiguous(ms, id, metaBuffer->first_address, metaBuffer->last_address); 
        } else{ 
            return linearSearchChained(ms, id, metaBuffer->first_address); 
        } 
    } 
}





//Suppression Logique
bool LogicalSuppression(FILE* ms, char fileName[], int id){
    searchResult result = searchElementByID(ms, fileName, id);
    if(!result.isFound){
        return false;
    } else{
        Bloc buffer;
        bool isDecremented = false;
        fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
        fread(&buffer, sizeof(Bloc), 1, ms);
        buffer.element[result.elementPosition].flag = 0;
        //ADDED
        buffer.nbr_element--;
        if(buffer.nbr_element == 0){
            isDecremented = true;
            int temp[2] = {result.blocPosition, result.blocPosition};
            MAJTable(ms, 1, 2, temp, 0);
        }
        fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
        fwrite(&buffer, sizeof(Bloc), 1, ms);

        //UPDATE METADATA
        MetaBuffer* metaBuffer = Get_MetaData(ms, fileName);
        metaBuffer->record -= 1;
        //ADDED
        if(isDecremented) metaBuffer->bloc--;
        Element* meta = MetaBufferToElement(*metaBuffer);
        fseek(ms, sizeof(Bloc)*metaBuffer->Address_bloc + sizeof(Element)*metaBuffer->Address_record, SEEK_SET);
        fwrite(meta, sizeof(Element), 1, ms);

        return true;
    }
}

//Suppression Physique
void PhysicalSuppression(FILE* ms, char filename[], int id){
    MetaBuffer* meta = Get_MetaData(ms, filename);
    if(!meta) printf("not finding meta\n");
    if(meta->ordered){
        if(LogicalSuppression(ms, filename, id)){
            defragmentation(ms, filename);
        }else{
            printf("Element not found or already deleted\n");
            return;
        }
    }else{
        searchResult result = searchElementByID(ms, filename, id);
        int lastElementIndex = meta->record - 1;

        //Case of the last element is the element to be deleted
        if(lastElementIndex == result.elementPosition){
            printf("CASE OF LAST ELEMENT IS THE ELEMENT TO BE DELETED DETECTED\n");
            Bloc Buffer;
            fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
            fread(&Buffer, sizeof(Bloc), 1, ms);


            Buffer.element[lastElementIndex].flag = 0;
            if(Buffer.nbr_element == 1){
                if(meta->first_address == meta->last_address){
                    meta->flag = 0;

                    int temp[2] = {meta->first_address, meta->last_address};
                    MAJTable(ms, 1, 2, temp, 0);
    
                }else{
                    meta->bloc--;
                    if(meta->contigue){
                        meta->last_address--;
                    }else{
                        meta->last_address = Buffer.last;
                        Buffer.last = -1;
                        
                        Bloc TempBuffer;
                        fseek(ms, sizeof(Bloc) * Buffer.last, SEEK_SET);
                        fread(&TempBuffer, sizeof(Bloc), 1, ms);
                        TempBuffer.next = -1;

                        fseek(ms, sizeof(Bloc) * Buffer.last, SEEK_SET);
                        fwrite(&TempBuffer, sizeof(Bloc), 1, ms);
                    }
                }
            }
            Buffer.nbr_element--;

            fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
            fwrite(&Buffer, sizeof(Bloc), 1, ms);

            meta->record -= 1;
            
            int temp[1]={};
            UpdateMeta(ms, filename, "", meta->bloc, meta->record, -1 , meta->last_address, temp,0, temp, 0,  0);

            return;
        }

        //General Case
        printf("GENERAL CASE DETECTED\n");
        Bloc lastBloc;
        Bloc elementToDeleteBloc;
        fseek(ms, sizeof(Bloc) * meta->last_address, SEEK_SET);
        fread(&lastBloc, sizeof(Bloc), 1, ms);
        printf("meta->last_address: %d\n", meta->last_address);

        fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
        fread(&elementToDeleteBloc, sizeof(Bloc), 1, ms);
        printf("result.blocPosition = %d\n", result.blocPosition);


        //if the two elements were in the same bloc don't write it here
        if(result.blocPosition !=  meta->last_address){
            elementToDeleteBloc.element[result.elementPosition].ID = lastBloc.element[lastElementIndex % FB].ID;
            strcpy(elementToDeleteBloc.element[result.elementPosition].row, lastBloc.element[lastElementIndex % FB].row);
            elementToDeleteBloc.element[result.elementPosition].flag = 1;
            printf("result.elementPosition = %d\n lastElementIndex = %d\n", result.elementPosition, (lastElementIndex % FB));
            
            fseek(ms, sizeof(Bloc) * result.blocPosition, SEEK_SET);
            fwrite(&elementToDeleteBloc, sizeof(Bloc), 1, ms);

            printf("Element has been written successfully\n");
        } else{
            lastBloc.element[result.elementPosition].ID = lastBloc.element[lastElementIndex % FB].ID;
            strcpy(lastBloc.element[result.elementPosition].row, lastBloc.element[lastElementIndex % FB].row);
            lastBloc.element[result.elementPosition].flag = 1;
            printf("result.elementPosition = %d\n lastElementIndex = %d\n", result.elementPosition, (lastElementIndex % FB));
        }




        lastBloc.element[lastElementIndex % FB].flag = 0;
        if(lastBloc.nbr_element == 1){
            int lastPreviousAddress = lastBloc.last;
            memset(&lastBloc, 0, sizeof(Bloc));
            fseek(ms, sizeof(Bloc) * meta->last_address, SEEK_SET);
            fwrite(&lastBloc, sizeof(Bloc), 1, ms);

            meta->bloc--;
            
            int temp[2] = {meta->last_address, meta->last_address};
            MAJTable(ms, 1, 2, temp, 0);

            if(meta->contigue){
                meta->last_address--;
            }else{
                meta->last_address = lastPreviousAddress;


                Bloc TempBuffer;
                fseek(ms, sizeof(Bloc) * meta->last_address, SEEK_SET);
                fread(&TempBuffer, sizeof(Bloc), 1, ms);
                TempBuffer.next = -1;

                fseek(ms, sizeof(Bloc) * meta->last_address, SEEK_SET);
                fwrite(&TempBuffer, sizeof(Bloc), 1, ms);
            }

        } else{
            lastBloc.nbr_element--;
            fseek(ms, sizeof(Bloc) * meta->last_address, SEEK_SET);
            fwrite(&lastBloc, sizeof(Bloc), 1, ms);
        }


        
        meta->record -= 1;

        int temp[1]={};
        UpdateMeta(ms, filename, "", meta->bloc,meta->record, -1, meta->last_address, temp,0, temp, 0, 0);
        
    }
}










void defragmentation(FILE* ms, char fileName[]){
    MetaBuffer* metaBuffer = Get_MetaData(ms,fileName);   
    int start = metaBuffer->first_address;
    Bloc Buffer;
    int total_recordCounter = 0;
    int total_WrittenBlocCounter = 0;
    int ElementCounter = 0; 
    Bloc Buffer0 = {};
    if (metaBuffer->contigue) {
        int writeIndex = start; //Index for writing defragmented blocks

        for (int i = 0; i < metaBuffer->bloc; i++) {
            fseek(ms, sizeof(Bloc) * (start + i), SEEK_SET);
            fread(&Buffer, sizeof(Bloc), 1, ms);

            for (int j = 0; j < Buffer.nbr_element; j++) {
                if (Buffer.element[j].flag) {
                    Buffer0.element[ElementCounter].ID = Buffer.element[j].ID;
                    strcpy(Buffer0.element[ElementCounter].row, Buffer.element[j].row);
                    Buffer0.element[ElementCounter].flag = 1;
                    ElementCounter++;
                    total_recordCounter++;

                    if (ElementCounter == FB) {
                        Buffer0.nbr_element = FB;
                        Buffer0.next = -1;
                        Buffer0.last = -1;
                        fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
                        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                        total_WrittenBlocCounter++;
                        writeIndex++;
                        ElementCounter = 0; 
                        memset(&Buffer0, 0, sizeof(Buffer0));  //To Clear Buffer0
                    }
                }
            }

            if (total_recordCounter == metaBuffer->record) break;
        }

        //Write remaining elements in Buffer0 if not empty
        if (ElementCounter > 0){
            Buffer0.nbr_element = ElementCounter;
            Buffer0.next = -1;
            Buffer0.last = -1;
            fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
            total_WrittenBlocCounter++;
            writeIndex++;
        }
        
        memset(&Buffer0, 0, sizeof(Buffer0));
        Buffer0.next = -1;
        Buffer0.last = -1;
        fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
        for(int i = writeIndex; i <= metaBuffer->last_address;i++) fwrite(&Buffer0, sizeof(Bloc), 1, ms);




        //Update Allocation Table
        int temp[2];
        temp[0] = writeIndex;
        temp[1] = metaBuffer->last_address;
        MAJTable(ms, 1, 2, temp, 0);


        
        //Update MetaData
        int temp0[1] = {};
        int temp1[1] = {};
        UpdateMeta(ms, metaBuffer->name, "", total_WrittenBlocCounter, -1, -1, writeIndex - 1, temp0, 0, temp1, 0, 0);



    } else{
        int total_recordCounter = 0; //CHANGED
        int BlocCounter = 0;
        int nexts[metaBuffer->bloc];
        nexts[0] = start;
        for(int i = 0; i < metaBuffer->bloc; i++){
            fseek(ms, sizeof(Bloc) * nexts[i], SEEK_SET);
            fread(&Buffer, sizeof(Bloc), 1, ms);      
            nexts[i+1] = Buffer.next;
            for(int j = 0; j < FB; j++){
                if (Buffer.element[j].flag == 1){
                    ElementCounter ++;
                    total_recordCounter++;
                    Buffer0.element[ElementCounter-1].ID = Buffer.element[j].ID;
                    strcpy(Buffer0.element[ElementCounter-1].row, Buffer.element[j].row);
                    Buffer0.element[ElementCounter-1].flag = 1;
                    if(ElementCounter == FB){
                        //WRITE
                        Buffer0.nbr_element = FB;
                        if (BlocCounter == 0) Buffer0.last = -1;
                        else Buffer0.last = nexts[BlocCounter-1];
                        if(total_recordCounter == metaBuffer->record){
                            Buffer0.next = -1;
                            ElementCounter = 0;
                            fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
                            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                            BlocCounter++;
                            break;
                        }
                        Buffer0.next = nexts[BlocCounter+1];
                        ElementCounter = 0;
                        fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
                        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                        BlocCounter++;
                        for(int i = 0; i < FB; i++){
                            printf("%d\n",Buffer0.element[i].ID);
                        }
                        memset(&Buffer0, 0, sizeof(Bloc)); 
                    }
                } 
            }
            if(total_recordCounter == metaBuffer->record) break;  
        }    

        if(ElementCounter != 0){
            //WRITE
            Buffer0.nbr_element = ElementCounter;
            if (BlocCounter == 0) Buffer0.last = -1;
            else Buffer0.last = nexts[BlocCounter-1];
            Buffer0.next = -1;
            fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
            BlocCounter++;
        }
        int *deleted = nexts + (BlocCounter);
        memset(&Buffer0, 0, sizeof(Bloc));
        Buffer0.next = -1;
        Buffer0.last = -1;
        for(int i = 0; i < metaBuffer->bloc - BlocCounter; i++){
            fseek(ms, sizeof(Bloc) * deleted[i] , SEEK_SET);
            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
        }
        int temp0[1] = {};
        UpdateMeta(ms, metaBuffer->name, "", BlocCounter, -1, -1, nexts[BlocCounter-1], temp0, 0, deleted, metaBuffer->bloc-BlocCounter, 0);           
    } 
} //last blocs pointers not deleted



















int Insertion_Contigue_NotOrdered(FILE* ms,char name[], int ID, char data[], MetaBuffer* metaBuffer){
    searchResult result = binarySearchContiguous(ms, ID, metaBuffer->first_address, metaBuffer->last_address);
    if (result.blocPosition == -1 && result.elementPosition == -1 && result.isFound == 0){
        if(metaBuffer->record < FB * metaBuffer->bloc){
            Bloc Buffer;
            int i = 0;
            fseek(ms, metaBuffer->last_address*sizeof(Bloc),SEEK_SET);
            fread(&Buffer, sizeof(Bloc), 1, ms);
            if(Buffer.nbr_element == FB){
                defragmentation(ms,name);
                metaBuffer = Get_MetaData(ms,name);
                fseek(ms, metaBuffer->last_address*sizeof(Bloc),SEEK_SET);
                fread(&Buffer, sizeof(Bloc), 1, ms);
            }
            
            while(Buffer.element[i].flag == 1){
                i++;
            }
            Buffer.element[i].ID = ID;
            strcpy(Buffer.element[i].row, data);
            Buffer.element[i].flag = 1;
            Buffer.nbr_element += 1;
            fseek(ms, -(long)sizeof(Bloc), SEEK_CUR);
            fwrite(&Buffer, sizeof(Bloc), 1, ms);
            int temp[3] = {};
            int temp0[3] = {};
            UpdateMeta(ms, name, "", -1, metaBuffer->record + 1, -1, -1, temp, 0, temp0, 0, 0);
        }else{
            
            if(CheckMSavailability(ms,1, 0)){
                AllocationTableBuffer* MC_Table = UploadAllocationTableToMC(ms);
                int start;
                int end;
                int* ptr = (int*)malloc(sizeof(int));
                int added_n;
                if(MC_Table->allocationTable[metaBuffer->last_address].state){
                    //Blocs to Buffer
                    Bloc Buffer[metaBuffer->bloc + 1];
                    fseek(ms, metaBuffer->first_address*sizeof(Bloc),SEEK_SET);
                    fread(&Buffer, sizeof(Bloc), metaBuffer->bloc, ms);
                    //MAJ blocs to 0
                    int temp[2];
                    temp[0] = metaBuffer->first_address;
                    temp[1] = metaBuffer->last_address;
                    MAJTable(ms, 1, 2, temp, 0);
                    //Check MS
                    ptr = CheckMSavailability(ms, metaBuffer->bloc + 1, 1);
                    start = ptr[0];
                    end = ptr[metaBuffer->bloc];
                    added_n = metaBuffer->bloc + 1; 
                    //1- if yes put all blocs 2-else the initial ones and show message
                    fseek(ms, ptr[0]*sizeof(Bloc),SEEK_SET);
                    memset(&Buffer[metaBuffer->bloc],0,sizeof(Bloc));
                    Buffer[metaBuffer->bloc].element[0].ID = ID;
                    strcpy(Buffer[metaBuffer->bloc].element[0].row , data);
                    Buffer[metaBuffer->bloc].element[0].flag = 1;
                    Buffer[metaBuffer->bloc].nbr_element= 1;
                    Buffer[metaBuffer->bloc].next = -1;
                    Buffer[metaBuffer->bloc].last = -1;
                    fwrite(&Buffer, sizeof(Bloc), metaBuffer->bloc+1, ms);
                    memset(&Buffer,0,metaBuffer->first_address*sizeof(Bloc));
                    for(int i = 0; i < metaBuffer->bloc; i++){
                        Buffer[i].next = -1;
                        Buffer[i].last = -1;
                    }
                    fseek(ms, metaBuffer->first_address*sizeof(Bloc),SEEK_SET);
                    fwrite(&Buffer, sizeof(Bloc), metaBuffer->bloc, ms);
                } else {
                    Bloc Buffer = {};
                    start = metaBuffer->first_address;
                    end = metaBuffer->last_address + 1;
                    int a = end;
                    ptr[0] = a;
                    added_n = 1;
                    Buffer.element[0].ID = ID;
                    strcpy(Buffer.element[0].row , data);
                    Buffer.element[0].flag = 1;
                    Buffer.nbr_element= 1;
                    Buffer.next = -1;
                    Buffer.last = -1;
                    fseek(ms, (metaBuffer->last_address + 1)*sizeof(Bloc),SEEK_SET);
                    fwrite(&Buffer, sizeof(Bloc), 1, ms);
                }
                rewind(ms);
                

                int bloc, record;
                bloc = metaBuffer->bloc + 1;
                record = metaBuffer->record + 1;
                int temp1 [2] = {};
                UpdateMeta(ms, name, "", bloc, record, start, end, ptr ,added_n, temp1, 0, 0);
            }else return -1;
        }
        return 0;
    }else return 1;
}


int Insertion_Chainnee_NotOrdered(FILE* ms,char name[], int ID, char data[], MetaBuffer* metaBuffer){
    searchResult result = binarySearchChained(ms, ID, metaBuffer->first_address);
    if (result.blocPosition == -1 && result.elementPosition == -1 && result.isFound == 0){
        if(metaBuffer->record < FB * metaBuffer->bloc){
            Bloc Buffer;
            int i = 0;
            fseek(ms, metaBuffer->last_address*sizeof(Bloc),SEEK_SET);
            fread(&Buffer, sizeof(Bloc), 1, ms);
            if(Buffer.nbr_element == FB){
                defragmentation(ms,name);
                metaBuffer = Get_MetaData(ms,name);
                fseek(ms, metaBuffer->last_address*sizeof(Bloc),SEEK_SET);
                fread(&Buffer, sizeof(Bloc), 1, ms);
            }
            
            while(Buffer.element[i].flag == 1) i++;
            Buffer.element[i].ID = ID;
            strcpy(Buffer.element[i].row ,data);
            Buffer.element[i].flag = 1;
            Buffer.nbr_element += 1;
            fseek(ms, -(long)sizeof(Bloc), SEEK_CUR);
            fwrite(&Buffer, sizeof(Bloc), 1, ms);
            int temp[3] = {};
            int temp0[3] = {};
            UpdateMeta(ms, name, "", -1, metaBuffer->record + 1, -1, -1, temp, 0, temp0, 0, 0);
        }else{
            Bloc Buffer;
            int* ptr = CheckMSavailability(ms, 1, 0);
            if(ptr){
                fseek(ms, metaBuffer->last_address*sizeof(Bloc),SEEK_SET);
                fread(&Buffer, sizeof(Bloc), 1, ms);
                Buffer.next = ptr[0];
                fseek( ms, -(long)sizeof(Bloc), SEEK_CUR);
                fwrite(&Buffer, sizeof(Bloc), 1, ms);
                memset(&Buffer,0,sizeof(Bloc));
                fseek(ms, ptr[0]*sizeof(Bloc),SEEK_SET);
                Buffer.element[0].ID = ID;
                strcpy(Buffer.element[0].row, data);
                Buffer.element[0].flag = 1;
                Buffer.nbr_element= 1;
                Buffer.next = -1;
                Buffer.last = metaBuffer->last_address;
                fwrite(&Buffer, sizeof(Bloc), 1, ms);
                int bloc = metaBuffer->bloc + 1;
                int record = metaBuffer->record + 1;
                int temp[2]={};
                UpdateMeta(ms, name, "", bloc, record, metaBuffer->first_address, ptr[0], ptr ,1, temp, 0, 0);
            }else return -1;
        }
        return 0;
    }else return 1;
}

int Adding_Contiguous_2(FILE* ms, char name[], int bloc, int record,int first_address, int last_address, int ID, char data[]){
    int start = first_address;
    Bloc Buffer;
    int total_recordCounter = 0;
    int total_WrittenBlocCounter = 0;
    int ElementCounter = 0; 
    Bloc Buffer0 = {};
    int added = 0;
    int writeIndex = start;

    for (int i = 0; i < bloc; i++) {
        fseek(ms, sizeof(Bloc) * (start + i), SEEK_SET);
        fread(&Buffer, sizeof(Bloc), 1, ms);

        for (int j = 0; j < Buffer.nbr_element; j++) {
            if (Buffer.element[j].flag) {
                if(Buffer.element[j].ID == ID){
                   return 1;
                }
                else{
                    if(Buffer.element[j].ID > ID && added == 0){
                    Buffer0.element[ElementCounter].ID = ID;
                    strcpy(Buffer0.element[ElementCounter].row, data);
                    Buffer0.element[ElementCounter].flag = 1;
                    ElementCounter++;
                    added = 1;
                    if (ElementCounter == FB) {
                        Buffer0.nbr_element = FB;
                        Buffer0.next = -1;
                        Buffer0.last = -1;
                        fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
                        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                        total_WrittenBlocCounter++;
                        writeIndex++;
                        ElementCounter = 0; 
                        memset(&Buffer0, 0, sizeof(Buffer0)); 
                    }
                }
                Buffer0.element[ElementCounter].ID = Buffer.element[j].ID;
                strcpy(Buffer0.element[ElementCounter].row, Buffer.element[j].row);
                Buffer0.element[ElementCounter].flag = 1;
                ElementCounter++;
                total_recordCounter++;

                if (ElementCounter == FB) {
                    Buffer0.nbr_element = FB;
                    Buffer0.next = -1;
                    Buffer0.last = -1;
                    fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
                    fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                    total_WrittenBlocCounter++;
                    writeIndex++;
                    ElementCounter = 0; 
                    memset(&Buffer0, 0, sizeof(Buffer0)); 
                }}
            }
        }

        if (total_recordCounter == record) break;
    }



    if(!added){
        Buffer0.element[ElementCounter].ID = ID;
        strcpy(Buffer0.element[ElementCounter].row, data);
        Buffer0.element[ElementCounter].flag = 1;
        added = 1 ;
        ElementCounter++;
        total_recordCounter++;
    }
    if (ElementCounter > 0){
        Buffer0.nbr_element = ElementCounter;
        Buffer0.next = -1;
        Buffer0.last = -1;
        fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
        total_WrittenBlocCounter++;
        writeIndex++;
    }
    
    memset(&Buffer0, 0, sizeof(Buffer0));
    Buffer0.next = -1;
    Buffer0.last = -1;
    fseek(ms, sizeof(Bloc) * writeIndex, SEEK_SET);
    for(int i = writeIndex; i <= last_address;i++) fwrite(&Buffer0, sizeof(Bloc), 1, ms);




    int temp[2];
    temp[0] = writeIndex;
    temp[1] = last_address;
    MAJTable(ms, 1, 2, temp, 0);


    
    int temp0[1] = {};
    int temp1[1] = {};
    UpdateMeta(ms, name, "", total_WrittenBlocCounter, record + 1, -1, writeIndex - 1, temp0, 0, temp1, 0, 0);
    return 0;
}



int binarySearchPosition(Element* elements, int nbr_element, int id){ 
    int left = 0; 
    int right = nbr_element - 1; 
    while (left <= right){ 
        int mid = left + (right - left) / 2; 
        if (elements[mid].ID == id){
            return -1;
        }
        else if (elements[mid].ID < id) left = mid + 1;
             else right = mid - 1;
    }          
    return left; 
} 

int Adding_Contiguous_1(FILE* ms,char name[], Bloc Buffer[], int* ptr, int size, int ID, char data[]){
    rewind(ms);
    int left = ptr[0]; 
    int right = ptr[size - 2]; 
    Bloc Buffer0 = {};
    while (left <= right){ 
        int mid = left + (right - left) / 2;
        if(Buffer[mid - ptr[0]].element[0].ID == ID || Buffer[mid - ptr[0]].element[FB - 1].ID == ID){
            int temp1 [2] = {};
            UpdateMeta(ms, name, "", size - 1,  -1, ptr[0], ptr[size - 2], ptr , size - 1, temp1, 0, 0);
            return 1;
        }
        if(Buffer[mid - ptr[0]].element[0].ID < ID && Buffer[mid - ptr[0]].element[FB - 1].ID > ID){
            int index = binarySearchPosition(Buffer[mid - ptr[0]].element, Buffer[mid - ptr[0]].nbr_element, ID); 
            if(index == -1){
                int temp1 [2] = {};
                UpdateMeta(ms, name, "", size - 1,  -1, ptr[0], ptr[size - 2], ptr , size - 1, temp1, 0, 0);
                return 1;
            }else{
                fseek(ms, sizeof(Bloc)* ptr[0], SEEK_SET);
                for(int i = ptr[0]; i < mid; i++){
                    fwrite(&Buffer[i-ptr[0]], sizeof(Bloc), 1, ms);
                }
                for (int i = 0; i < index; i++){
                    Buffer0.element[i].ID = Buffer[mid - ptr[0]].element[i].ID;
                    strcpy(Buffer0.element[i].row, Buffer[mid - ptr[0]].element[i].row);
                    Buffer0.element[i].flag = 1;
                }
                Buffer0.element[index].ID = ID;
                strcpy(Buffer0.element[index].row, data);
                Buffer0.element[index].flag = 1;
                for(int i = index + 1; i < FB; i++){
                    Buffer0.element[i].ID = Buffer[mid - ptr[0]].element[i-1].ID;
                    strcpy(Buffer0.element[i].row, Buffer[mid - ptr[0]].element[i-1].row);
                    Buffer0.element[i].flag = 1;
                }

                Buffer0.nbr_element = FB;
                Buffer0.last = -1;
                Buffer0.next = -1;
                fwrite(&Buffer0, sizeof(Bloc), 1, ms); 
                Buffer0.element[0].ID = Buffer[mid - ptr[0]].element[FB-1].ID;
                strcpy(Buffer0.element[0].row,Buffer[mid - ptr[0]].element[FB-1].row);
                for(int i = mid + 1; i < ptr[size - 1]; i++){
                    Buffer0.element[0].ID = Buffer[i - 1 - ptr[0]].element[FB-1].ID;
                    strcpy(Buffer0.element[0].row,Buffer[i - 1 - ptr[0]].element[FB-1].row);
                    for(int j = 1; j < FB; j++){
                        Buffer0.element[j].ID = Buffer[i - ptr[0]].element[j-1].ID;
                        strcpy(Buffer0.element[j].row, Buffer[i - ptr[0]].element[j-1].row);
                    }
                    fseek(ms, sizeof(Bloc) * i, SEEK_SET);
                    fwrite(&Buffer0, sizeof(Bloc), 1, ms); 
                }
                memset(&Buffer0, 0,sizeof(Bloc));
                Buffer0.element[0].ID = Buffer[ptr[size - 1] - 1 - ptr[0]].element[FB-1].ID;
                strcpy(Buffer0.element[0].row,Buffer[ptr[size - 1] - 1 - ptr[0]].element[FB-1].row);
                Buffer0.element[0].flag = 1;
                Buffer0.nbr_element = 1;
                Buffer0.last = -1;
                Buffer0.next = -1;
                fwrite(&Buffer0, sizeof(Bloc), 1, ms); 
                int temp1 [2] = {};
                UpdateMeta(ms, name, "", size, (size - 1) * FB + 1, ptr[0], ptr[size - 1], ptr , size, temp1, 0, 0);    
                return 0;
            }
        }
        if (Buffer[mid - ptr[0]].element[0].ID > ID){ 
            right = mid - 1; 
        } else{ 
            left = mid + 1;
        } 
    } 
    fseek(ms, sizeof(Bloc) * ptr[0], SEEK_SET); 
    fwrite(&Buffer[0], sizeof(Bloc), left - ptr[0] , ms);
    Buffer0.element[0].ID = ID;
    strcpy(Buffer0.element[0].row, data);
    Buffer0.element[0].flag = 1;
    Buffer0.last = -1;
    Buffer0.next = -1;
    Buffer0.nbr_element = 1;
    fseek(ms, sizeof(Bloc) * left, SEEK_SET); 
    fwrite(&Buffer0, sizeof(Bloc), 1, ms); 
    fseek(ms, sizeof(Bloc) * (left+1), SEEK_SET); 
    fwrite(&Buffer[left - ptr[0]], sizeof(Bloc), ptr[size-1] - left, ms);
    int temp1 [2] = {};
 
    UpdateMeta(ms, name, "", size, (size - 1) * FB + 1, ptr[0], ptr[size - 1], ptr , size, temp1, 0, 0);

    rewind(ms);
    return 0;
}

int Insertion_Contigue_Ordered(FILE* ms, char name[], int ID, char data[], MetaBuffer* metaBuffer){
    if(metaBuffer->record != FB * metaBuffer->bloc){
        return Adding_Contiguous_2(ms, metaBuffer->name, metaBuffer->bloc, metaBuffer->record, metaBuffer->first_address, metaBuffer->last_address, ID, data);
    }else if(CheckMSavailability(ms,1, 0)){
        Bloc Buffer[metaBuffer->bloc + 1];
        fseek(ms, metaBuffer->first_address*sizeof(Bloc),SEEK_SET);
        fread(&Buffer, sizeof(Bloc), metaBuffer->bloc, ms);

        int temp[2];
        temp[0] = metaBuffer->first_address;
        temp[1] = metaBuffer->last_address;
        MAJTable(ms, 1, 2, temp, 0);

        int* ptr = CheckMSavailability(ms, metaBuffer->bloc + 1, 1);
        return Adding_Contiguous_1(ms,name, Buffer, ptr, metaBuffer->bloc + 1, ID, data);
    }else return -1;
}



int Adding_Chainnee_1(FILE* ms, char name[], int ID, char data[], int first_address, int bloc, int record) {
    int start = first_address;
    Bloc Buffer;
    int total_recordCounter = 0;
    int ElementCounter = 0; 
    Bloc Buffer0 = {};
    int BlocCounter = 0;
    int nexts[bloc];
    nexts[0] = start;
    int added = 0;
    for(int i = 0; i < bloc; i++){
        fseek(ms, sizeof(Bloc) * nexts[i], SEEK_SET);
        fread(&Buffer, sizeof(Bloc), 1, ms);      
        nexts[i+1] = Buffer.next;
        for(int j = 0; j < FB; j++){
            if (Buffer.element[j].flag == 1){
                if(Buffer.element[j].ID == ID){
                    return 1;
                }else{
                    if(Buffer.element[j].ID > ID && added == 0){
                        Buffer0.element[ElementCounter].ID = ID;
                        strcpy(Buffer0.element[ElementCounter].row, data);
                        Buffer0.element[ElementCounter].flag = 1;
                        ElementCounter++;
                        added = 1 ;
                        if(ElementCounter == FB){
                        //WRITE
                        Buffer0.nbr_element = FB;
                        if (BlocCounter == 0) Buffer0.last = -1;
                        else Buffer0.last = nexts[BlocCounter-1];
                        Buffer0.next = nexts[BlocCounter+1];
                        ElementCounter = 0;
                        fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
                        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                        BlocCounter++;
                        memset(&Buffer0, 0, sizeof(Bloc)); 
                        }
                    }
                    total_recordCounter++;
                    Buffer0.element[ElementCounter].ID = Buffer.element[j].ID;
                    strcpy(Buffer0.element[ElementCounter].row, Buffer.element[j].row);
                    Buffer0.element[ElementCounter].flag = 1;
                    ElementCounter ++;
                    if(ElementCounter == FB){
                        //WRITE
                        Buffer0.nbr_element = FB;
                        if (BlocCounter == 0) Buffer0.last = -1;
                        else Buffer0.last = nexts[BlocCounter-1];
                        if(total_recordCounter == record){
                            Buffer0.next = -1;
                            ElementCounter = 0;
                            fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
                            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                            BlocCounter++;
                            break;
                        }
                        Buffer0.next = nexts[BlocCounter+1];
                        ElementCounter = 0;
                        fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
                        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                        BlocCounter++;
                        memset(&Buffer0, 0, sizeof(Bloc)); 
                    }
                }
            } 
        }
        if(total_recordCounter == record) break;  
    }    

    if(!added){
        Buffer0.element[ElementCounter].ID = ID;
        strcpy(Buffer0.element[ElementCounter].row, data);
        Buffer0.element[ElementCounter].flag = 1;
        added = 1 ;
        ElementCounter++;
        total_recordCounter++;
    }
    if(ElementCounter != 0){
        //WRITE
        Buffer0.nbr_element = ElementCounter;
        if (BlocCounter == 0) Buffer0.last = -1;
        else Buffer0.last = nexts[BlocCounter-1];
        Buffer0.next = -1;
        fseek(ms,sizeof(Bloc)*nexts[BlocCounter],SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
        BlocCounter++;
    }
    int *deleted = nexts + (BlocCounter);
    memset(&Buffer0, 0, sizeof(Bloc));
    Buffer0.next = -1;
    Buffer0.last = -1;
    for(int i = 0; i < bloc - BlocCounter; i++){
        fseek(ms, sizeof(Bloc) * deleted[i] , SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
    }
    int temp0[1] = {};
    UpdateMeta(ms, name, "", BlocCounter, record + 1, -1, nexts[BlocCounter-1], temp0, 0, deleted, bloc-BlocCounter, 0);
    return 0;           
} 



int Adding_Chainnee_2(FILE* ms, char name[], int ID, char data[], int first_address, int bloc, int added_bloc){
    rewind(ms);
    Bloc Buffer; 
    Bloc Buffer0 = {};
    int currentBlock = first_address; 

    int* addresses = Get_ChainneeAddresses(ms, first_address); 

    int left = 0; 
    int right = bloc - 1; 
    int mid;
    while (left <= right){ 
        mid = left + (right - left) / 2; 
        
        currentBlock = addresses[mid]; 
        
        fseek(ms, sizeof(Bloc) * currentBlock, SEEK_SET); 
        fread(&Buffer, sizeof(Bloc), 1, ms); 
        if(Buffer.element[0].ID == ID || Buffer.element[FB - 1].ID == ID){
            return 1;
        }   
        if(Buffer.element[0].ID < ID && Buffer.element[FB-1].ID > ID){ 
            int index = binarySearchPosition(Buffer.element, Buffer.nbr_element, ID); 
            if (index == -1){
                return 1;
            }else{
                if(index < FB/2){
                    for(int i = 0; i < index; i++){
                        Buffer0.element[i].ID = Buffer.element[i].ID;
                        strcpy(Buffer0.element[i].row,Buffer.element[i].row);
                        Buffer0.element[i].flag = 1;
                        memset(&Buffer.element[i],0,sizeof(Element));
                    }
                    Buffer0.element[index].ID = ID;
                    strcpy(Buffer0.element[index].row,data);
                    Buffer0.element[index].flag = 1;
                    Buffer0.nbr_element = index + 1;
                    Buffer0.last = Buffer.last;
                    Buffer0.next = addresses[mid];
                    Buffer.nbr_element = FB - index;
                    Buffer.last = added_bloc;
                    fseek(ms, currentBlock*sizeof(Bloc), SEEK_SET);
                    fwrite(&Buffer, sizeof(Bloc), 1, ms);
                    fseek(ms, added_bloc*sizeof(Bloc), SEEK_SET);
                    fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                    if(Buffer0.last != -1){
                        fseek(ms, addresses[mid - 1]*sizeof(Bloc), SEEK_SET);
                        fread(&Buffer, sizeof(Bloc), 1, ms);
                        Buffer.next = added_bloc;
                        fseek(ms, addresses[mid - 1]*sizeof(Bloc), SEEK_SET);
                        fwrite(&Buffer, sizeof(Bloc), 1, ms);
                    }
                }else{
                    Buffer0.element[index-1].ID = ID;
                    strcpy(Buffer0.element[index-1].row,data);
                    Buffer0.element[index-1].flag = 1;
                    for(int i = index ; i < FB; i++){
                        Buffer0.element[i].ID = Buffer.element[i].ID;
                        strcpy(Buffer0.element[i].row,Buffer.element[i].row);
                        Buffer0.element[i].flag = 1;
                        memset(&Buffer.element[i],0,sizeof(Element));
                    }
                    Buffer0.nbr_element = FB - index + 1;
                    Buffer0.last = addresses[mid];
                    Buffer0.next = Buffer.next;
                    Buffer.nbr_element = index;
                    Buffer.next = added_bloc;
                    fseek(ms, currentBlock*sizeof(Bloc), SEEK_SET);
                    fwrite(&Buffer, sizeof(Bloc), 1, ms);
                    fseek(ms, added_bloc*sizeof(Bloc), SEEK_SET);
                    fwrite(&Buffer0, sizeof(Bloc), 1, ms);
                    if(Buffer0.next != -1){
                        fseek(ms, addresses[mid + 1]*sizeof(Bloc), SEEK_SET);
                        fread(&Buffer, sizeof(Bloc), 1, ms);
                        Buffer.last = added_bloc;
                        fseek(ms, addresses[mid + 1]*sizeof(Bloc), SEEK_SET);
                        fwrite(&Buffer, sizeof(Bloc), 1, ms);
                    }
                }
                int temp[2] = {added_bloc};
                UpdateMeta(ms, name, "", bloc + 1, bloc * FB + 1, -1, added_bloc, temp , 1, temp, 0, 0);    
                return 0;
            }
        }
        if (Buffer.element[0].ID > ID) right = mid - 1;
        else left = mid + 1;
    }
    Buffer0.element[0].ID = ID;
    strcpy(Buffer0.element[0].row, data);
    Buffer0.element[0].flag = 1;
    Buffer0.nbr_element = 1;

    if (left == bloc) {
        Buffer0.last = addresses[bloc - 1];
        Buffer0.next = -1;
        fseek(ms, sizeof(Bloc) * added_bloc, SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);

        fseek(ms, sizeof(Bloc) * addresses[bloc - 1], SEEK_SET);
        fread(&Buffer, sizeof(Bloc), 1, ms);
        Buffer.next = added_bloc;
        fseek(ms, sizeof(Bloc) * addresses[bloc - 1], SEEK_SET);
        fwrite(&Buffer, sizeof(Bloc), 1, ms);
    } else {
        Buffer0.last = (left == 0) ? -1 : addresses[left - 1]; 
        Buffer0.next = addresses[left]; 

        fseek(ms, sizeof(Bloc) * added_bloc, SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);

        if (left > 0) {
            fseek(ms, sizeof(Bloc) * addresses[left - 1], SEEK_SET);
            fread(&Buffer0, sizeof(Bloc), 1, ms);
            Buffer0.next = added_bloc;
            fseek(ms, sizeof(Bloc) * addresses[left - 1], SEEK_SET);
            fwrite(&Buffer0, sizeof(Bloc), 1, ms);
        }

        fseek(ms, sizeof(Bloc) * addresses[left], SEEK_SET);
        fread(&Buffer0, sizeof(Bloc), 1, ms);
        Buffer0.last = added_bloc;
        fseek(ms, sizeof(Bloc) * addresses[left], SEEK_SET);
        fwrite(&Buffer0, sizeof(Bloc), 1, ms);
    }
    int temp[2] = {added_bloc};
    UpdateMeta(ms, name, "", bloc + 1, bloc * FB + 1, -1, added_bloc, temp , 1, temp, 0, 0);    
    rewind(ms);
    return 0;
}

int Insertion_Chainnee_Ordered(FILE* ms, char name[], int ID, char data[], MetaBuffer* metaBuffer){
    if(metaBuffer->record != FB * metaBuffer->bloc){
        return Adding_Chainnee_1(ms, name, ID, data, metaBuffer->first_address, metaBuffer->bloc, metaBuffer->record);
    }else{ 
        int* ptr = CheckMSavailability(ms,1, 0);
        if(ptr){
            return Adding_Chainnee_2(ms, name, ID, data, metaBuffer->first_address, metaBuffer->bloc, *ptr);
        }else return -1;
    }
}


int insertElement(FILE* ms, char name[], int ID, char data[]){
    MetaBuffer* metaBuffer = Get_MetaData(ms,name);
    if(metaBuffer->contigue){
        if(metaBuffer->ordered) return Insertion_Contigue_Ordered(ms, name, ID, data, metaBuffer);
        else return Insertion_Contigue_NotOrdered(ms, name, ID, data, metaBuffer);
    }else{
        if(metaBuffer->ordered) return Insertion_Chainnee_NotOrdered(ms, name, ID, data, metaBuffer);
        else return Insertion_Chainnee_NotOrdered(ms, name, ID, data, metaBuffer);
    }
} 