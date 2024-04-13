#ifndef UTILS_H
#define UTILS_H

#include <iostream>
#include <ostream>
#include <sstream>
#include <string>
#include <cstring>

// Copies 'length' number of characters beginning at the index position 'begin'
// from array 'data.'
char* Substring(char* array, int begin, size_t length);

// Finds the delimiter characters specified in 'delim' within 'array' beginning
// from array[begin], unless 'begin' is unspecified; its default value is 0. 
// Returns the position of the starting position of the delimiter, if found; 
// otherwise -1 is returned.
int Find(char* array, char delim[], int begin = 0);

// Merges two supplied character arrays at the end of arr1 and beginning of arr2; 
// if delim is specified, it is placed in the middle of arr1 and arr2.
// Returns a null-terminated heap-allocated character array pointer. 
char* Merge(char* arr1, char* arr2, const char* delim = NULL);

char* Append(const char* destination, const char* source);

const char* FileLineFunc(const char* file, int line, const char* function);

void LogSocketError(const char* msg, int wsagla);
// overload for a message to be output to std::cerr.
void LogSocketError(const char* msg);
// overload for more robust and technical error logging.
void LogSocketError(const char* msg, int wsagla, const char* file, int line, const char* function);

template<typename T>
void LogSocket(std::ostream& stream, T data) {

    stream << data;

}

template<typename T, typename... Args>
void LogSocket(std::ostream& stream, T data, Args... args) {

    stream << data;
    LogSocket(stream, args...);

}

#endif