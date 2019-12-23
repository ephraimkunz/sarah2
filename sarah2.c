#include "sarah2.h"

size_t sarah2_output_size(size_t message_len) {
    return 0;
}

void sarah2_encrypt(char *key, char *message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds) {

}

void sarah2_decrypt(char *key, char *encrypted_message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds) {

}

void sarah2_generate_key(char *out_buf) {

}

void sarah2_print_key(char *key) {

}

bool sarah2_validate_key(char *key) {
    return false;
}

/**** Just for development ****/
int main() {

}
