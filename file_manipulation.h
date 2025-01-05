#ifndef FILE_MANIPULATION_H
#define FILE_MANIPULATION_H

#include <stdio.h>
#include <stdbool.h>
#include "ms.h"






//S T R U C T U R E S
typedef struct searchResult{
    int blocPosition;
    int elementPosition;
    int isFound;
}searchResult;


typedef struct MetaBuffer {
	int Address_bloc;
	int Address_record;
	char name[21];
	int bloc;
	int record;
	int first_address;
	int last_address;
	int contigue;
	int ordered;
	int flag;
} MetaBuffer;





// F U N C T I O N S
void defragmentation(FILE *ms, char fileName[]);
MetaBuffer* ElementToMetaBufferPart(Element meta);
Element* MetaBufferToElement(MetaBuffer Buffer);
MetaBuffer* Get_MetaData(FILE* ms, char name[]);
int* Get_ChainneeAddresses(FILE* ms,int first_address);
void defragmentation_Meta(FILE* ms);
void Create_MetaData(FILE* ms, char name[], int bloc, int ne, int first, int last, int contigue, int ordered);
void ChargeFile(FILE* ms, FILE* inputFile, int size, int nbr_e, int* addressList, int contigue);
int Create_file(FILE* ms,FILE* inputFile, char name[], int ne, int contigue, int ordered);
int binarySearchElements(Element* elements, int nbr_element, int id);
searchResult binarySearchContiguous(FILE* ms, int id, int firstBlock, int lastBlock);
searchResult binarySearchChained(FILE* ms, int id, int firstBlock);
searchResult linearSearchContiguous(FILE* ms, int id, int firstBlock, int lastBlock);
searchResult linearSearchChained(FILE* ms, int id, int firstBlock);
searchResult searchElementByID(FILE* ms, char fileName[], int id);
bool LogicalSuppression(FILE* ms, char fileName[], int id);
void PhysicalSuppression(FILE* ms, char filename[], int id);
int Insertion_Contigue_NotOrdered(FILE* ms,char name[], int ID, char data[], MetaBuffer* metaBuffer);
int Insertion_Chainnee_NotOrdered(FILE* ms,char name[], int ID, char data[], MetaBuffer* metaBuffer);
int UpdateMeta(FILE* ms, char curr_name[], char new_name[],int bloc,int record, int first_address, int last_address, int Add_Addresses[],int added_a, int delete_Addresses[],int deleted_a, int delete);
int Adding_Contiguous_2(FILE* ms, char name[], int bloc, int record,int first_address, int last_address, int ID, char data[]);
int binarySearchPosition(Element* elements, int nbr_element, int id);
int Adding_Contiguous_1(FILE* ms,char name[], Bloc Buffer[], int* ptr, int size, int ID, char data[]);
int Insertion_Contigue_Ordered(FILE* ms, char name[], int ID, char data[], MetaBuffer* metaBuffer);
int Adding_Chainnee_1(FILE* ms, char name[], int ID, char data[], int first_address, int bloc, int record);
int Adding_Chainnee_2(FILE* ms, char name[], int ID, char data[], int first_address, int bloc, int added_bloc);
int Insertion_Chainnee_Ordered(FILE* ms, char name[], int ID, char data[], MetaBuffer* metaBuffer);
int insertElement(FILE* ms, char name[], int ID, char data[]);




#endif