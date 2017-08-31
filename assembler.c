/* Program Assembler
* Description: Program gets input assembly file, converts it and builds 3 output files
* @author Victoria Shenkevich
* @date 20/08/2017
*/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "assembler.h"

/*Main program function controls the proccess*/
int main (int argc, char *argv[])
{
	if (argc == 2 && (access (argv[1], F_OK) != -1))
	{
		FILE *inputFile = fopen (argv[1], "r");
		char line [LINE_MAX_LENGTH];

		openFiles (argv[1]);

		fprintf (stdout, "\n-----FIRST READ-----\n");

		while (fgets (line, sizeof (line), inputFile)) 
		{
			if (line[0] != ';' && line[0] != '\n')
				 firstReadParseLine (line);
			lineID++;
		}

		updateAddressesAfterFirstRead ();

		printSymbolsList ();
		printDataEncodingList ();

		fprintf (objectFile, FILE_LINE_FORMAT, convertToAlphaBase4(ic-IC_START), convertToAlphaBase4(dc-DC_START));
		ic = IC_START;
		lineID = 0;

		rewind (inputFile);

		fprintf (stdout, "\n-----SECOND READ-----\n");

		while (fgets (line, sizeof (line), inputFile)) 
		{
			if (line[0] != ';' && line[0] != '\n')
				 secondReadParseLine (line);
			lineID++;
		}

		codeEncodingListTail->next = dataEncodingListHead;

		printCodeEncodingList ();
	}
	else
	{
		fprintf (stderr, "ERROR: LINE %d - ILLEGAL PROGRAM ARGUMENTS NUMBER\n", lineID);
		return 1;
	}

	return 0;
}

/*Function gets a name and opens files for writing*/
void openFiles (char *fileName)
{
	int fileNameLength = strlen (fileName);

	objectFile = fopen (strcat (fileName, ".obj"), "w");
	fileName[fileNameLength] = '\0';

	entryFile = fopen (strcat (fileName, ".ent"), "w");
	fileName[fileNameLength] = '\0';

	externFile = fopen (strcat (fileName, ".ext"), "w");
	fileName[fileNameLength] = '\0';
}

/*Function gets a line and performs first read processing*/
void firstReadParseLine (char line[]) 
{
	char *pch = (char *) malloc (sizeof (char) * strlen (line));

	char *label = (char *) malloc (sizeof (char) * (LABEL_MAX_LENGTH + 2));
	char *instruction = (char *) malloc (sizeof (char) * (INSTRUCTION_MAX_LENGTH + 2));
	char *cmd = (char *) malloc (sizeof (char) * (CMD_MAX_LENGTH + 1));

	int currentWordID = 0;

	int cmdArgsCounter = 0;
	int startAddress;
	int isExtern = 0;
	int isR = 0;
	int matSize = 0;
	int matCurrent = 0;

	pch = strtok (line, " ,\t\n\"");

	while (pch)
	{	
		if (instruction[0] == '\0' && cmd[0] == '\0')
		{
			if (isLabel (pch))
			{
				label = pch;
				label[strlen(label) - 1] = '\0';
			}
			else if (isInstruction (pch))
			{
				instruction = pch;
				startAddress = dc;
			}
			else if (getCommandCode (pch) != UNKNOWN_CMD)
			{
				cmd = pch;
				startAddress = ic;
				ic++;
			}
			else
			{
				fprintf (stderr, "ERROR: LINE %d - ILLEGAL LINE\n", lineID);
				break;
			}		
		}
		else if (cmd[0] != '\0')
		{
			cmdArgsCounter++;
			
			if (cmdArgsCounter <= getCmdArgsNum (getCommandCode (cmd)))
			{
				ic++;
				
				if (getAddressingCode (pch) == MATRIX)
					ic++;
				else if (getAddressingCode (pch) == REGISTER)
				{	
					if (!isR) 
						isR++;
					else
						ic--;
				}
			}
			else
			{
				fprintf (stderr, "ERROR: LINE %d - ILLEGAL COMMAND ARGUMENTS NUMBER\n", lineID);
				break;
			}
		}
		else if (instruction[0] != '\0')
		{
			if (strcmp (instruction, ".entry") == EQUAL)
				break;
			else if (strcmp (instruction, ".extern") == EQUAL)
			{
				label = pch;
				startAddress = 0;
				isExtern++;
				break;
			}
			else if (strcmp (instruction, ".string") == EQUAL)
			{
				int i = 0;
				while (pch[i] != '\0')
					addToDataEncodingList (pch[i++]);
				addToDataEncodingList (0);
			}
			else if (strcmp (instruction, ".mat") == EQUAL)
			{
				if (matSize == 0)
					matSize = (pch[ROWS] - '0') * (pch[COLUMNS] - '0');
				else if (matCurrent++ < matSize)
					addToDataEncodingList (atoi (pch));
				else
					fprintf (stderr, "ERROR: LINE %d - ILLEGAL MATRIX PARAMETERS NUMBER\n", lineID);
			}
			else
				addToDataEncodingList (atoi (pch));
		}
		
		currentWordID++;
		pch = strtok (NULL, " ,\t\n\"");
	}		

	if (label[0] != '\0')
	{
		addToSymbolsList (label, 
				 startAddress,
				 isExtern,
				 cmd[0] != '\0' ? 1 : 0);
	}
	else if (instruction[0] != '\0' && strcmp (instruction, ".mat") == EQUAL)
	{
		while (matCurrent++ < matSize)
			addToDataEncodingList (0);
	}
}

/*Function gets a line and performs second read processing*/
void secondReadParseLine (char line[])
{	
	char *pch = (char *) malloc (sizeof (char) * strlen (line));

	char *label = (char *) malloc (sizeof (char) * (LABEL_MAX_LENGTH + 2));
	char *instruction = (char *) malloc (sizeof (char) * (INSTRUCTION_MAX_LENGTH + 2));
	char *cmd = (char *) malloc (sizeof (char) * (CMD_MAX_LENGTH + 1));
	char *cmdCode = (char *) malloc (sizeof (char) * (CMD_CODE_MAX_LENGTH + 1));

	int currentWordID = 0;

	int cmdArgsCounter = 0;
	int isR = 0;

	fprintf (stdout, "\n%s", line);

	pch = strtok (line, " ,\t\n\"");

	while (pch)
	{	
		if (instruction[0] == '\0' && cmd[0] == '\0')
		{
			if (isLabel (pch))
			{
				label = pch;
				label[strlen(label) - 1] = '\0';
			}
			else if (isInstruction (pch))
			{
				instruction = pch;

				if (strcmp (instruction, ".entry") != EQUAL)
					break;
			}
			else if (getCommandCode (pch) != UNKNOWN_CMD)
			{
				cmd = pch;
				cmdCode = convertFromDecToBin (getCommandCode (cmd), CMD_CODE_MAX_LENGTH);
				codeEncodingListCurrentCmd = addToCodeEncodingList (cmdCode);

				if (getCmdArgsNum (getCommandCode (cmd)) == 0)
					strcat (codeEncodingListCurrentCmd->word, convertFromDecToBin (EMPTY, ADDRESSING_CODE_MAX_LENGTH * CMD_MAX_ARGS));
			}
			else
			{
				fprintf (stderr, "ERROR: LINE %d - ILLEGAL LINE\n", lineID);
				break;
			}		
		}
		else if (cmd[0] != '\0')
		{
			cmdArgsCounter++;

			if (getCmdArgsNum (getCommandCode (cmd)) == 1)
				strcat (codeEncodingListCurrentCmd->word, convertFromDecToBin (EMPTY, ADDRESSING_CODE_MAX_LENGTH));

			if (cmdArgsCounter <= getCmdArgsNum (getCommandCode (cmd)))
			{
				strcat (codeEncodingListCurrentCmd->word, convertFromDecToBin (getAddressingCode (pch), ADDRESSING_CODE_MAX_LENGTH));

				if (getAddressingCode (pch) == NUMBER)
					addToCodeEncodingList (convertFromDecToBin (pch[1] - '0', WORD_MAX_LENGTH));	
				else if (getAddressingCode (pch) == VARIABLE)
				{
					int address = findAddressInSymbolsList (pch);

					addToCodeEncodingList (convertFromDecToBin (address, ADDRESS_MAX_LENGTH));

					if (address == 0)
					{
						strcat (codeEncodingListTail->word, convertFromDecToBin (EXTERNAL, ENCODING_TYPE_MAX_LENGTH));
						fprintf (externFile, FILE_LINE_FORMAT, pch, convertToAlphaBase4 (codeEncodingListTail->address));
					}
					else
						strcat (codeEncodingListTail->word, convertFromDecToBin (RELOCATABLE, ENCODING_TYPE_MAX_LENGTH));
				}
				else if (getAddressingCode (pch) == MATRIX)	
				{
					int address;

					char *matIndexes = (char *) malloc (sizeof (char) * (REGISTER_MAX_LENGTH * 2 + 1));

					int rowsR = pch[strchr (pch, '[') + 1 + ROWS - pch] - '0';
					int colR = pch[strchr (pch, '[') + 2 + COLUMNS - pch]- '0';
					
					strcpy (matIndexes, convertFromDecToBin(rowsR, REGISTER_MAX_LENGTH));
					strcat (matIndexes, convertFromDecToBin(colR, REGISTER_MAX_LENGTH));
					strcat (matIndexes, convertFromDecToBin(ABSOLUTE, ENCODING_TYPE_MAX_LENGTH));

					label = pch;
					label[strchr (label, '[') - label]= '\0';

					address = findAddressInSymbolsList (label);

					addToCodeEncodingList (convertFromDecToBin (address, ADDRESS_MAX_LENGTH));

					if (address == 0)
					{
						strcat (codeEncodingListTail->word, convertFromDecToBin (EXTERNAL, ENCODING_TYPE_MAX_LENGTH));
						fprintf (externFile, FILE_LINE_FORMAT, pch, convertToAlphaBase4 (codeEncodingListTail->address));
					}
					else
						strcat (codeEncodingListTail->word, convertFromDecToBin (RELOCATABLE, ENCODING_TYPE_MAX_LENGTH));
					
					addToCodeEncodingList(matIndexes);
				}
				else if (getAddressingCode (pch) == REGISTER)
				{	
					if (!isR)
					{
						addToCodeEncodingList (convertFromDecToBin (pch[1] - '0', REGISTER_MAX_LENGTH));
						isR++;
					}
					else
						strcat (codeEncodingListTail->word, convertFromDecToBin (pch[1] - '0', REGISTER_MAX_LENGTH));
				}
			}
			else
			{
				fprintf (stderr, "ERROR: LINE %d - ILLEGAL COMMAND ARGUMENTS NUMBER\n", lineID);
				break;
			}
		}
		else if (instruction[0] != '\0')
		{
			fprintf (entryFile, FILE_LINE_FORMAT, pch, convertToAlphaBase4 (findAddressInSymbolsList (pch)));
			break;
		}
		
		currentWordID++;
		pch = strtok (NULL, " ,\t\n\"");
	}

	if (cmd[0] != '\0')
	{
		strcat (codeEncodingListCurrentCmd->word, convertFromDecToBin (ABSOLUTE, ENCODING_TYPE_MAX_LENGTH));
		
		if (isR)
		{
			if (strlen (codeEncodingListTail->word) == REGISTER_MAX_LENGTH)
				strcat (codeEncodingListTail->word, convertFromDecToBin (0, REGISTER_MAX_LENGTH));

			strcat (codeEncodingListTail->word, convertFromDecToBin (ABSOLUTE, ENCODING_TYPE_MAX_LENGTH));
		}
		
		fprintf (stdout, "cmd adrs:%d word:%s\n", codeEncodingListCurrentCmd->address, codeEncodingListCurrentCmd->word);
		encodingListNodePointer t = codeEncodingListCurrentCmd->next;
		
		while (t)
		{	
			fprintf (stdout, "    adrs:%d word:%s\n", t->address, t->word);
			t = t->next;
		}
	}
}

/*Function gets word and returns if it's a instruction*/
int isInstruction (char *word)
{
	if (word[0] == '.')
		return 1;
	return 0;
}

/*Function gets word and returns if it's a label*/
int isLabel (char *word)
{
	int length = strlen (word);

	if (word[length - 1] == ':')
	{
		if (isalpha (word[0]) && length < LABEL_MAX_LENGTH)
			return 1;
		else
			fprintf (stderr, "ERROR: LINE %d - ILLEGAL NAME FOR LABEL\n", lineID);
	}

	return 0;
}

/*Function gets command word and returns its code*/
int getCommandCode (char *word)
{
	if 	(strcmp (word, "mov") == EQUAL)
		return 0;
	else if (strcmp (word, "cmp") == EQUAL)
		return 1;
	else if (strcmp (word, "add") == EQUAL)
		return 2;
	else if (strcmp (word, "sub") == EQUAL)
		return 3;
	else if (strcmp (word, "lea") == EQUAL)
		return 6;
	
	else if (strcmp (word, "not") == EQUAL)
		return 4;
	else if (strcmp (word, "clr") == EQUAL)
		return 5;
	else if (strcmp (word, "inc") == EQUAL)
		return 7;
	else if (strcmp (word, "dec") == EQUAL)
		return 8;
	else if (strcmp (word, "jmp") == EQUAL)
		return 9;
	else if (strcmp (word, "bne") == EQUAL)
		return 10;
	else if (strcmp (word, "red") == EQUAL)
		return 11;
	else if (strcmp (word, "prn") == EQUAL)
		return 12;
	else if (strcmp (word, "jsr") == EQUAL)
		return 13;

	else if (strcmp (word, "rts") == EQUAL)
		return 14;
	else if (strcmp (word, "stop") == EQUAL)
		return 15;
	else
		return UNKNOWN_CMD;
}

/*Function gets decimal number and returns its binary representation (in length that function gets)*/
char *convertFromDecToBin (int code, int length)
{
	int isNegative = code < 0 ? 1 : 0;
	int isFirstFound = 0;
	int i = length;
	char *bin = (char *) malloc (sizeof (char) * (length+1));

	while (i > 0)
	{
		if (isNegative && isFirstFound)
			bin [--i] = code%2 ? '0' : '1';
		else
		{
			bin [--i] = code%2 ? '1' : '0';
			
			if (isNegative && code%2)
				isFirstFound++;
		}
			
		code /= 2;
	}
		
	return bin;
}

/*Function gets binary word and returns its decimal representation*/
int convertFromBinToDec (char *word)
{
	int code = 0;
	int i = strlen(word);
	int power = 0;

	while (i > 0)
		code += (pow (2, power++) * (word[--i] - '0'));	

	return code;
}

/*Function gets decimal number and returns its alpha base 4 representation*/
char *convertToAlphaBase4 (int code)
{
	int i = 4;/*FIXME*/
	char *alphaBase4 = (char *) malloc (sizeof (char) * (i+1));

	while (i > 0)
	{
		int modulo = code%4;

		if (modulo == 0)
			alphaBase4 [--i] = 'a';
		else if (modulo == 1)
			alphaBase4 [--i] = 'b';
		else if (modulo == 2)
			alphaBase4 [--i] = 'c';
		else if (modulo == 3)
			alphaBase4 [--i] = 'd';					

		code /= 4;		
	}

	return alphaBase4;
}

/*Function gets command code and returns the number of arguments command should get*/
int getCmdArgsNum (int cmdCode)
{
	if ( (cmdCode >= 0 && cmdCode <= 3) || cmdCode == 6)
		return 2;
	else if (cmdCode >= 4 && cmdCode <= 13 && cmdCode != 6)
		return 1;
	else
		return 0;
}

/*Function gets word, indicates its type and returns addressing code*/
int getAddressingCode (char *word)
{
	if (word[0] == '#')
		return 0;
	else if (word[0] == 'r')
		return 3;
	else
	{
		int length = strlen (word);
	
		if (word[length-1] == ']')
			return 2;
		else
			return 1;
	}
}

/*Function adds new symbol and its information to symbols table*/
void addToSymbolsList ( char *label, 
			int address,
			int isExtern,
			int isCmd)
{
	if (!isExistInSymbolsList (label))
	{
		symbolsListNodePointer t = (symbolsListNodePointer) malloc (sizeof (symbolsListNode));

		t->label = malloc (sizeof (char) * (strlen (label) + 1));
		strcpy (t->label, label);

		t->address = address;
		t->isExtern = isExtern;

		if (!isExtern)
			t->isCmd = isCmd;
		else
			t->isCmd = -1;
	
		if (symbolsListHead == NULL)
		{
			symbolsListHead = t;
			symbolsListTail = symbolsListHead;
		}
		else
		{
			symbolsListTail->next = t;
			symbolsListTail = t;
		}
	}
	else
		fprintf (stderr, "ERROR: LINE %d - SYMBOL ALREADY EXISTS\n", lineID);
}

/*Function checks by label if symbol already exists symbols table*/
int isExistInSymbolsList (char *label)
{
	int isExist = 0;
	symbolsListNodePointer t = symbolsListHead;

	while (t)
	{	
		if (strcmp (label, t->label) == EQUAL)
			isExist++;		
		t = t->next;
	}

	return isExist;
}

/*Function prints symbols table*/
void printSymbolsList ()
{
	symbolsListNodePointer t = symbolsListHead;

	fprintf (stdout, "\n-----SYMBOLS TABLE-----\n");

	while (t)
	{	
		fprintf (stdout, "\nlabel:%s\n", t->label);
		fprintf (stdout, "address:%d\n", t->address);
		fprintf (stdout, "isExtern:%d\n", t->isExtern);
		fprintf (stdout, "isCmd:%d\n", t->isCmd);
		
		t = t->next;
	}
}

/*Function adds new word to data table*/
void addToDataEncodingList (int value)
{
	encodingListNodePointer t = (encodingListNodePointer) malloc (sizeof (encodingListNode));

	t->address = dc;
	t->word = convertFromDecToBin (value, WORD_MAX_LENGTH);

	if (dataEncodingListHead == NULL)
	{
		dataEncodingListHead = t;
		dataEncodingListTail = dataEncodingListHead;
	}
	else

	{
		dataEncodingListTail->next = t;
		dataEncodingListTail = t;
	}

	dc++;
}

/*Function prints data table*/
void printDataEncodingList ()
{
	encodingListNodePointer t = dataEncodingListHead;

	fprintf (stdout, "\n-----DATA ENCODING TABLE-----\n");

	while (t)
	{	
		fprintf (stdout, "\naddress:%d\n", t->address);
		fprintf (stdout, "word:%s\n", t->word);
		
		t = t->next;
	}
}

/*Function updates addresses in symbols and data tables after first read*/
void updateAddressesAfterFirstRead ()
{
	symbolsListNodePointer t = symbolsListHead;
	encodingListNodePointer d = dataEncodingListHead;

	while (t)
	{	
		if (!(t->isExtern) && !(t->isCmd))
			t->address += ic;
		t = t->next;
	}
	
	while (d)
	{	
		d->address += ic;
		d = d->next;
	}
}

/*Function adds new word to codes table*/
encodingListNodePointer addToCodeEncodingList (char *word)
{
	encodingListNodePointer t = (encodingListNodePointer) malloc (sizeof (encodingListNode));

	t->word = malloc (sizeof (char) * (WORD_MAX_LENGTH + 1));
	strcpy (t->word, word);

	t->address = ic;

	ic++;

	if (codeEncodingListHead == NULL)
	{
		codeEncodingListHead = t;
		codeEncodingListTail = codeEncodingListHead;
	}
	else
	{
		codeEncodingListTail->next = t;
		codeEncodingListTail = t;
	}

	return t;
}

/*Function prints codes table in alpha base format*/
void printCodeEncodingList ()
{
	encodingListNodePointer t = codeEncodingListHead;

	fprintf (stdout, "\n-----CODE ENCODING TABLE-----\n");

	while (t)
	{	
		fprintf (stdout, "\naddress:%d\n", t->address);
		fprintf (stdout, "word:%s\n", t->word);

		fprintf (objectFile, FILE_LINE_FORMAT, convertToAlphaBase4(t->address), convertToAlphaBase4(convertFromBinToDec(t->word)));
		
		t = t->next;
	}
}

/*Function looks for symbol by its label and returns symbols address*/
int findAddressInSymbolsList (char *label)
{
	symbolsListNodePointer t = symbolsListHead;

	while (t)
	{	
		if (strcmp (label, t->label) == EQUAL)
			return t->address;
		
		t = t->next;
	}
	
	return 0;
}
