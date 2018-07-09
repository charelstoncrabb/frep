// frep.h

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#ifndef __FREP_H__
#define __FREP_H__

#include "frep.doc.h"

#define EQSTR 0 // return value of string compare strcmp() = 0 if the strings are equal
#define SS_MAXLEN 1000 // max search string length; TODO make this dynamic to allow for user optimization

/* This typedef is for clarity, and used as the character buffer while file parsing */
typedef int 
longchar;

/* Error handling functions */
int
Argv_Error(void)
{
	fprintf(stderr,"Error parsing command line arguments!");
	return -1;
};

int
Fopen_Error(const char* filename)
{
	fprintf(stderr,"Error while opening file [%s]!",filename);
	return 2;
};

/* Option struct for book-keeping available command-line arguments */
typedef 
struct
{
	char* name;		// Option name to be used on command line
	char* verboseName;	// Verbose option name to be used on command line
	char* value;		// Value option is set to
	bool  isSet;		// Boolean whether the option is set
	char* doc;		// Documentation/description of option
} Option;

void 
new_Option(Option* self, const char* n, const char* vn, const char* v, const char* d)
{
	self->name = (char*)malloc(sizeof(char)*strlen(n));
	strcpy(self->name,n);
	self->verboseName = (char*)malloc(sizeof(char)*strlen(vn));
	strcpy(self->verboseName,vn);
	self->value = (char*)malloc(sizeof(char));
	strcpy(self->value,"\0"); // Currently unused
	self->doc = (char*)malloc(sizeof(char)*strlen(d));
	strcpy(self->doc,d);
}

void 
free_Option(Option* self)
{
	free(self->name);
	free(self->verboseName);
	free(self->value);
	free(self->doc);
}

/* argvOptions struct for tracking specified command-line arguments */
typedef 
struct
{
	Option* version;
	Option* verbose;
	Option* help;
	Option* searchfile;
	Option* insensitive;
	char*  inFilename;
	char*  ftFilename;
	char*  repstr;
	int    nSearchStrings;
	size_t*ssLengths;
	char** search;
	FILE*  inFile;
	FILE*  ftFile;
	bool*  lastSSMatch;
	bool   execute;
} argvOptions;

/* Forward Declarations */
void reset_lastSSMatch(argvOptions*);
bool any(bool*,int);
char* stripnewline(char*);
bool isLetter(const char);

/* Initialize argvOptions struct for command-line argument parsing */
void
new_argvOptions(argvOptions* self)
{
	self->version = (Option*)malloc(sizeof(Option));
	new_Option(self->version,"v","-version","",verdoc);
	self->verbose = (Option*)malloc(sizeof(Option));
	new_Option(self->verbose,"b","-verbose","",verbdoc);
	self->help = (Option*)malloc(sizeof(Option));
	new_Option(self->help,"h","-help","",helpdoc);
	self->searchfile = (Option*)malloc(sizeof(Option));
	new_Option(self->searchfile,"sf","-search-file","",sfdoc);
	self->insensitive = (Option*)malloc(sizeof(Option));
	new_Option(self->insensitive,"i","-insensitive","",idoc);
	self->inFilename = NULL;
	self->ftFilename = NULL;
	self->search = NULL;
	self->nSearchStrings = 0;
	self->ssLengths = NULL;
	self->repstr = NULL;
	self->inFile = NULL;
	self->ftFile = NULL;
	self->lastSSMatch = NULL;
	self->execute = true;
	return;
};

void
free_argvOptions(argvOptions* self)
{
	free_Option(self->version);
	free(self->version);
	free_Option(self->verbose);
	free(self->verbose);
	free_Option(self->help);
	free(self->help);
	free_Option(self->searchfile);
	free(self->searchfile);
	free_Option(self->insensitive);
	free(self->insensitive);
	for(int i = 0; i < self->nSearchStrings; i++)
		free(self->search[i]);
	free(self->search);
	free(self->repstr);
	free(self->ssLengths);
	free(self->lastSSMatch);
	fclose(self->inFile);
	fclose(self->ftFile);
}

/* Function for parsing command line arguments
Use:
    $ frep [options] [file] [^searchstring] [newstring]

^Exception:
Use if --search-file(-sf) [SEARCHSTRINGFILE] is specified:
    $ frep [options] [file] [newstring]

*/
int
parse_argv(argvOptions* self, 
			int argc,
			char** argv)
{
	int rval = -1,
		searchstrPosition = argc-2, // initialize searchstring arg to second-to-last
		inFilePosition = argc-3;
	char searchStringBuffer[SS_MAXLEN];
	FILE* searchFile;
	const char ftFnamePrefix[11] = ".freptemp.";
	if( argc < 2 )
		return Argv_Error();
	else
	{
		// Parse newstring argument:
		self->repstr = (char*)malloc(sizeof(char)*(strlen(argv[argc-1])+1));
		strcpy(self->repstr,argv[argc-1]);
		for(int i = 1; i < argc && argv[i][0] == '-'; i++)
		{
			if( strlen(argv[i]) < 2 )
				return Argv_Error();
			// Parse version option
			if( strcmp((argv[i]+1),self->version->name) == EQSTR || strcmp(argv[i]+1,self->version->verboseName) == EQSTR )
			{
				self->version->isSet = true;
				self->execute = false;
				printf("%s",self->version->doc);
				return 0;
			}
			// Parse help option
			else if( strcmp(argv[i]+1,self->help->name) == EQSTR || strcmp(argv[i]+1,self->help->verboseName) == EQSTR )
			{
				self->help->isSet = true;
				self->execute = false;
				printf("%s",self->help->doc);
				return 0;
			}
			// Parse case insensitive option
			else if( strcmp(argv[i]+1,self->insensitive->name) == EQSTR || strcmp(argv[i]+1,self->insensitive->verboseName) == EQSTR )
			{
				self->insensitive->isSet = true;
				// self->execute = true; (DEFAULT)
			}
			// Parse verbose outputting option
			else if( strcmp(argv[i]+1,self->verbose->name) == EQSTR || strcmp(argv[i]+1,self->verbose->verboseName) == EQSTR )
			{
				self->verbose->isSet = true;
				// self->execute = true; (DEFAULT)
			}
			// Parse search-file option
			else if( strcmp(argv[i]+1,self->searchfile->name) == EQSTR || strcmp(argv[i]+1,self->searchfile->verboseName) == EQSTR )
			{
				searchFile = fopen(argv[i+1],"r");
				if( searchFile == NULL )
					return Fopen_Error(argv[i+1]);
				// Parse the number of search strings from searchstring file
				while( fgets(searchStringBuffer,SS_MAXLEN,searchFile) )
					self->nSearchStrings++;
				// Initialize strings for storing search strings
				self->search = (char**)malloc(sizeof(char*)*(self->nSearchStrings));
				for(int i = 0; i < self->nSearchStrings; i++)
					self->search[i] = (char*)malloc(sizeof(char)*SS_MAXLEN);
				// Reset searchstring file stream 
				rewind(searchFile);
				// Copy search strings from file into search string array
				for(int i = 0; i < self->nSearchStrings; i++){
					strcpy(self->search[i],fgets(searchStringBuffer,SS_MAXLEN,searchFile));
					stripnewline(self->search[i]);
				}
				searchstrPosition = 0; // Set position to zero since search string successfully parsed from file
				inFilePosition = argc-2; // Reset input filename position since search strings parsed from file
				i++; // skip to next argv string
				fclose(searchFile); // close searchstring file stream
				// self->execute = true; (DEFAULT)
			}
			else
				return Argv_Error();
		}
		// If search-file option has not been specified, use second-to-last argument as searchstring
		if( searchstrPosition == argc-2 )
		{
			self->search = (char**)malloc(sizeof(char*));
			self->search[0] = (char*)malloc(sizeof(char)*(strlen(argv[searchstrPosition])+1));
			strcpy(self->search[0],argv[searchstrPosition]);
			self->nSearchStrings = 1;
		}
		// Set individual search string lengths
		self->ssLengths = (size_t*)malloc(sizeof(size_t)*self->nSearchStrings);
		for(int i = 0; i < self->nSearchStrings; i++)
			self->ssLengths[i] = strlen(self->search[i]);
		// Open input file and set to options FILE*
		self->inFilename = (char*)malloc(sizeof(char)*strlen(argv[inFilePosition]));
		strcpy(self->inFilename,argv[inFilePosition]);
		self->inFile = fopen(self->inFilename,"r");
		if( self->inFile == NULL )
		{
			self->execute = false;
			return Fopen_Error(self->inFilename);
		}
		
		// Use input file name to set up frep temporary file
		self->ftFilename = (char*)malloc(sizeof(char)*(strlen(argv[inFilePosition])+11));
		strcpy(self->ftFilename,".freptemp.");
		strcat(self->ftFilename,argv[inFilePosition]);
		self->ftFile = fopen(self->ftFilename,"w+");
		if( self->ftFile == NULL )
		{
			self->execute = false;
			return Fopen_Error(self->ftFilename);
		}
	}
	// Initialize search string matching bool array
	self->lastSSMatch = (bool*)malloc(sizeof(bool)*self->nSearchStrings);
	reset_lastSSMatch(self);
	return 0;
};

void 
printArgvOptions(argvOptions* self)
{
	printf("Input file name: %s\n",self->inFilename);
	printf("Replace string: %s\n",self->repstr);
	printf("Number of search strings: %d\n",self->nSearchStrings);
	printf("Search string(s):\n");
	for(int i = 0; i < self->nSearchStrings; i++)
		printf("\t%s (length: %lu)\n",self->search[i],strlen(self->search[i]));
}

void 
reset_lastSSMatch(argvOptions* self)
{
	for(int i = 0; i < self->nSearchStrings; i++)
		self->lastSSMatch[i] = true;
}

int 
searchmatch(argvOptions* self, char buffer, int pos)
{
	for(int i = 0; i < self->nSearchStrings; i++){
		if( pos < self->ssLengths[i] ){
			char compchar = self->search[i][pos];
			bool charmatch = (compchar == buffer);
			if( self->insensitive->isSet ) // If --insensitive option is set, perform case-insensitive char match
				if( isLetter(compchar) && isLetter(buffer) && ((compchar-buffer)%32) == 0 )
					charmatch = true;
			self->lastSSMatch[i] = self->lastSSMatch[i] && charmatch;
		}
		else
			self->lastSSMatch[i] = false;
		if( self->lastSSMatch[i] )
			return i;
	}
	return -1;
}

void 
write_repstr(argvOptions* self)
{
	size_t repstrlen = strlen(self->repstr);
	for(size_t i = 0; i < repstrlen; i++)
		fputc(self->repstr[i],self->ftFile);
}

char* 
stripnewline(char* str)
{
	size_t len = strlen(str);
	for(;len > 0 && ( str[len-1] == '\n' || str[len-1] == '\r' ); len--){
		str[len-1] = '\0';
	}
	return str;
}

bool 
any(bool* boolarray, int size)
{
	for(int i = 0; i < size; i++)
		if( boolarray[i] )
			return true;
	return false;
}

bool
isLetter(const char c)
{
	if( ( c >= 65 && c <= 90 ) || ( c >= 97 && c <= 122 ) )
		return true;
	return false;
}

#endif
