#include "sarah2.h"

#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

// Indexes into array representing S-Box (key), given the i, j position into 
// a 27 by 27 matrix holding pairs of characters at each position.
#define PAIR_FIRST_INDEX(i, j) (((i) * 2) + ((j) * num_symbols * 2))
#define PAIR_SECOND_INDEX(i, j) ((PAIR_FIRST_INDEX(i, j)) + 1)

static const size_t num_symbols = 27; // a-z + _
const size_t sarah2_key_size = num_symbols * num_symbols * 2; // num_symbols in 2D array, with two chars at each position.

const char *symbols = "_abcdefghijklmnopqrstuvwxyz";

static int randint(int n);

size_t sarah2_output_size(size_t message_len) {
    // Odd length messages will be padded by _ to make them even length.
    if ((message_len % 2) != 0) {
        return message_len + 1;
    }

    return message_len;
}

void sarah2_encrypt(char *key, char *message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds) {

}

void sarah2_decrypt(char *key, char *encrypted_message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds) {

}

void sarah2_generate_key(char *out_buf) {
    // Fill matrix with pairs.
    for (size_t i = 0; i < num_symbols; ++i) {
        for (size_t j = 0; j < num_symbols; ++j) {
            out_buf[PAIR_FIRST_INDEX(i, j)] = symbols[i];
            out_buf[PAIR_SECOND_INDEX(i, j)] = symbols[j];
        }
    }

    // Randomize pair positions.
    for (size_t i = 0; i < num_symbols; ++i) {
        for (size_t j = 0; j < num_symbols; ++j) {
            int swap_pos_x = randint(num_symbols);
            int swap_pos_y = randint(num_symbols);
            
            // Swap the pair at the i/j'th position with the pair at the swap_pos_x/y position.
            char temp = out_buf[PAIR_FIRST_INDEX(i, j)];
            out_buf[PAIR_FIRST_INDEX(i, j)] = out_buf[PAIR_FIRST_INDEX(swap_pos_x, swap_pos_y)];
            out_buf[PAIR_FIRST_INDEX(swap_pos_x, swap_pos_y)] = temp;

            temp = out_buf[PAIR_SECOND_INDEX(i, j)];
            out_buf[PAIR_SECOND_INDEX(i, j)] = out_buf[PAIR_SECOND_INDEX(swap_pos_x, swap_pos_y)];
            out_buf[PAIR_SECOND_INDEX(swap_pos_x, swap_pos_y)] = temp;
        }
    }
}

void sarah2_print_key(char *key) {
    for (size_t i = 0; i < num_symbols; ++i) {
        for (size_t j = 0; j < num_symbols; ++j) {
            char first = key[PAIR_FIRST_INDEX(i, j)];
            char second = key[PAIR_SECOND_INDEX(i, j)];
            printf("%c%c ", first, second);
        }
        printf("\n");
    }
}

bool sarah2_validate_key(char *key) {
    // Make sure each possible pair appears exactly once.
    // Ideally we'd insert each pair into a hashmap and make sure it appeared only once.
    // Since I don't want to build a hashmap, we'll just check that each symbol in symbols
    // is used exactly 26 times in both the first and second position. This should be essentially
    // equivalent.
    int first_symbol_accumulator[num_symbols] = {0};
    int second_symbol_accumulator[num_symbols] = {0};

    for (int i = 0; i < sarah2_key_size; i += 2) {
        char first = key[i];
        char *ptr = strchr(symbols, first);
        if (ptr == NULL) {
            fprintf(stderr, "Validation failed: character %c is invalid\n", first);
            return false;
        }

        int index = ptr - symbols;
        int count = ++ (first_symbol_accumulator[index]);
        if (count > num_symbols) {
            fprintf(stderr, "Validation failed: too many characters %c found in first position\n", first);
            return false;
        }

        char second = key[i + 1];
        ptr = strchr(symbols, second);
        if (ptr == NULL) {
            fprintf(stderr, "Validation failed: character %c is invalid\n", first);
            return false;
        }

        index = ptr - symbols;
        count = ++ (second_symbol_accumulator[index]);
        if (count > num_symbols) {
            fprintf(stderr, "Validation failed: too many characters %c found in second position\n", first);
            return false;
        }
    } 

    // We need to run through each accumulator array to ensure there are num_symbols for each position. 
    // I'm not sure this is necessary with the previous checks, but we'll do it just to be safe.
    for (int i = 0; i < num_symbols; ++i) {
        if (first_symbol_accumulator[i] != num_symbols) {
            fprintf(stderr, "Validation failed: wrong number of characters %c found in first position: \n", i, first_symbol_accumulator[i]);
            return false;
        }
    }

    for (int i = 0; i < num_symbols; ++i) {
        if (second_symbol_accumulator[i] != num_symbols) {
            fprintf(stderr, "Validation failed: wrong number of characters %c found in second position: \n", i, second_symbol_accumulator[i]);
            return false;
        }
    }

    return true;
}

/* Returns an integer in the range [0, n).
 * Uses rand(), and so is affected-by/affects the same seed.
 * Source: https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c
 */
static int randint(int n) {
    if ((n - 1) == RAND_MAX) {
        return rand();
    } else {
        // Supporting larger values for n would requires an even more
        // elaborate implementation that combines multiple calls to rand()
        assert (n <= RAND_MAX);

        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        assert (end > 0);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end);

        return r % n;
    }
}

/**** Just for development / testing ****/
int main() {
    srand(time(NULL));

    // Tests
    assert(sarah2_output_size(5) == 6);
    assert(sarah2_output_size(4) == 4);

    char buf[sarah2_key_size];
    assert(!sarah2_validate_key(buf)); // Random bytes on the stack shouldn't be a valid key.

    sarah2_generate_key(buf);
    sarah2_print_key(buf);
    assert(sarah2_validate_key(buf));
}
