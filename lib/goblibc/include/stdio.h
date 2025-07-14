#pragma once
#include <features.h>
#include <file.h>
#include <stdarg.h>
/* The value returned by fgetc and similar functions to indicate the
   end of the file.  */
#define EOF (-1)

#define feof(f) ((f)->flags & F_EOF)
#define ferror(f) ((f)->flags & F_ERR)

/// @brief Reads data from the stream and stores them according to parameter format into the locations pointed by the elements in the variable argument list identified by arg.
/// @param file The source stream 
/// @param fmt Format spec string
/// @param args Values to write data to
/// @return 
extern int gob_vfscanf(FILE *file, const char *fmt, va_list args);

/// @brief Read formatted input from a string and stores the result in the provided variables. 
/// @param str String to read from
/// @param fmt Format of the string 
/// @param  
/// @return The amount of matches in the provided strign
int gob_sscanf(const char *str, const char *fmt, ...);

/// @brief Format a string using sprintf formatting system
/// @param str Target string
/// @param fmt Formating options
/// @param
/// @return -1 on failure
int gob_sprintf(char *str, const char *fmt, ...);

/// @brief Loads the data from the locations, defined by vlist, converts them to character string equivalents and writes the results to a virtual file
/// @param file Destination to which to write values to
/// @param fmt Format using printf formatting symbols
/// @param args provided list of values
/// @return 
int gob_vfprintf(FILE *file, const char *fmt, va_list args);
