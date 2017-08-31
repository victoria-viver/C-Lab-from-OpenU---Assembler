#define LINE_MAX_LENGTH 100

#define LABEL_MAX_LENGTH 30
#define INSTRUCTION_MAX_LENGTH 6
#define CMD_MAX_LENGTH 4
#define CMD_MAX_ARGS 2

#define WORD_MAX_LENGTH 10
#define ADDRESS_MAX_LENGTH 8
#define BASE_4_MAX_LENGTH 4
#define REGISTER_MAX_LENGTH 4
#define CMD_CODE_MAX_LENGTH 4
#define ADDRESSING_CODE_MAX_LENGTH 2
#define ENCODING_TYPE_MAX_LENGTH 2

#define UNKNOWN_CMD -1
#define EMPTY 0
#define EQUAL 0

#define ROWS 1
#define COLUMNS 4

#define ABSOLUTE 0
#define EXTERNAL 1
#define RELOCATABLE 2

#define NUMBER 0
#define VARIABLE 1
#define MATRIX 2
#define REGISTER 3

#define FILE_LINE_FORMAT "%s\t%s\n"

#define IC_START 100
#define DC_START 0

typedef struct symbolNode *symbolsListNodePointer;
typedef struct symbolNode
{
	char *label;
	int address;
	int isExtern;
	int isCmd;
	symbolsListNodePointer next;
} symbolsListNode;

typedef struct encodingNode *encodingListNodePointer;
typedef struct encodingNode
{
	int address;
	char *word;
	encodingListNodePointer next;
} encodingListNode;

/*****VARIABLES****/

int ic = IC_START;
int dc = DC_START;

int lineID = 0;

FILE *objectFile;
FILE *entryFile;
FILE *externFile;

symbolsListNodePointer symbolsListHead = NULL;
symbolsListNodePointer symbolsListTail = NULL;

encodingListNodePointer dataEncodingListHead = NULL;
encodingListNodePointer dataEncodingListTail = NULL;

encodingListNodePointer codeEncodingListHead = NULL;
encodingListNodePointer codeEncodingListCurrentCmd = NULL;
encodingListNodePointer codeEncodingListTail = NULL;

/****FUNCTIONS*****/

void openFiles (char *fileName);

void firstReadParseLine (char line[]);
void secondReadParseLine (char line[]);

int isInstruction (char *word);
int isLabel (char *word);
int getCommandCode (char *word);

char *convertFromDecToBin (int code, int length);
int   convertFromBinToDec (char *word);
char *convertToAlphaBase4 (int code);

int getCmdArgsNum (int cmdCode);
int getAddressingCode (char *word);

void addToSymbolsList ( char *label, 
			int address,
			int isExtern,
			int isCmd);
int isExistInSymbolsList (char *label);
void printSymbolsList ();

void addToDataEncodingList (int value);
void printDataEncodingList ();

encodingListNodePointer addToCodeEncodingList (char *word);
void printCodeEncodingList ();

void updateAddressesAfterFirstRead ();

int findAddressInSymbolsList (char *label);
