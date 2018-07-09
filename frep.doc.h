
#ifndef __FREP_DOC_H_
#define __FREP_DOC_H_

#define DOCLEN 700

/* Application Version documentation and contact information  */
const char verdoc[DOCLEN] = "\
frep - version 1.0.0\n\
    f(ile) rep(lace): string find and replace command line tool\n\
\n\
\n\
For bug reports, contact charcr466@gmail.com\n\
\n\
\n\
";

/* Application help documentation and options definitions */
const char helpdoc[DOCLEN] = "\
frep - version 1.0.0\n\
Use:\n\
    $ frep [options] [file] [searchstring] [newstring]\n\
\n\
    --help ( -h ):\n\
        display this help message\n\
\n\
    --version ( -v ):\n\
        display version information\n\
\n\
    --search-file [FILE] ( -sf [FILE] ):\n\
        use each line of text in [FILE] as search string to be replaced by [newstring]\n\
\n\
    --insensitive ( -i ):\n\
        use case-insensitive character matching\n\
\n\
    --verbose ( -b ):\n\
        verbose outputting to stdout during program execution\n\
\n\
Future capability:\n\
    --opt-sstr [SEARCHSTRING LENGTH] ( -oss [SEARCHSTRING LENGTH] ) :\n\
        optimize search string length for memory if using many short search strings\n\
\n\
";

/* Search file option documentation */
const char sfdoc[DOCLEN] = "Use lines of text in [FILE] as search string";

/* Case-insensitive option documentation */
const char idoc[DOCLEN] = "Use case-insensitive character matching";

/* Verbose outputting documentation */
const char verbdoc[DOCLEN] = "verbose outputting to stdout during program execution";

#endif
