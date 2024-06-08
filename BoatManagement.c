#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_STRING 128
#define MAX_BOATS 120

typedef char String[MAX_STRING];

typedef enum {
	slip,
	land,
	trailor,
	storage,
	no_place
} PlaceType;

typedef struct {
	int slipNumber; 
} Slip;

typedef struct {
	char bayLetter; 
} Land;

typedef struct {
	String trailorTag; 
} Trailor;

typedef struct {
	int storageNumber; 
} Storage;

typedef union {
	Slip slip;
	Land land;
	Trailor trailor;
	Storage storage;
} BoatInfo; 

typedef struct {
	String name;
	int length;
	PlaceType place;
	BoatInfo boatInfo;
	double moneyOwed;
} BoatData;
//-------------------------------------------------------------------------------------------------
void FileRead(BoatData **BoatArray, int *NumBoats, char *fileName);
void MenuOptions(BoatData **BoatArray, int *NumBoats);
void PrintBoatInventory(BoatData **BoatArray, int *NumBoats);
void AddBoat(BoatData **BoatArray, int *NumBoats);
void RemoveBoat(BoatData **BoatArray, int *NumBoats);
void AcceptPayment(BoatData **BoatArray, int *NumBoats);
void UpdateOwed(BoatData **BoatArray, int *NumBoats);
int BoatComparator(const void *a, const void *b);
PlaceType StringToPlaceType(char * PlaceString);
char * PlaceToString(PlaceType Place);
void FileWrite(BoatData **BoatArray, int *NumBoats, char *fileName);
void *Malloc(size_t Size);
void Free(void **Memory);
//-------------------------------------------------------------------------------------------------
int main(int argc, char *argv[]) {
	//make an array of 120 pointers that point to BoatData
	BoatData **BoatArray = (BoatData**) Malloc(MAX_BOATS * sizeof(BoatData*));
	for (int i = 0; i < MAX_BOATS; i++) {
    	BoatArray[i] = NULL;
	}
	//check if memory was allocated
	if(BoatArray == NULL){
		printf("ERROR: Memory not allocated\n");
		return(EXIT_FAILURE);
	}

	//initialize the number of boats to 0
	int NumBoats = 0;

	//check command line arguments
	if(argc != 2){
		printf("ERROR: Incorrect number of arguments\n");
		return(EXIT_FAILURE);
	}

	printf("Welcome to the Boat Management System\n");
	printf("-------------------------------------\n");
	//read the file and store the data in the BoatArray
	FileRead(BoatArray, &NumBoats, argv[1]);
	//display the menu 
	MenuOptions(BoatArray, &NumBoats);
	//write changed boat array to the file 
	FileWrite(BoatArray, &NumBoats, argv[1]);

	printf("Exiting the Boat Management System\n");
	//free the memory
	for (int i = 0; i < MAX_BOATS; i++) {
		Free((void **) &BoatArray[i]);
	}
	return (EXIT_SUCCESS);
}
//-------------------------------------------------------------------------------------------------
void FileRead(BoatData **BoatArray, int *NumBoats, char *fileName) {
	//open the file
	FILE *InventoryFile;
	if ((InventoryFile = fopen(fileName, "r+")) != NULL) {
		//initialize the variables to hold the boat data
		char name_holder[MAX_STRING];
		int length_holder;
		char place_holder[MAX_STRING];
		double moneyOwed_holder;
		char boatInfo_holder[MAX_STRING];

		//scan the file and store the data in the BoatArray
		while ((fscanf(InventoryFile, "%127[^,],%d,%127[^,],%127[^,],%lf\n", name_holder, &length_holder, place_holder, boatInfo_holder, &moneyOwed_holder)) != EOF) {
			//allocate memory for each boat as it is read in
			BoatArray[*NumBoats] = (BoatData*) Malloc(sizeof(BoatData));
			strcpy(BoatArray[*NumBoats]->name, name_holder);
			BoatArray[*NumBoats]->length = length_holder;
			BoatArray[*NumBoats]->place = StringToPlaceType(place_holder);

			//store the additional boat info depending on place stored
			switch (BoatArray[*NumBoats]->place) {
				case slip:
					BoatArray[*NumBoats]->boatInfo.slip.slipNumber = atoi(boatInfo_holder);
					break;
				case land:
					BoatArray[*NumBoats]->boatInfo.land.bayLetter = boatInfo_holder[0];
					break;
				case trailor:
					strcpy(BoatArray[*NumBoats]->boatInfo.trailor.trailorTag, boatInfo_holder);
					break;
				case storage:
					BoatArray[*NumBoats]->boatInfo.storage.storageNumber = atoi(boatInfo_holder);
					break;
				default:
					printf("Unknown place type.\n");
					break;
			}

			BoatArray[*NumBoats]->moneyOwed = moneyOwed_holder;
			(*NumBoats)++;

			//sort the array of boats
			qsort(BoatArray, *NumBoats, sizeof(BoatData*), BoatComparator);
		}
		//if the file did not open
	} else {
		printf("ERROR: File not found\n");
		return;
	}
	fclose(InventoryFile);
	return;
}
//-------------------------------------------------------------------------------------------------
void MenuOptions(BoatData **BoatArray, int *NumBoats) {
	//display the menu options and call the appropriate function
	char input;
	char option = '\0';

	while(option != 'X'){
		printf("(I)nventory, (A)dd, (R)emove, (P)ayment, (M)onth, e(X)it : ");
		scanf("%c", &input);
		option = toupper(input);
		int c;
    	while ((c = getchar()) != '\n' && c != EOF);
		switch (option) {
		case 'I':
			PrintBoatInventory(BoatArray, NumBoats);
			break;
		case 'A':
			AddBoat(BoatArray, NumBoats);
			break;
		case 'R':
			RemoveBoat(BoatArray, NumBoats);
			break;
		case 'P':
			AcceptPayment(BoatArray, NumBoats);
			break;
		case 'M':
			UpdateOwed(BoatArray, NumBoats);
			break;
		case 'X':
			break;
		//check for invalid input
		default:
			printf("Invalid Option %c\n", option);
			break;
		}
	}
}
//-------------------------------------------------------------------------------------------------
void PrintBoatInventory(BoatData **BoatArray, int *NumBoats) {
	//print the boat inventory, boat by boat, in sorted order by name
	int i;
	char boatInfo[MAX_STRING];

	qsort(BoatArray, *NumBoats, sizeof(BoatData*), BoatComparator);
	for (i = 0; i < *NumBoats; i++) {
		switch (BoatArray[i]->place){
        case slip:
            sprintf(boatInfo, "# %d", BoatArray[i]->boatInfo.slip.slipNumber);
            break;
        case land:
            sprintf(boatInfo, "%c", BoatArray[i]->boatInfo.land.bayLetter);
            break;
        case trailor:
            strcpy(boatInfo, BoatArray[i]->boatInfo.trailor.trailorTag);
            break;
        case storage:
            sprintf(boatInfo, "# %d", BoatArray[i]->boatInfo.storage.storageNumber);
            break;
        default:
            boatInfo[0] = '\0';
            break;
        }
		printf("%-20s\t%d'\t%s\t%s\t Owes $%.2f\n", BoatArray[i]->name, BoatArray[i]->length, PlaceToString(BoatArray[i]->place), boatInfo, BoatArray[i]->moneyOwed);
	}
	return;
}
//-------------------------------------------------------------------------------------------------
void AddBoat(BoatData **BoatArray, int *NumBoats) {
	//add a boat to the BoatArray from user input
	char name_holder[MAX_STRING];
	int length_holder;
	char place_holder[MAX_STRING];
	double moneyOwed_holder;
	char boatInfo_holder[MAX_STRING];

	printf("Please enter the boat data in CSV format                     : ");
	scanf("%127[^,],%d,%127[^,],%127[^,],%lf%*c", name_holder, &length_holder, place_holder, boatInfo_holder, &moneyOwed_holder);
	BoatArray[*NumBoats] = (BoatData*) Malloc(sizeof(BoatData));
	strcpy(BoatArray[*NumBoats]->name, name_holder);
	BoatArray[*NumBoats]->length = length_holder;
	BoatArray[*NumBoats]->place = StringToPlaceType(place_holder);
	switch (BoatArray[*NumBoats]->place) {
		case slip:
			BoatArray[*NumBoats]->boatInfo.slip.slipNumber = atoi(boatInfo_holder);
			break;
		case land:
			BoatArray[*NumBoats]->boatInfo.land.bayLetter = boatInfo_holder[0];
			break;
		case trailor:
			strcpy(BoatArray[*NumBoats]->boatInfo.trailor.trailorTag, boatInfo_holder);
			break;
		case storage:
			BoatArray[*NumBoats]->boatInfo.storage.storageNumber = atoi(boatInfo_holder);
			break;
		default:
			printf("Unknown place type.\n");
			break;
	}
	BoatArray[*NumBoats]->moneyOwed = moneyOwed_holder;
	qsort(BoatArray, *NumBoats, sizeof(BoatData*), BoatComparator);
	(*NumBoats)++;
	return;
}
//-------------------------------------------------------------------------------------------------
void RemoveBoat(BoatData **BoatArray, int *NumBoats) {
	//remove a boat from the BoatArray given the name
	char name[MAX_STRING];
	int i;

	printf("Please enter the boat name                               : ");
	fgets(name, MAX_STRING, stdin);
	name[strcspn(name, "\n")] = '\0';
	for (i = 0; i < *NumBoats; i++) {
		//if the name is found, shift the array to remove the boat
		if (!strcasecmp(name, BoatArray[i]->name)) {
			for (int j = i; j < *NumBoats - 1; j++) {
				BoatArray[j] = BoatArray[j + 1];
			}
			//and decrement the number of boats
			(*NumBoats)--;
			return;
		}
	}
	printf("No boat with that name\n");
	return;
}
//-------------------------------------------------------------------------------------------------
void AcceptPayment(BoatData **BoatArray, int *NumBoats) {
	//accept a payment for a boat given the name
	double payment;
	char name[MAX_STRING];
	int i;

	printf("Please enter the boat name                               : ");
	fgets(name, MAX_STRING, stdin);
	name[strcspn(name, "\n")] = '\0';
	for (int i = 0; i < *NumBoats; i++) {
		//if the name is found, subtract the payment from the money owed
		if (!strcasecmp(name, BoatArray[i]->name)) {
			printf("Please enter the amount to be paid                       : ");
			scanf("%lf", &payment);
			int c;
    		while ((c = getchar()) != '\n' && c != EOF);

			//check if the payment exceeds the amount owed
			if(payment > BoatArray[i]->moneyOwed){
				printf("That is more than the amount owed, $%.2f\n", BoatArray[i]->moneyOwed);
				return;
			} else {
				BoatArray[i]->moneyOwed -= payment;
				return;
			}
		}
	}
	printf("No boat with that name\n");
	return;
}
//-------------------------------------------------------------------------------------------------
void UpdateOwed(BoatData **BoatArray, int *NumBoats) {
	//update the amount owed for each boat based on the monthly rate per foot
	for(int i = 0; i < *NumBoats; i++){
		switch (BoatArray[i]->place){
			case slip:
				BoatArray[i]->moneyOwed += (12.50*BoatArray[i]->length);
				break;
			case land:
				BoatArray[i]->moneyOwed += (14.0*BoatArray[i]->length);
				break;
			case trailor:
				BoatArray[i]->moneyOwed += (25.0*BoatArray[i]->length);
				break;
			case storage:
				BoatArray[i]->moneyOwed += (11.20*BoatArray[i]->length);
				break;
			default:
				break;
		}
	}
	return;
}
//-------------------------------------------------------------------------------------------------
int BoatComparator(const void *a, const void *b) {
	//compare the names of two boats and return 0 if they are the same, 1 if BoatA is greater, -1 if BoatB is greater
	BoatData *BoatA = *(BoatData**) a;
	BoatData *BoatB = *(BoatData**) b;
	//comparing case insensitive since the rest of the program is case insensitive
	return(strcasecmp(BoatA->name, BoatB->name));
}
//-------------------------------------------------------------------------------------------------
void FileWrite(BoatData **BoatArray, int *NumBoats, char *fileName) {
	//open the file for writing, add the updated BoatArray to the file, and close the file
	FILE *InventoryFile;
	InventoryFile = fopen(fileName, "w");
	if (InventoryFile == NULL) {
		printf("ERROR: File not found\n");
		return;
	}
	for (int i = 0; i < *NumBoats; i++) {
		char boatInfo_holder[MAX_STRING];
		switch (BoatArray[i]->place) {
			case slip:
				sprintf(boatInfo_holder, "%d", BoatArray[i]->boatInfo.slip.slipNumber);
				break;
			case land:
				sprintf(boatInfo_holder, "%c", BoatArray[i]->boatInfo.land.bayLetter);
				break;
			case trailor:
				strcpy(boatInfo_holder, BoatArray[i]->boatInfo.trailor.trailorTag);
				break;
			case storage:
				sprintf(boatInfo_holder, "%d", BoatArray[i]->boatInfo.storage.storageNumber);
				break;
			default:
				printf("Unknown place type.\n");
				break;
		}
		fprintf(InventoryFile, "%s,%d,%s,%s,%lf\n", BoatArray[i]->name, BoatArray[i]->length, PlaceToString(BoatArray[i]->place), boatInfo_holder, BoatArray[i]->moneyOwed);
	}
	fclose(InventoryFile);
}
//-------------------------------------------------------------------------------------------------
PlaceType StringToPlaceType(char * PlaceString) {
	if (!strcasecmp(PlaceString,"slip")) {
		return(slip);
		}
		if (!strcasecmp(PlaceString,"land")) {
			return(land);
		}
		if (!strcasecmp(PlaceString,"trailor")) {
			return(trailor);
		}
		if (!strcasecmp(PlaceString,"storage")) {
			return(storage);
		}
	return(no_place);
}
//-------------------------------------------------------------------------------------------------
char * PlaceToString(PlaceType Place) {
	switch (Place) {
		case slip:
			return("slip");
		case land:
			return("land");
		case trailor:
			return("trailor");
		case storage:
			return("storage");
		case no_place:
			return("no_place");
		default:
			exit(EXIT_FAILURE);
			break;
	}
}
//-------------------------------------------------------------------------------------------------
void *Malloc(size_t Size) {
	void * Memory;
	if ((Memory = malloc(Size)) == NULL) {
		perror("Cannot malloc");
		exit(EXIT_FAILURE);
	} else {
		return(Memory);
	}
}
//-------------------------------------------------------------------------------------------------
void Free(void ** Memory) {
	free(*Memory);
	*Memory = NULL;
	return;
}
//-------------------------------------------------------------------------------------------------
