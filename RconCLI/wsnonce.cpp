// A Library for generating a 128 bit nonce for websocket connections,
// and verifying the server response

#include <RconCLI/wsnonce.h>

// Static function for indexing an array of bytes (chars) by bit rather than
// by byte. It is written to be indexed by multiples of 6 bits only,
// specifically for base64 encoding.
static char sixBitsFrom(char *array, int n) {
	int quo = n / 8;	// quotient of bit n / 8 for byte index
	int rem = n % 8;	// remainder of n / 8 for offset calc
	//printf("%d, %d\n", quo, rem);	// For debugging
	char buf = array[quo];	// Byte at index of quo

	// Offset bits in buf based on the remainder of n / 8, and add
	// bits from the left of next byte if necessary
	if (rem == 0) {
		buf >>= 2;
	} else if (rem == 6) {
		buf <<= 4;
		buf &= 0b00110000;
		buf |= (array[quo+1] >> 4) & 0b00001111;
	} else if (rem == 4) {
		buf <<= 2;
		buf &= 0b00111100;
		buf |= (array[quo+1] >> 6) & 0b00000011;
	} else if (rem == 2) {
		buf &= 0b00111111;
	} else return *"Erorr.\n";

	buf &= 0b00111111;	// Unset first two bits of buf

	/*//This section is for debugging
	printbin((long) array[quo], 8);
	printf(" ");
	printbin((long) array[quo+1], 8);
	printf("\n");
	for (int i = 0; i < rem; i++) {
		printf(" ");
	}
	printbin((long) buf, 6);
	printf("\n");
	printf("%c\n%c, %d\nEND\n\n", array[quo], lut[(int)buf], buf);
	*/

	return buf;
}
// Static function for printing the ASCII representation of a number in binary
// Used solely for debugging.
static void printbin(long number, int num_digits) {
    for(int digit = num_digits - 1; digit >= 0; digit--) {
	printf("%c", number & (1 << digit) ? '1' : '0');
    }
}

// A function for encoding data formatted as a character array into a base64
// string.
// To use this function, you MUST assign it to a *variable, and call free() on
// it after use, as this function allocates memory with malloc().
// Do NOT reference the return value of this function by directly calling it for
// that very reason.
// This function returns a pointer to the location in memory that has been
// allocated.
char *toBase64(char *input) {

	// Lookup table for base64 encoding, where the index of each char is
	// the six-bit value that converts to that char
	char *lut = (char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int inlen = strlen(input);	// Lenght of input in bytes
	// Number of bytes to pad at end of input data.
	// If remainder of inlen / 3 == 0, set to 0; otherwise,
	// set to 3 - remainder.
	int pad =  inlen % 3 == 0 ? 0 : 3 - (inlen % 3);
	int dlen = inlen+pad;	// Length of data buffer after padding
	char data[dlen+1];	// Declaration of data buffer +1 for null byte

	/*// This section is for debugging
	for (int i = 0; i < inlen; i++) {
		printbin((long)input[i], 8);
		printf("\n");
	}
	*/

	// Put input data in data buffer, and add padding if needed
	strncpy(data, input, inlen+1);
	if (pad > 0) {
		for (int i = 0; i < pad; i++) {
			strncat(data, "\0", 2);
		}
	}

	// Length for output data buffer set to the length of the input times
	// 1.333... +1
	int enc_dlen = dlen + (dlen/3) + 1;
	char *encoded_data = (char*)malloc(enc_dlen);	// Declartion of output data buffer
	char byte;	// Single-byte buffer

	// Iterate through input buffer by bit index, and consult the lookup
	// table for which ASCII character to encode the six bits of data as
	for (int i = 0; i < dlen*8; i+=6) {
		byte = sixBitsFrom(data, i);
		for (int j = 0; j < 64; j++) {
			if (byte == j) {
				//encoded_data[(i/6)] = lut[j];
				memcpy(&encoded_data[i/6], &lut[j], 1);
				break;
			}
		}
		/*// This section is for debugging
		printf("%d, %d / ", i, sixBitsFrom(data, i));
		printbin(sixBitsFrom(data, i), 8);
		printf("\n");
		*/
	}

	// If data was padded, substitute trailing null data for pad chars (=),
	// save for the last byte, reserved for the null terminator
	if (pad > 0) {
		for (int i = 0; i < pad; i++) {
			memset(&encoded_data[enc_dlen-i-2], *"=", 1);
		}
	}

	// Pad the last byte of the output buffer with a null byte to terminate
	// the string
	encoded_data[enc_dlen-1] = *"\0";

	/*//This section is for debugging
	printf("Data in: %s\n", input);
	printf("Data length: %d\n", inlen);
	printf("Bytes to pad: %d\n", pad);
	printf("New Data: %s\n", data);
	printf("Encoded data: %s\n", encoded_data);
	*/

	return encoded_data;
}

// To use this function, you MUST assign it to a *variable, and call free() on
// it after use, as this function allocates memory with malloc().
// Do NOT reference the return value of this function by directly calling it for
// that very reason.
// This function returns a pointer to the location in memory that has been
// allocated.
char *genNonce() {
	srand(time(0));
	char num;
	char bignum[16];

	for (int i = 0; i<16; i++) {
		num = rand(); // % 254 + 1;
		memcpy(&bignum[i], &num, 1);
	}
	char *input = bignum;
	// Lookup table for base64 encoding, where the index of each char is
	// the six-bit value that converts to that char
	char *lut = (char*)"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	int inlen = 16; //strlen(input);	// Lenght of input in bytes
	// Number of bytes to pad at end of input data.
	// If remainder of inlen / 3 == 0, set to 0; otherwise,
	// set to 3 - remainder.
	int pad =  inlen % 3 == 0 ? 0 : 3 - (inlen % 3);
	int dlen = inlen+pad;	// Length of data buffer after padding
	char data[dlen+1];	// Declaration of data buffer +1 for null byte

	/*// This section is for debugging
	for (int i = 0; i < inlen; i++) {
		printbin((long)input[i], 8);
		printf("\n");
	}
	*/

	// Put input data in data buffer, and add padding if needed
	memcpy(data, input, inlen);
	if (pad > 0) {
		for (int i = 0; i < pad; i++) {
			strncat(data, "\0", 2);
		}
	}

	// Length for output data buffer set to the length of the input times
	// 1.333... +1
	int enc_dlen = dlen + (dlen/3) + 1;
	char *encoded_data = (char*)malloc(enc_dlen);	// Declartion of output data buffer
	char byte;	// Single-byte buffer

	// Iterate through input buffer by bit index, and consult the lookup
	// table for which ASCII character to encode the six bits of data as
	for (int i = 0; i < dlen*8; i+=6) {
		byte = sixBitsFrom(data, i);
		for (int j = 0; j < 64; j++) {
			if (byte == j) {
				//encoded_data[(i/6)] = lut[j];
				memcpy(&encoded_data[i/6], &lut[j], 1);
				break;
			}
		}
		/*// This section is for debugging
		printf("%d, %d / ", i, sixBitsFrom(data, i));
		printbin(sixBitsFrom(data, i), 8);
		printf("\n");
		*/
	}

	// If data was padded, substitute trailing null data for pad chars (=),
	// save for the last byte, reserved for the null terminator
	if (pad > 0) {
		for (int i = 0; i < pad; i++) {
			memset(&encoded_data[enc_dlen-i-2], *"=", 1);
		}
	}

	// Pad the last byte of the output buffer with a null byte to terminate
	// the string
	encoded_data[enc_dlen-1] = *"\0";

	/*//This section is for debugging
	printf("Data in: %s\n", input);
	printf("Data length: %d\n", inlen);
	printf("Bytes to pad: %d\n", pad);
	printf("New Data: %s\n", data);
	printf("Encoded data: %s\n", encoded_data);
	*/

	return encoded_data;
}

// This function returns 0 if the supplied nonce corresponds to the hash
// supplied by the websocket response. It returns 1 otherwise.
int verifyResponseHash(char *nonce, char *response) {

	// UUID used by the websocket protocol to hash the nonce supplied by
	// the client by concatenating it to the end of the base64 string
	// representation of the nonce supplied by the client.
	const char *wsuuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

	// Do the same concatenation and hashing as the server to compare with
	// the server response.
	char concat[strlen(nonce)+strlen(wsuuid)+1];
	memset(concat, 0, strlen(concat));
	strncat(concat, nonce, strlen(nonce)+1);
	strncat(concat, wsuuid, strlen(wsuuid)+1);

	char hash[20];
	SHA1((const unsigned char*)concat, strlen(concat), (unsigned char*)hash);

	// Convert the locally computed hash to Base 64 to match the response
	char hashString[21];
	memcpy(&hashString, &hash, 20);
	memset(&hashString[20], '\0', 1);

	char *hash_b64 = toBase64(hashString);

	// For debugging purposes
	//printf("\n%s\n%s\n%s\n", response, hash_b64, concat);

	// Compare every byte in the locally generated hash to those of the
	// server-generated hash. If any discrepancy, return 1 immediately.
	for(int i = 0; i < strlen(hash_b64); i++) {
		if(hash_b64[i] != response[i]) {
			free(nonce);
			return 1;
		}
	}
	free(nonce);
	free(hash_b64);
	return 0;
	
}

