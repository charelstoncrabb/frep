#include "frep.h"
/*
This program performs a string search and replace in a specified file.
*/
int 
main(int argc, char** argv)
{
	/* VARIABLE DECLARATIONS */
	longchar buffer; // Character stream from input file
	argvOptions* argv_options; // Command line options struct
	argv_options = (argvOptions*)malloc(sizeof(argvOptions));
	size_t sssize, rssize, nFoundSS = 0;
	long offset; // Buffer offset that tracks string comparison location

	/* VARIABLE INITIALIZATIONS */
	new_argvOptions(argv_options);

	/* PROGRAM EXECUTION */
	parse_argv(argv_options,argc,argv);
	if( argv_options->execute )
	{
		if( argv_options->verbose->isSet )
			printArgvOptions(argv_options);
		buffer = fgetc(argv_options->inFile);
		while( buffer != EOF )
		{
			// Search and dump to temp file until we hit search string
			if( searchmatch(argv_options,buffer,0) >= 0 )
			{
				offset = 0; // offset tracks location of finding start of search string
				// Iterate until we stop matching the search strings
				for(; buffer != EOF && searchmatch(argv_options,buffer,-offset) >= 0; offset--)
					buffer = fgetc(argv_options->inFile);
				// If buffer is not end-of-file, then check last character
				if( buffer != EOF )
				{
					offset++;
					fseek(argv_options->inFile,-2,SEEK_CUR);
					buffer = fgetc(argv_options->inFile);
					reset_lastSSMatch(argv_options);
				}
				// Return to start of search string position in file
				fseek(argv_options->inFile,offset,SEEK_CUR);
				// If the full length of any search string was found, write the repstring to the temp file 
				if( searchmatch(argv_options,buffer,-offset) >= 0 )
				{
					nFoundSS++;
					write_repstr(argv_options);
					fseek(argv_options->inFile,-offset,SEEK_CUR);
				}// otherwise write the partially-found string to frep temp file
				else
					for(;offset < 0; offset++)
						fputc(fgetc(argv_options->inFile),argv_options->ftFile);
				reset_lastSSMatch(argv_options);
			}
			else
			{
				fputc(buffer,argv_options->ftFile);
				reset_lastSSMatch(argv_options);
			}
			buffer = fgetc(argv_options->inFile);
		}
	}
	// Remove input file and rename the temporary frep file to the original input file
	if( argv_options->verbose->isSet )
		printf("Found %lu occurences of search strings\n",nFoundSS);

	if( nFoundSS > 0 ){
		remove(argv_options->inFilename);
		rename(argv_options->ftFilename,argv_options->inFilename);
	}
	else
		remove(argv_options->ftFilename);
	free_argvOptions(argv_options);
	exit(0);
}
