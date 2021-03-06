#include "sarah2.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <math.h>

#define NUM_SYMBOLS 27 // a-z + _

// Indexes into array representing S-Box (key), given the i, j position into
// a 27 by 27 matrix holding pairs of characters at each position.
#define PAIR_FIRST_INDEX(r, c) (((c)*2) + ((r) * NUM_SYMBOLS * 2))
#define PAIR_SECOND_INDEX(r, c) ((PAIR_FIRST_INDEX(r, c)) + 1)

const size_t sarah2_key_size = NUM_SYMBOLS * NUM_SYMBOLS * 2; // NUM_SYMBOLS in 2D array, with two chars at each position.
const char *symbols = "_abcdefghijklmnopqrstuvwxyz";

static int randint(int n);

size_t sarah2_output_size(size_t message_len)
{
    // Odd length messages will be padded by _ to make them even length.
    if ((message_len % 2) != 0)
    {
        return message_len + 1;
    }

    return message_len;
}

static int calculate_num_rounds(size_t message_len, Sarah2_Rounds rounds, size_t custom_num_rounds)
{
    switch (rounds)
    {
    case Sarah2_Rounds_Custom:
        return custom_num_rounds;
    case Sarah2_Rounds_Minimal:
        return ceil(log2(message_len));
    case Sarah2_Rounds_Medium:
        return ceil(log2(message_len)) + 2;
    case Sarah2_Rounds_Maximal:
    default:
        return ceil(log2(message_len)) * 2;
    }
}

void sarah2_encrypt(char *key, char *message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds)
{
    if (key == NULL || message == NULL || out_buf == NULL)
    {
        // fprintf(stderr, "Key, message, or out_buf are null, won't continue.\n");
        return;
    }

    memcpy(out_buf, message, message_len);

    // Pad by '_' if necessary.
    size_t out_buf_size = sarah2_output_size(message_len);
    if (message_len < out_buf_size)
    {
        out_buf[out_buf_size - 1] = '_';
        message_len = out_buf_size;
    }

    size_t half_length = message_len / 2;
    char odds[half_length];
    char evens[half_length];
    int num_rounds = calculate_num_rounds(message_len, rounds, custom_num_rounds);

    for (int round = 0; round < num_rounds; ++round)
    {
        // Substitution
        for (size_t i = 0; i < message_len; i += 2)
        {
            int row_index = strchr(symbols, out_buf[i]) - symbols;
            int column_index = strchr(symbols, out_buf[i + 1]) - symbols;

            out_buf[i] = key[PAIR_FIRST_INDEX(row_index, column_index)];
            out_buf[i + 1] = key[PAIR_SECOND_INDEX(row_index, column_index)];
        }

        // Permutation. It seems like there should be a way to do this in-place in out_buf.
        // Don't do this on the last round since it's useless (involves no key material).
        if (round != num_rounds - 1)
        {
            int odd_index, even_index;
            odd_index = even_index = 0;

            for (size_t i = 1; i <= message_len; ++i)
            {
                if (i % 2 == 0)
                {
                    evens[even_index++] = out_buf[i - 1];
                }
                else
                {
                    odds[odd_index++] = out_buf[i - 1];
                }
            }

            int out_index = 0;
            for (size_t i = 0; i < half_length; ++i)
            {
                out_buf[out_index++] = odds[i];
            }

            for (size_t i = 0; i < half_length; ++i)
            {
                out_buf[out_index++] = evens[i];
            }
        }
    }
}

void sarah2_decrypt(char *key, char *encrypted_message, size_t message_len, char *out_buf, Sarah2_Rounds rounds, size_t custom_num_rounds)
{
    if (key == NULL || encrypted_message == NULL || out_buf == NULL)
    {
        // fprintf(stderr, "Key, message, or out_buf are null, won't continue.\n");
        return;
    }

    memcpy(out_buf, encrypted_message, message_len);

    size_t half_length = message_len / 2;
    char first_half[half_length];
    char second_half[half_length];
    int num_rounds = calculate_num_rounds(message_len, rounds, custom_num_rounds);

    for (int round = 0; round < num_rounds; ++round)
    {
        // Unpermutation. Don't do this on the first round.
        if (round != 0)
        {
            memcpy(first_half, out_buf, half_length);
            memcpy(second_half, &out_buf[half_length], half_length);

            int out_index = 0;
            for (size_t i = 0; i < half_length; ++i)
            {
                out_buf[out_index++] = first_half[i];
                out_buf[out_index++] = second_half[i];
            }
        }

        // Unsubstitution
        for (size_t i = 0; i < message_len; i += 2)
        {
            char first = out_buf[i];
            char second = out_buf[i + 1];

            // Find it in the key. TODO: Profiling shows this lookup is very slow, as we must go through the whole key looking for a pair.
            // Could we create some sort of constant-time cache data structure to speedup lookups?
            for (size_t r = 0; r < NUM_SYMBOLS; ++r)
            {
                for (size_t c = 0; c < NUM_SYMBOLS; ++c)
                {
                    if (key[PAIR_FIRST_INDEX(r, c)] == first && key[PAIR_SECOND_INDEX(r, c)] == second)
                    {
                        out_buf[i] = symbols[r];
                        out_buf[i + 1] = symbols[c];
                    }
                }
            }
        }
    }
}

void sarah2_generate_key(char *out_buf)
{
    // Fill matrix with pairs.
    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        for (size_t j = 0; j < NUM_SYMBOLS; ++j)
        {
            out_buf[PAIR_FIRST_INDEX(i, j)] = symbols[i];
            out_buf[PAIR_SECOND_INDEX(i, j)] = symbols[j];
        }
    }

    // Randomize pair positions.
    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        for (size_t j = 0; j < NUM_SYMBOLS; ++j)
        {
            int swap_pos_x = randint(NUM_SYMBOLS);
            int swap_pos_y = randint(NUM_SYMBOLS);

            // Swap the pair at the i/j'th position with the pair at the swap_pos_x/y position.
            char temp = out_buf[PAIR_FIRST_INDEX(i, j)];
            out_buf[PAIR_FIRST_INDEX(i, j)] = out_buf[PAIR_FIRST_INDEX(swap_pos_y, swap_pos_x)];
            out_buf[PAIR_FIRST_INDEX(swap_pos_y, swap_pos_x)] = temp;

            temp = out_buf[PAIR_SECOND_INDEX(i, j)];
            out_buf[PAIR_SECOND_INDEX(i, j)] = out_buf[PAIR_SECOND_INDEX(swap_pos_y, swap_pos_x)];
            out_buf[PAIR_SECOND_INDEX(swap_pos_y, swap_pos_x)] = temp;
        }
    }
}

static void print_header()
{
    // Leave space for the vertical header.
    printf("  ");
    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        printf("%2c ", symbols[i]);
    }

    printf("\n  ");

    for (size_t i = 0; i < (NUM_SYMBOLS * 3); ++i)
    {
        printf("-");
    }

    printf("\n");
}

void sarah2_print_key(char *key)
{
    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        if (i == 0)
        {
            print_header();
        }

        // Print the portion of the vertical header.
        printf("%c| ", symbols[i]);

        for (size_t j = 0; j < NUM_SYMBOLS; ++j)
        {
            char first = key[PAIR_FIRST_INDEX(i, j)];
            char second = key[PAIR_SECOND_INDEX(i, j)];
            printf("%c%c ", first, second);
        }
        printf("\n");
    }
}

bool sarah2_validate_key(char *key)
{
    // Make sure each possible pair appears exactly once.
    // Ideally we'd insert each pair into a hashmap and make sure it appeared only once.
    // Since I don't want to build a hashmap, we'll just check that each symbol in symbols
    // is used exactly 26 times in both the first and second position. This should be essentially
    // equivalent.
    unsigned first_symbol_accumulator[NUM_SYMBOLS];
    unsigned second_symbol_accumulator[NUM_SYMBOLS];
    memset(first_symbol_accumulator, 0, NUM_SYMBOLS * sizeof(int));
    memset(second_symbol_accumulator, 0, NUM_SYMBOLS * sizeof(int));

    for (size_t i = 0; i < sarah2_key_size; i += 2)
    {
        char first = key[i];
        char *ptr = strchr(symbols, first);
        if (ptr == NULL)
        {
            // fprintf(stderr, "Validation failed: character '%c' is invalid\n", first);
            return false;
        }

        int index = ptr - symbols;

        if (index < 0 || index > 26)
        {
            // An invalid key may have the null character, which strchr will find at the end of our string. Prevent that.
            // fprintf(stderr, "Validation failed: character '%c' is was found at an invalid index: %d\n", first, index);
            return false;
        }

        size_t count = ++(first_symbol_accumulator[index]);
        if (count > NUM_SYMBOLS)
        {
            // fprintf(stderr, "Validation failed: too many characters '%c' found in first position\n", first);
            return false;
        }

        char second = key[i + 1];
        ptr = strchr(symbols, second);
        if (ptr == NULL)
        {
            // fprintf(stderr, "Validation failed: character '%c' is invalid\n", first);
            return false;
        }

        index = ptr - symbols;

        if (index < 0 || index > 26)
        {
            // An invalid key may have the null character, which strchr will find at the end of our string. Prevent that.
            // fprintf(stderr, "Validation failed: character '%c' is was found at an invalid index: %d\n", first, index);
            return false;
        }
        count = ++(second_symbol_accumulator[index]);
        if (count > NUM_SYMBOLS)
        {
            // fprintf(stderr, "Validation failed: too many characters '%c' found in second position\n", first);
            return false;
        }
    }

    // We need to run through each accumulator array to ensure there are NUM_SYMBOLS for each position.
    // I'm not sure this is necessary with the previous checks, but we'll do it just to be safe.
    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        if (first_symbol_accumulator[i] != NUM_SYMBOLS)
        {
            // fprintf(stderr, "Validation failed: wrong number of characters '%c' found in first position: %ul\n", symbols[i], first_symbol_accumulator[i]);
            return false;
        }
    }

    for (size_t i = 0; i < NUM_SYMBOLS; ++i)
    {
        if (second_symbol_accumulator[i] != NUM_SYMBOLS)
        {
            // fprintf(stderr, "Validation failed: wrong number of characters '%c' found in second position: %ul\n", symbols[i], second_symbol_accumulator[i]);
            return false;
        }
    }

    return true;
}

/* Returns an integer in the range [0, n).
 * Uses rand(), and so is affected-by/affects the same seed.
 * Source: https://stackoverflow.com/questions/822323/how-to-generate-a-random-int-in-c
 */
static int randint(int n)
{
    if ((n - 1) == RAND_MAX)
    {
        return rand();
    }
    else
    {
        // Supporting larger values for n would requires an even more
        // elaborate implementation that combines multiple calls to rand()
        assert(n <= RAND_MAX);

        // Chop off all of the values that would cause skew...
        int end = RAND_MAX / n; // truncate skew
        assert(end > 0);
        end *= n;

        // ... and ignore results from rand() that fall above that limit.
        // (Worst case the loop condition should succeed 50% of the time,
        // so we can expect to bail out of this loop pretty quickly.)
        int r;
        while ((r = rand()) >= end)
            ;

        return r % n;
    }
}
