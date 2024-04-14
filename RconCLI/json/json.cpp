#include <RconCLI/json/json.h>

// Removes all whitespace from a character array; keeps spaces inside quotes.
char* JSON::RemoveSpace(char* array) {

    int c = 0;
    bool q_switch = false;
    char* cpy = (char*)malloc(sizeof(char*) * (strlen(array) + 1));
    memset(cpy, 0, strlen(array) + 1);

    for (int i = 0; array[i]; i++) {

        if (array[i] == '"') {
            if (q_switch == true) {
                q_switch = false;
            } else { q_switch = true; }
        }
        if (array[i] == ' ' && q_switch == true) {

            cpy[c++] = array[i];

        }
        if (array[i] == '\n' ) {

            i++;
            
        }
        if (array[i] != ' ') {

            cpy[c++] = array[i];

        }

    }
    cpy[c] = '\0';
    return cpy;

}

// Locates a specified character in a supplied character array starting from a specified position in the array.
// Returns the position of the token if found, otherwise returns -1.
int JSON::TokenPosition(int x, char c, char* contents) {

    x++;
    while (contents[x] != c) {

        x++;
        if (contents[x] == 0) return -1;
        if (contents[x] == c) return x;

    }

    if (contents[x] != c) return -1;
    return x;
}

// Parses JSON tokens from a given character array and places them into a linked list of structs with other related information.
JSON::token* JSON::ParseToken(char* contents) {
    
    token* fToken = new token;
    token* Token = fToken;

    char* c = contents;
    int i = 0;
    int str_i = 0;
    int count = 0;

    for (*c; *c != 0; *c++) {    

        switch (*c) {

            case 0:

                *c = 0;
                return 0; // error: unexpected null termination

            case '{':

                Token->type = JSON::TOKEN_TYPE::BRACE_OPEN;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case '"':
                
                Token->type = JSON::TOKEN_TYPE::STRING;
                Token->position = i;
                str_i = JSON::TokenPosition(i, '"', contents);
                Token->next = new token;
                Token = Token->next;

                // Next token which should be a string token
                Token->type = JSON::TOKEN_TYPE::STRING;
                Token->position = str_i;
                Token->next = new token;
                Token = Token->next;

                // move *c to 1 character beyond quotation marks indicating string

                while (count != str_i) {
                    
                    if (*c == '\0') return 0; // error: unexpected null termination
                    if (*c == '}') break;
                    count++;
                    *c++; 

                }

                i = str_i;
                i++;

                break;

            case ':':

                Token->type = JSON::TOKEN_TYPE::COLON;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case ',':

                Token->type = JSON::TOKEN_TYPE::COMMA;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case '[':

                Token->type = JSON::TOKEN_TYPE::BRACKET_OPEN;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case '}':

                Token->type = JSON::TOKEN_TYPE::BRACE_CLOSED;
                Token->position = i;
                if ((*c + 1) == 0) return fToken;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case ']':

                Token->type = JSON::TOKEN_TYPE::BRACKET_CLOSED;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case '1':
             
                Token->type = JSON::TOKEN_TYPE::NUMBER;
                Token->position = i;
                Token->next = new token;
                Token = Token->next;
                i++;

                break;

            case 't':

                if ((*c + 1) == 'r' && (*c + 2) == 'u' && (*c + 3) == 'e') {

                    Token->type = JSON::TOKEN_TYPE::BOOLEAN_TRUE;
                    Token->position = i;
                    Token->next = new token;
                    Token = Token->next;
                    i = i + 3;
                    break;

                } else { 
                    i++;
                    break;
                }

            case 'f':

                if ((*c + 1) == 'a' && (*c + 2) == 'l' && (*c + 3) == 's' && (*c + 4) == 'e') {

                    Token->type = JSON::TOKEN_TYPE::BOOLEAN_FALSE;
                    Token->position = i;
                    Token->next = new token;
                    Token = Token->next;
                    i = i + 4;
                    break;

                } else { 
                    i++;
                    break;
                }

            default:

                i++;
                break;

        }

        count++;

    }   

    return fToken;

}

// Copies characters between two given positions; s_pos is starting position, e_pos is ending position.
void JSON::JsonCopy(char* out_buffer, char* in_array, int s_pos, int e_pos) {

    size_t length = e_pos - s_pos + 1;
    for (int i = s_pos, x = 0; i != e_pos, x < length; i++, x++) {
        memcpy(&out_buffer[x], &in_array[i], 1);
    }
    out_buffer[length] = '\0';
    
}

// This function is used to specifically parse the content of a JSON array; this function works 
// nearly the same way as ParseString(), except that it keeps track of opening brackets before 
// a closing bracket is found, in order to not incorrectly parse an array when an array contains
// another array: the full array will be parsed and return a character array of its contents,
// and will be parsed again to individualize its contents.
// NOTE: in reality, ParseString() could be reworked to be used for this case as well, but it
// would require extra work which I don't feel like doing.
char* JSON::ParseArray(JSON::token* token_link, char* contents) {
    
    char* array_Out;
    int length;
    int t = 0;
    int t0 = 0;
    JSON::token* next_Token = token_link->next;
    while (t0 <= t) {

        if (next_Token->type == JSON::TOKEN_TYPE::BRACKET_OPEN) t++;
        next_Token = next_Token->next;
        if (next_Token->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) {
            t0++;
        }
        if (t0 > t) {
            break;
        }

    }

    // Get length of array and allocate memory and copy characters into buffer (array_Out).
    length = next_Token->position - token_link->position + 1;
    array_Out = (char*)malloc(sizeof(char*) * length);
    memset(array_Out, 0, sizeof(char*) * length);
    JsonCopy(array_Out, contents, (token_link->position), (next_Token->position));
    
    return array_Out;

}

// Parses the content contained within two associated braces.
char* JSON::ParseObject(JSON::token* token_link, char* contents) {

    char* object_Out;
    int length;
    int offset = 0;
    int position = 0;
    JSON::token* next_Token = token_link->next;
    while (offset <= position) {

        if (next_Token->type == JSON::TOKEN_TYPE::BRACE_OPEN) position++;
        next_Token = next_Token->next;
        if (next_Token->type == JSON::TOKEN_TYPE::BRACE_CLOSED) offset++;
        if (offset > position) break;

    }

    length = next_Token->position - token_link->position + 1;
    object_Out = (char*)malloc(sizeof(char*) * length);
    memset(object_Out, 0, sizeof(char*) * length);
    JsonCopy(object_Out, contents, (token_link->position), (next_Token->position));
    return object_Out;

}

// This function is to specifically parse the content of a JSON string; particularly, the function
// trims quotation marks, therefore it won't work properly for general use.
// NOTE: this can be re-designed in order to allow more versatility with a single function
char* JSON::ParseString(JSON::token* token_link, char* contents) {

    char* string_Out;
    int length;
    JSON::token* next_Token = token_link->next;
    if (next_Token->position > token_link->position) length = (next_Token->position - 1) - (token_link->position + 1) + 1;
    string_Out = (char*)malloc(sizeof(char*) * length);
    memset(string_Out, 0, sizeof(char*) * length);
    JsonCopy(string_Out, contents, (token_link->position + 1), (next_Token->position - 1));

    return string_Out;
}

// For parsing non-string values, i.e., numbers.
char* JSON::ParseValue(JSON::token* token_link, int delim_Pos, char* contents) {

    char* value;
    int length;
    if (delim_Pos > token_link->position) {

        length = delim_Pos - (token_link->position) + 1;

    } else { return NULL; }
    value = (char*)malloc(sizeof(char*) * length);
    memset(value, 0, length);
    JsonCopy(value, contents, (token_link->position), delim_Pos);

    return value;
}

// Returns the amount of token structs in a given linked list of token structs.
int JSON::ExtractToken(JSON::token* token_list) {

    int count = 0;
    JSON::token* tl = token_list;
    while (tl->next != NULL) {

        count++;
        tl = tl->next;
        if (tl->next == NULL) break;

    }

    return count;
}

// Determines whether a JSON object/array has members; returns a positive, non-zero number if the object/array has members
// (the number of members); otherwise, the return value is -1 for error.
int JSON::HasMembers(JSON::token* token_link, JSON::TYPE jsontype, char* contents) {

    int memberCount = 0;

    // Validate jsontype is either JSON_ARRAY or JSON_OBJECT
    switch (jsontype) {

        case JSON::TYPE::JSON_ARRAY:
            break;
        case JSON::TYPE::JSON_OBJECT:
            break;
        default:
            printf("\nJSON::HasMembers(JSON::token*, JSON::JSON_TYPE): invalid JSON_TYPE input.");
            return -1; // temporary error

    }

    // Check if array/object is empty by checking the next token.
    switch (jsontype) {

        case JSON::JSON_TYPE::JSON_ARRAY:
            if (token_link->next->type == JSON::TOKEN_TYPE::BRACKET_CLOSED)
                return memberCount;
            break;
        case JSON::JSON_TYPE::JSON_OBJECT:
            if (token_link->next->type == JSON::TOKEN_TYPE::BRACE_CLOSED)
                return memberCount;
            break;
        default: 
            return -1; // temporary error

    }

    // If all above checks succeed, set memberCount to 1 because it is assumed that the amount of object/array members is 
    // at least 1.
    memberCount = 1;

    int open = 0;
    int closed = 0;

    // Iterate through the token list and count the commas; the amount of commas indicates the amount of members.
    while (token_link->next != NULL) {
        
        // Skip the first token, as it can mess up parsing; the first token is either a bracket or a brace.
        token_link = token_link->next;
        //printf("\nW_TOKEN: %s", token_link->value);
        
        // Skip sub-object/arrays
        if (token_link->next != NULL) {

            if (token_link->type == JSON::TOKEN_TYPE::BRACKET_OPEN) {
            
                open++;
                while (open != closed) {
                    if (token_link->next->type == JSON::TOKEN_TYPE::BRACKET_OPEN) open++;
                    if (token_link->next->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) closed++;
                    token_link = token_link->next;
                }
                token_link = token_link->next;

            } else if (token_link->type == JSON::TOKEN_TYPE::BRACE_OPEN) {

                open++;
                while (open != closed) {
                    //printf("\n__TOKEN: %s", token_link->value);
                    if (token_link->next->type == JSON::TOKEN_TYPE::BRACE_OPEN) open++;
                    if (token_link->next->type == JSON::TOKEN_TYPE::BRACE_CLOSED) closed++;
                    token_link = token_link->next;
                }
                token_link = token_link->next;

            }

        }
        open = 0;
        closed = 0;

        if (!token_link->next) return memberCount;
        if (token_link->type == JSON::TOKEN_TYPE::COMMA) memberCount++;
        if (token_link->next->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) return memberCount;
        if (token_link->next->type == JSON::TOKEN_TYPE::BRACE_CLOSED) return memberCount;
        continue;

    }

    return memberCount;

}

// Function for processing case TOKEN_TYPE::BRACKET_OPEN in parse_Key().
void JSON::BracketOpen(JSON::JSON_MAP*& jsonmap, JSON::token*& token_link, char* contents) {

    if (jsonmap == NULL || token_link == NULL) return;
    
    // Parse array then set type to JSON_ARRAY.
    char* arrayString = JSON::ParseArray(token_link, contents);
    jsonmap->type = JSON::JSON_TYPE::JSON_ARRAY;

    // Tokenize array string and retrieve the count.
    JSON::token* arrayList = JSON::ParseToken(arrayString);
    int tokenCount = JSON::ExtractToken(arrayList);

    // Get number of members; if none, zero is returned, if members, the return value is non-zero. -1 on error.
    jsonmap->memberCount = JSON::HasMembers(arrayList, JSON::TYPE::JSON_ARRAY, arrayString);

    // Index the array token list forward so the program doesn't get stuck in a loop when parse_Key() is called.
    arrayList = arrayList->next;

    // Parse the keys and values of the array then set jsonmap->members to the parsed keys/values
    // (the keys/values are members of the array that is the value of jsonmap->key)
    JSON::JSON_MAP* arrayMap = ParseKey(*&arrayList, arrayString);

    // Free arrayString because it returns a char* that is heap-allocated by JsonCopy()
    free(arrayString);
    //delete arrayList;

    if (arrayMap) {
        jsonmap->members = arrayMap;
        jsonmap->members->arrayFirst = true;
    }

    // Move token_link forward by the corresponding count returned by ExtractToken().
    jsonmap->next = new JSON_MAP;
    jsonmap = jsonmap->next;

}

// Function for processing case TOKEN_TYPE::BRACE_OPEN in parse_Key().
void JSON::BraceOpen(JSON::JSON_MAP*& jsonmap, JSON::token* token_link, char* contents) {

    if (jsonmap == NULL || token_link == NULL) return;

    // Parse object, set value to full parsed object char pointer, then set type to JSON_OBJECT.
    char* objectString = JSON::ParseObject(token_link, contents);
    //jsonmap->value = objectString;
    jsonmap->type = JSON::JSON_TYPE::JSON_OBJECT;

    // Tokenize array string and retrieve the count.
    JSON::token* objectList = JSON::ParseToken(objectString);
    int tokenCount = JSON::ExtractToken(objectList);

    // Get number of members; if none, zero is returned, if members, the return value is non-zero. -1 on error.
    jsonmap->memberCount = JSON::HasMembers(objectList, JSON::TYPE::JSON_OBJECT, objectString);

    // Index the array token list forward so the program doesn't get stuck in a loop when parse_Key() is called.
    objectList = objectList->next;

    // Parse the keys and values of the array then set jsonmap->members to the parsed keys/values
    // (the keys/values are members of the array that is the value of jsonmap->key)
    JSON::JSON_MAP* objectMap = ParseKey(*&objectList, objectString);

    // Free objectList and objectString as both are no longer needed.
    free(objectString);
    //delete objectList;

    if (objectMap) {
        jsonmap->members = objectMap;
        jsonmap->members->objectFirst = true;
    }

    // Move forward jsonmap.
    jsonmap->next = new JSON_MAP;
    jsonmap = jsonmap->next;

}

// Function for processing case TOKEN_TYPE::COLON in parse_Key().
void JSON::Colon(JSON_MAP* jsonmap, token* token_link, char* contents) {

    JSON::token* nextToken = token_link->next;
    if (nextToken->type == JSON::TOKEN_TYPE::NUMBER) {

        // Get position of the comma in contents from the position of nextToken.
        int commaPos = JSON::TokenPosition(nextToken->position, ',', contents);
        char* numberChar = JSON::ParseValue(nextToken, (commaPos - 1), contents); // -1 so the comma is not included in output
        if (numberChar == NULL) return;
        jsonmap->value = numberChar;

        jsonmap->next = new JSON_MAP;
        jsonmap = jsonmap->next;

        free(numberChar);

    } else if (nextToken->type == JSON::TOKEN_TYPE::BOOLEAN_TRUE) {

        jsonmap->value = NULL;
        jsonmap->boolean = true;
        jsonmap->booleanValue = true;
        jsonmap->next = new JSON_MAP;
        jsonmap = jsonmap->next;

    } else if (nextToken->type == JSON::TOKEN_TYPE::BOOLEAN_FALSE) {

        jsonmap->value = NULL;
        jsonmap->boolean = true;
        jsonmap->booleanValue = false;
        jsonmap->next = new JSON_MAP;
        jsonmap = jsonmap->next;

    } else {

        return;

    }

}

// Function for processing case JSON::TOKEN_TYPE::STRING in parse_Key(). The reason this function returns a JSON::TOKEN
// is because there are instances where the switch-case needs to be broken or continued depending on the result of some 
// conditional statements; I used a pre-existing system (enum JSON::TOKEN) to accomplish this.
JSON::TOKEN JSON::JsonString(JSON_MAP*& jsonmap, JSON::token*& token_link, char* contents) {

    char* data;
    JSON::token* nextToken = token_link->next;
    if (nextToken->type == JSON::TOKEN_TYPE::STRING) {

        data = JSON::ParseString(token_link, contents);
        jsonmap->type = JSON::JSON_TYPE::JSON_STRING;
        token_link = token_link->next;
        token_link = token_link->next;

    } else { return JSON::TOKEN_TYPE::STRING; }

    if (token_link->type == JSON::TOKEN_TYPE::COLON) {

        jsonmap->key = data;
        return JSON::TOKEN_TYPE::COLON;

    } else if (token_link->type == JSON::TOKEN_TYPE::COMMA || token_link->type == JSON::TOKEN_TYPE::BRACE_CLOSED || token_link->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) {

        if (token_link->type != JSON::TOKEN_TYPE::COMMA) {

            //printf("\nCOLLECTION_LAST: %s", jsonmap->value);
            if (token_link->type == JSON::TOKEN_TYPE::BRACE_CLOSED) jsonmap->objectLast = true;
            if (token_link->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) jsonmap->arrayLast = true;

        }
        jsonmap->value = data;
        jsonmap->next = new JSON_MAP;
        jsonmap = jsonmap->next;
        return JSON::TOKEN_TYPE::OTHER;

    } else { return JSON::TOKEN_TYPE::OTHER; }

    return JSON::TOKEN_TYPE::NULL_TYPE;

}

void JSON::PrintInformation(JSON::JSON_MAP* jsonmap) {

    if (jsonmap != NULL) {

        printf("\n");
        if (jsonmap->key) printf("\nKEY: %s", jsonmap->key);
        if (jsonmap->value) printf("\nVALUE: %s", jsonmap->value);

        switch (jsonmap->type) {

            case JSON_STRING:
                printf("\nTYPE: JSON_STRING");
                break;
            case JSON_OBJECT:
                printf("\nTYPE: JSON_OBJECT");
                break;
            case JSON_OBJECT_LAST:
                printf("\nTYPE: JSON_OBJECT_LAST");
                break;
            case JSON_ARRAY:
                printf("\nTYPE: JSON_ARRAY");
                break;
            case JSON_ARRAY_LAST:  
                printf("\nTYPE: JSON_ARRAY_LAST");
                break;
            case JSON_NUMBER:   
                printf("\nTYPE: JSON_NUMBER");
                break;
            case JSON_BOOLEAN_TRUE: 
                printf("\nTYPE: JSON_BOOLEAN_TRUE");
                break;
            case JSON_BOOLEAN_FALSE: 
                printf("\nTYPE: JSON_BOOLEAN_FALSE");
                break;
            case JSON_NULL:
                printf("\nTYPE: JSON_NULL");
                break;
            default:
                printf("\nTYPE: ERROR_NO_TYPE");
                break;

        }

        if (jsonmap->objectFirst == true) printf("\nobjectFirst: TRUE");
        if (jsonmap->objectLast == true) printf("\nobjectLast: TRUE");
        if (jsonmap->arrayFirst == true) printf("\narrayFirst: TRUE");
        if (jsonmap->arrayLast == true) printf("\narrayLast: TRUE");
        if (jsonmap->memberCount) printf("\nmemberCount: %d", jsonmap->memberCount);
        if (jsonmap->members) {
            printf("\nmembers: VALID, %d", jsonmap->memberCount);
        } else { printf("\nmembers: NONE"); }
        if (jsonmap->next) {
            printf("\nnext: VALID");
        } else { printf("\nnext: NULL"); }
        printf("\n");

    }

}

// Parses keys and values from a linked list of token structs and places them into a linked list of JSON_MAP structs; the JSON_MAP linked list
// includes successive information on each key/value and is used to edit and read specific information from the JSON file.
JSON::JSON_MAP* JSON::ParseKey(JSON::token* token_list, char* contents) {

    if (!token_list) return NULL;

    JSON_MAP* json_map = new JSON_MAP;
    JSON_MAP* it_json = json_map;
    JSON::TOKEN tokenType;

    while (token_list->next != NULL) {

        switch (token_list->type) {

            case JSON::TOKEN_TYPE::STRING:

                tokenType = JSON::JsonString(it_json, token_list, contents);
                if (tokenType == JSON::TOKEN_TYPE::STRING) {

                    token_list = token_list->next;
                    continue;

                } else if (tokenType == JSON::TOKEN_TYPE::COLON) {

                    break;

                } else if (tokenType == JSON::TOKEN_TYPE::OTHER) {

                    continue;

                } else { continue; }
                token_list = token_list->next;

            case JSON::TOKEN_TYPE::COLON:

                JSON::Colon(it_json, token_list, contents);
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BRACKET_OPEN:
                
                if (token_list->next->type == JSON::TOKEN_TYPE::BRACKET_CLOSED) {

                    token_list = token_list->next;
                    continue;

                }
                JSON::BracketOpen(it_json, token_list, contents);
                it_json->arrayFirst = true;
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BRACE_OPEN:

                JSON::BraceOpen(it_json, token_list, contents);
                it_json->objectFirst = true;
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BRACE_CLOSED:

                it_json->type = JSON::JSON_TYPE::JSON_OBJECT_LAST;
                it_json->next = new JSON::JSON_MAP;
                it_json = it_json->next;
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BRACKET_CLOSED:

                it_json->type = JSON::JSON_TYPE::JSON_ARRAY_LAST;
                it_json->next = new JSON::JSON_MAP;
                it_json = it_json->next;
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BOOLEAN_TRUE:

                it_json->type = JSON::JSON_TYPE::JSON_BOOLEAN_TRUE;
                it_json->boolean = true;
                it_json->next = new JSON::JSON_MAP;
                it_json = it_json->next;
                token_list = token_list->next;
                continue;

            case JSON::TOKEN_TYPE::BOOLEAN_FALSE:

                it_json->type = JSON::JSON_TYPE::JSON_BOOLEAN_FALSE;
                it_json->boolean = true;
                it_json->next = new JSON::JSON_MAP;
                it_json = it_json->next;
                token_list = token_list->next;
                continue;
             
            default:
                if (token_list->next != NULL) token_list = token_list->next;

        }

    }

    return json_map;
}

// Writes and formats JSON_MAP struct information according to JSON specification to make a valid JSON file.
int JSON::OutputJson(JSON::JSON_MAP*& jsonmap) {

    FILE* jsonFile = fopen("vote.json", "w");
    if (!jsonFile) {

        printf("\nJSON::OutputJson(JSON_MAP*): fopen(): Failed to open/create file.");
        return 1;

    }

    while (jsonmap->next != NULL) {

        if (jsonmap->type == JSON::JSON_TYPE::JSON_OBJECT) fprintf(jsonFile, "{\n");
        if (jsonmap->type == JSON::JSON_TYPE::JSON_OBJECT_LAST && jsonmap->next->type == JSON::JSON_TYPE::JSON_OBJECT) {

            fprintf(jsonFile, "},\n");

        } else if (jsonmap->type == JSON::JSON_TYPE::JSON_OBJECT_LAST && jsonmap->next->type != JSON::JSON_TYPE::JSON_OBJECT) { 

            fprintf(jsonFile, "}\n"); 

        }

        if (jsonmap->type == JSON::JSON_TYPE::JSON_ARRAY) fprintf(jsonFile, "\"%s\" : [\n", jsonmap->key);
        if (jsonmap->key && jsonmap->value) {
            
            if ((jsonmap->objectFirst == true && jsonmap->objectLast == true) || (jsonmap->arrayFirst == true && jsonmap->arrayLast == true)) {

                fprintf(jsonFile, "\t\"%s\" : \"%s\"\n", jsonmap->key, jsonmap->value);

            } else if ((jsonmap->objectFirst == true || jsonmap->arrayFirst == true) && (jsonmap->objectLast != true || jsonmap->arrayLast != true)) {

                fprintf(jsonFile, "\t\"%s\" : \"%s\", \n", jsonmap->key, jsonmap->value);

            } else if (jsonmap->objectLast == true || jsonmap->arrayLast == true) { 

                fprintf(jsonFile, "\t\"%s\" : \"%s\"\n", jsonmap->key, jsonmap->value); 

            } else { fprintf(jsonFile, "\t\"%s\" : \"%s\", \n", jsonmap->key, jsonmap->value); }

        } else if (!jsonmap->key && jsonmap->value) {

            if ((jsonmap->objectFirst == true || jsonmap->arrayFirst == true) && (jsonmap->objectLast != true || jsonmap->arrayLast != true)) {

                fprintf(jsonFile, "\t\"%s\", \n", jsonmap->value);

            } 
            if (jsonmap->objectLast == true || jsonmap->arrayLast == true) { 

                fprintf(jsonFile, "\t\"%s\"\n", jsonmap->value); 

            }

        }

        if (jsonmap->type == JSON::JSON_TYPE::JSON_ARRAY_LAST &&
            (jsonmap->next->type == JSON::JSON_TYPE::JSON_OBJECT || 
            jsonmap->next->type == JSON::JSON_TYPE::JSON_ARRAY || 
            jsonmap->next->type == JSON::JSON_TYPE::JSON_STRING)) {

               fprintf(jsonFile, "\n], \n");

        } else if (jsonmap->type == JSON::JSON_TYPE::JSON_ARRAY_LAST) { 

            fprintf(jsonFile, "\n]\n"); 

        }
        
        jsonmap = jsonmap->next;
        
    }

    return 0;

}

void JSON::Destroy(JSON::JSON_MAP*& jsonmap) {}

// Interface public function: gets the value of a specific key. Returns the key's value if found, otherwise NULL.
char* JSON::GetValue(const char* key) {

    char* value;
    if (JSON::JsonMap->next) {

        while (JSON::JsonMap->next != NULL) {

            if (JSON::JsonMap->key) {

                if (!strncmp(JSON::JsonMap->key, key, strlen(key))) {
                    
                    if (JSON::JsonMap->value) {

                        value = JSON::JsonMap->value;
                        return value;

                    } else { JSON::JsonMap = JSON::JsonMap->next; }

                } else { JSON::JsonMap = JSON::JsonMap->next; }

            } else { JSON::JsonMap = JSON::JsonMap->next; }

            if (JSON::JsonMap->next == NULL) return NULL;

        }

    }

    return NULL;

}

// Returns 1 if true, 0 if false, and -1 if key not found.
int JSON::GetBoolValue(const char* key) {

    char* value;
    if (JSON::JsonMap->next) {

        while (JSON::JsonMap->next != NULL) {

            if (JSON::JsonMap->key) {

                if (!strncmp(JSON::JsonMap->key, key, strlen(key))) {
                    
                    if (JSON::JsonMap->boolean == true) {

                        // Value is of boolean type, so return the corresponding boolean value.
                        if (JSON::JsonMap->booleanValue == true) {
                            return 1;
                        } else { return 0; }

                    } else { JSON::JsonMap = JSON::JsonMap->next; }

                } else { JSON::JsonMap = JSON::JsonMap->next; }

            } else { JSON::JsonMap = JSON::JsonMap->next; }

            if (JSON::JsonMap->next == NULL) return -1;

        }

    }

    return -1;

}

const char* JSON::GetValue(JSON::JSON_MAP* jsonmap, const char* key) {

    char* g_value;
    const char* value;
    if (jsonmap->next) {

        while (jsonmap->next != NULL) {

            if (jsonmap->key) {

                if (!strncmp(jsonmap->key, key, strlen(key))) {
                    
                    if (jsonmap->value) {

                        g_value = jsonmap->value;
                        value = g_value;
                        return value;

                    } else { jsonmap = jsonmap->next; }

                } else { jsonmap = jsonmap->next; }

            } else { jsonmap = jsonmap->next; }

            if (jsonmap->next == NULL) return NULL;

        }

    }

    return NULL;

}

// Constructor overload 1; opens the JSON file, reads it, trims it of whitespace, then tokenizes and parses the trimmed JSON string.
// If isFile is 1, then path is treated as a file path, otherwise, it is taken as direct character input.
JSON::JSON(const char* path, bool isFile) {

    // Open the JSON file, read it into a string, then convert the string to const char*.
    if (isFile) {

        try {

            std::ifstream file(path);
            std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            const char* jsonString = contents.c_str();
            
            // Remove white space, tokenize, and parse.
            char* jsonTrim = JSON::RemoveSpace((char*)jsonString);
            JSON::token* jsonTokens = ParseToken(jsonTrim);
            JSON::JsonMap = ParseKey(*&jsonTokens, jsonTrim); // JsonMap is a public JSON class member.

            // Non-whitespace JSON char* is no longer needed.
            free(jsonTrim);

        } catch (std::ifstream::failure e) {

            std::cerr << "\n[JSON] " << e.what();
            throw;

        }

    } else {

        // Remove white space, tokenize, and parse.
        char* jsonTrim = JSON::RemoveSpace((char*)path);
        JSON::token* jsonTokens = ParseToken(jsonTrim);
        JSON::JsonMap = ParseKey(*&jsonTokens, jsonTrim); // JsonMap is a public JSON class member.

        // Non-whitespace JSON char* is no longer needed.
        free(jsonTrim);

    }

}

// Default constructor; does nothing.
JSON::JSON() {}

// Parses a JSON file/character array; to be used if no JSON file is associated with the current object.
JSON::JSON_MAP* JSON::Parse(const char* data) {

    // NULL check.
    if (!data) return NULL;

    JSON::JSON_MAP* jsonmap = NULL;
    // This will somewhat-unreliably determine if the data argument is a path or not.
    if (strpbrk(data, "/") || strpbrk(data, ".")) {

        std::ifstream file(data);
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        const char* jsonString = contents.c_str();
        
        // Remove white space, tokenize, and parse.
        char* jsonTrim = JSON::RemoveSpace((char*)jsonString);
        JSON::token* jsonTokens = ParseToken(jsonTrim);
        jsonmap = ParseKey(*&jsonTokens, jsonTrim); 

        // Non-whitespace JSON char* is no longer needed.
        free(jsonTrim);

    } else {

        // Remove white space, tokenize, and parse.
        char* jsonTrim = JSON::RemoveSpace((char*)data);
        JSON::token* jsonTokens = ParseToken(jsonTrim);
        jsonmap = ParseKey(*&jsonTokens, jsonTrim);

        // Non-whitespace JSON char* is no longer needed.
        free(jsonTrim);

    }

    return jsonmap;

}

const char* JSON::ParseContainer(const char* container, const char* key) {

    JSON::JSON_MAP* jsonmap = JSON::Parse(container);
    const char* value = JSON::GetValue(jsonmap, key);
    if (value) {
        return value;
    } else { return NULL; }

    return NULL;

}

JSON::token::~token() {

    token* tmp = NULL;
    while (this->next != NULL) {
        tmp = this->next;
        delete this->next;
        tmp = tmp->next;
    }
}

// Deconstructor; frees linked lists of structs and data that may have been heap-allocated within.
JSON::~JSON() {

    //delete JsonMap;

}