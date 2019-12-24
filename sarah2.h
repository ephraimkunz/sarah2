// See full algorithm and explanation at http://laser-calcium.glitch.me.
// C implementation by Ephraim Kunz.

#include <stddef.h>
#include <stdbool.h>

#ifndef SARAH2_H
#define SARAH2_H

extern const size_t sarah2_key_size;

typedef enum Sarah2_Rounds{
    // Supply your own number of rounds.
    Sarah2_Rounds_Custom,

    // "I'm only encrypting a handful of messages, and adversaries will never get a chance to make me encrypt
    // anything they choose."
    Sarah2_Rounds_Minimal, 

    // "I'm encrypting tons of messages, but adversaries will still probably not get a chance to make me encrypt 
    // anything they choose."
    Sarah2_Rounds_Medium,

    // "My adversary can encrypt and decrypt things using my key, but they don't know the key."
    Sarah2_Rounds_Maximal
} Sarah2_Rounds;

/**** Encryption and decryption ****/

// IMPORTANT: Make sure to seed RNG with srand() before using this library.

// Given a plaintext message length (from strlen, for instance), returns the size of the encrypted 
// message length.
size_t sarah2_output_size(size_t message_len);

// Given a key of sarah2_key_size, encrypts message of message_length into out_buf (of length returned
// returned by a call to sarah2_output_size) using rounds number of rounds. If message_len is not the same as 
// the length of out_buf, the message is '_' padded before encryption. If rounds is Sarah2_Rounds_Custom,
// custom num_rounds will be used. Otherwise, custom_num_rounds should be 0.
void sarah2_encrypt(char *key, char *message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds);

// Given a key of sarah2_key_size, decrypts ciphertext of message_length into out_buf (of length equal
// to message_length) using rounds number of rounds. If rounds is Sarah2_Rounds_Custom,
// custom num_rounds will be used. Otherwise, custom_num_rounds should be 0.
void sarah2_decrypt(char *key, char *encrypted_message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds);

/**** Key generation and display ****/

// Generate a key of sarah2_key_size into out_buf.
void sarah2_generate_key(char *out_buf);

// Print nicely formatted key of size sarah2_key_size to std out.
void sarah2_print_key(char *key);

// Validate that key of sarah2_key_size is valid, returning true if it 
// is and false otherwise.
bool sarah2_validate_key(char *key);

#endif // SARAH2_H
