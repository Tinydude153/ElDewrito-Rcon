#include <RconCLI/utils.h>

// Copies 'length' number of characters beginning at the index position 'begin'
// from array 'data.'
char* Substring(char* array, int begin, size_t length) {

    // Ensure validity of length.
    if (length < 1) {
        return NULL;
    }

    // Increment length for NULL-byte.
    length++;

    // Allocate memory then copy 'length' amount of characters.
    char* out = (char*)malloc(sizeof(char*) * length + 1);
    memset(out, 0, length + 1);
    for (int i = begin, x = 0; i < length; i++, x++) {
        memcpy(&out[x], &array[i], 1);
    }
    out[length] = '\0';
    
    return out;

}

// Finds the delimiter characters specified in 'delim' within 'array' beginning
// from array[begin], unless 'begin' is unspecified; its default value is 0. 
// Returns the position of the starting position of the delimiter, if found; 
// otherwise -1 is returned.
int Find(char* array, char delim[], int begin) {

    // Ensure proper indexing.
    if (begin > strlen(array)) {
        return -1;
    }

    // This algorithm is deductive and isn't 100% reliable, but suffices.
    for (int i = begin; i < strlen(array); i++) {
        if (array[i] == delim[0] && array[i + strlen(delim) - 1] == delim[strlen(delim) - 1]) {
            return i;
        } else {continue;}
    } 
    
    // Otherwise, return -1 for failure.
    return -1;

}

// Merges two supplied character arrays at the end of arr1 and beginning of arr2; 
// if delim is specified, it is placed in the middle of arr1 and arr2.
char* Merge(char* arr1, char* arr2, const char* delim) {

    size_t length;
    if (delim) {

        length = strlen(arr1) + strlen(arr2) + strlen(delim);

    } else { length = strlen(arr1) + strlen(arr2); }
    length++; // null-byte
    char* merged = (char*)malloc(sizeof(char*) * length);
    memset(merged, 0, length);

    memcpy(merged, arr1, strlen(arr1));
    if (delim) {

        memcpy(merged + strlen(merged), delim, strlen(delim));

    }
    memcpy(merged + strlen(merged), arr2, strlen(arr2));
    merged[length] = 0;

    return merged;

}

char* Append(const char* destination, const char* source) {

    size_t length = strlen(destination) + strlen(source) + 1;
    char* appended = (char*)malloc(sizeof(char*) * length);
    memset(appended, 0, length);

    memcpy(appended, source, strlen(source));
    memcpy(appended + strlen(appended), destination, strlen(destination));
    
    return appended;

}

const char* FileLineFunc(const char* file, int line, const char* function) {

    std::stringstream ss;
    ss << '[' << file << ':' << line << ':' << function << "()" << "] ";
    return ss.str().c_str();

}

void LogSocketError(const char* msg, int wsagla) {

    LogSocket(std::cerr, "[SOCKET] ", "Winsock Error ", wsagla, " | ", msg, '\n');

}

void LogSocketError(const char* msg) {

    LogSocket(std::cerr, "[SOCKET] ", msg, '\n');

}

void LogSocketError(const char* msg, int wsagla, const char* file, int line, const char* function) {

    LogSocket(std::cerr, "[SOCKET] ", '[', file, ':', line, ':', function, "(): ", msg, wsagla, '\n');

}