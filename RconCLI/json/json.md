# C++ JSON Interface

## Basic Functionality

The primary motivation for writing a JSON parser in C++ was to simply read a JSON config file for a websocket-based program that I am currently writing, with an expansive possibility to also write to a JSON file. The parser is in a very basic functioning state with only one method: `GetValue(const char* key)`; and, even then, this method is extremely limited in functionality, as it will only get the value of the first found `key`, so long as the key has an associated value, otherwise it continues searching.

## How It Works

The parser works in two parts: tokenization of the raw JSON string, then interpretation of the JSON tokens in sequential order.

The tokenization process utilizes its own struct to organize the tokens into a linked list; the struct includes information that is important to actual parsing and interpretation.

The process of interpretation works very similarly to the process of tokenization; it uses its own struct to organize keys and values, along with other information that is important in re-formatting the JSON file to output a valid JSON file.

Both processes are outlined in detail in the following sub-sections.

### Tokenization

Before the tokenization process takes place, the JSON file is opened and read character by character into a character array, then trimmed of any whitespace using the internal function `JSON::RemoveSpace(char*)`. Immediately following this, the internal function `JSON::ParseToken(char*)` is used to parse the JSON tokens.

`JSON::ParseToken(char*)` fills multiple instances of a struct and returns the head of that same struct; this struct is named `JSON::Token`.

```
struct Token {
	
	int position;
	TOKEN_TYPE type;
	Token* next - NULL;
	
};
```

`int position` holds the literal index position of the specific token in the raw JSON character array.

`TOKEN_TYPE type` holds a `TOKEN_TYPE` enum value; this enum is defined as follows:

```
typedef enum TOKEN {

	BRACE_OPEN,
	BRACE_CLOSED,
	STRING,
	NUMBER,
	FLOAT,
	BRACKET_OPEN,
	BRACKET_CLOSED,
	BOOLEAN,
	COLON,
	COMMA,
	NULL_TYPE,
	OTHER
	
} TOKEN_TYPE;
```

These values are used to indicate the type of JSON token that a `Token` struct instance represents.

`Token* next` is a pointer to the next `Token` struct; this is the only link between structs, and it can only be traversed forward and linearly. This makes it a singly-linked list.

#### JSON Token Parsing

The raw JSON character array is parsed with one function: `JSON::ParseToken(char*)`, which receives the raw JSON character array as input to iterate through. With each iteration, a character at an incrementing index position of the character array passed to the function is compared against valid JSON tokens with a switch statement. When a valid JSON token is found, a `Token` is filled with data that corresponds to its position and type. `JSON::ParseToken(char*)` returns a `Token*`.

Some tokens require special processing, such as quotation marks ("). When a quotation mark is found, the `position` member is filled with the token's position in the character array and the `type` member is set to `JSON::TOKEN_TYPE::STRING`. After this, the next token is searched for and, until a quotation mark is found, every token is ignored; if no quotation marks are found, the function fails. The character array pointer is then moved forward by the amount of characters in-between the two quotation marks. The characters between the quotation marks is parsed later with the two `Token` structs (which are linked together with the first `Token` instance's `next`) and the `position` member of both `Token` instances.

### Interpretation Phase