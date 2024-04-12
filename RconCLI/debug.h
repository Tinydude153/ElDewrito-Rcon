#ifndef DEBUG_H
#define DEBUG_H

// Macros for toggling debug output printing.

//#define DEBUG

#ifdef DEBUG
#define PRINTDEBUG(...) printf(__VA_ARGS__)
#else 
#define PRINTDEBUG(...) 
#endif

#endif