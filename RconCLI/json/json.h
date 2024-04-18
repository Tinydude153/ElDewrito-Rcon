#ifndef JSON_H
#define JSON_H

#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <vector>

class JSON {

    typedef enum TOKEN {

        BRACE_OPEN,
        BRACE_CLOSED,
        STRING,
        NUMBER,
        FLOAT,
        BRACKET_OPEN,
        BRACKET_CLOSED,
        BOOLEAN_TRUE,
        BOOLEAN_FALSE,
        COLON,
        COMMA,
        NULL_TYPE,
        OTHER

    } TOKEN_TYPE;

    typedef enum TYPE {
        
        JSON_STRING,
        JSON_OBJECT,
        JSON_OBJECT_LAST,
        JSON_ARRAY,
        JSON_ARRAY_LAST,
        JSON_NUMBER,
        JSON_BOOLEAN_TRUE,
        JSON_BOOLEAN_FALSE,
        JSON_NULL

    } JSON_TYPE;

    struct JSON_MAP {
        public:

        JSON_TYPE type;
        bool objectLast = false;
        bool objectFirst = false;
        bool arrayLast = false;
        bool arrayFirst = false;
        bool boolean = false;
        bool booleanValue = false;
        char* key = NULL; 
        char* value = NULL;
        int num_Value;
        int memberCount = 0;
        JSON_MAP* members = NULL; // For any members, e.g., value is an array or object.
        JSON_MAP* next = NULL;

        ~JSON_MAP() {
            delete this->next;
        }

    };

    struct token {
        public:

        int position; // position of token in array
        char* value = NULL;
        int number = 0;
        bool number_State = false;
        TOKEN_TYPE type;
        token* next = {};

        token() {
            next = {};
            value = {};
        }
        ~token();

    };

    JSON_MAP* JsonMap = NULL;

    // Prints all available information on a given JSON_MAP struct.
    void PrintInformation(JSON_MAP* jsonmap);
    void PrintTokens(token* token_list);

    // Locates a specified character in a supplied character array starting from a specified position in the array.
    // Returns the position of the token if found, otherwise returns -1.
    int TokenPosition(int x, char c, char* contents);

    // Copies characters between two given positions; s_pos is starting position, e_pos is ending position.
    void JsonCopy(char* out_buffer, char* in_array, int s_pos, int e_pos);

    // Function that returns the amount of members an array or object has by counting the amount of commas
    // in a given token list. It is assumed that the inputted token list is a single array or object.
    int HasMembers(token* token_link, TYPE jsontype, char* contents);

    // Function which handles opening brackets key/value parsing.
    void BracketOpen(JSON_MAP*& jsonmap, token*& token_link, char* contents);

    // Function which handles opening braces key/value parsing.
    void BraceOpen(JSON_MAP*& jsonmap, token* token_link, char* contents);

    // Function which handles colons in key/value parsing.
    void Colon(JSON_MAP* jsonmap, token* token_link, char* contents);

    // Function which handles strings in key/value parsing.
    TOKEN JsonString(JSON_MAP*& jsonmap, token*& token_link, char* contents);

    // must use free() after usage; do not directly call for that reason
    char* RemoveSpace(char* array); // removes whitespace

    // Parses each JSON syntax TOKEN and places each token into a linked list of structs 
    // with extra information for further parsing 
    token* ParseToken(char* contents);

    // Parses the content contained within two associated braces.
    char* ParseObject(token* token_link, char* contents);

    // Parses the content contained within two associated brackets.
    char* ParseArray(token* token_link, char* contents);

    // Parses a string and trims quotation marks; used in parse_Key
    char* ParseString(token* token_link, char* contents);

    // Parses non-string values, i.e., numbers/floats.
    char* ParseValue(token* token_link, int delim_Pos, char* contents);

    // Parses key and value pairs and places them in a JSON_MAP struct
        // NOTE: "ParseKey" is a misleading function name as it also parses the values.
    JSON_MAP* ParseKey(token* token_list, char* contents);

    // Returns the amount of token structs in a given linked list of token structs.
    int ExtractToken(token* token_list);

    // Parses an array or object, specifically.
    const char* ParseContainer(const char* container, const char* key);

    // Frees and deletes all dynamically-allocated memory in specified struct.
    void Destroy(JSON_MAP*& jsonmap);

    JSON_MAP* Parse(const char* data);
    // Main Interface //

    public:
    
    // Constructor; takes the path to the JSON file as input.
    JSON(const char* path, bool isFile);
    JSON();
    // Deconstructor; frees all structs and members.
    ~JSON();
    // Gets and returns the value of the key passed to the function. Returns the value if successful, NULL otherwise.
    char* GetValue(const char* key);
    // Returns 1 if true, 0 if false, and -1 if key not found.
    int GetBoolValue(const char* key);

    // Writes and formats JSON_MAP struct information according to JSON specification to make a valid JSON file.
    int OutputJson(JSON_MAP*& jsonmap);

    // Interface for parsing multiple JSON files/text under one instantiation //

    // Parses JSON data.
    //JSON_MAP* Parse(const char* data);
    // Gets and returns the value of the key passed to the function; returns NULL if key is not found.
    // Uses an external JSON_MAP instead of JSON::JsonMap. 
    const char* GetValue(JSON_MAP* jsonmap, const char* key);

};

#endif