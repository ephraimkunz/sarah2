#include "sarah2.h"

#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Test encryption and decryption of sample given at http://laser-calcium.glitch.me.
void test_example_from_website() {
    char *key =
    "lgokksfvsjyw_soyqqwjx_bpdpbxwtkvsxjygyexskstgxejslow_g" 
    "bsxatnrlmgjwbmzyspaknbpgt_szpbvantbulr_anqykmbwgaxdouk" 
    "znsbqfpkwvsmoabhcfaeqalbwwxpnnkifatpranlawlqfhlufmlhsq" 
    "cmxjjixyqzihyczmnafbzqdfb_xnwq_yeftjcoh_cshnhplkaa_vwn" 
    "truumtj_lfwunpxfwibyyikjld_nfz_rmirvuzhakfwkqtzouyapez" 
    "qsvrrequojmq_imnjdpntoyoixhmegbrq_khcbu_nkubtdpdfpcnqp" 
    "hlpmckkpsoqrdtemjotwfjuralfxewevypjrrgjfngxzyuadljzdhb" 
    "gnxwryiwklhwaukwgkazodzvogfnfim_iabljaosswkouxsghf_hjn" 
    "hrvkd_hzpegcrhcatefkdeoftgxmmpzedwli_zbn_qopjswynz_pqx" 
    "jmcprnlznvfodlfupxxgdqtkkzrujpanoxkrmovsshyfvqaqikfwqm" 
    "ugjzw_hsdkwooovjhhbctugmvdidvvmmqlcrqbkmbfcyknscqwgheh" 
    "lnmcmyrmvfmscxvmdreptsy_mvndzwrcbvv_hemloqiozkearfdhhu" 
    "kdctfckcxkccdgwhlmdmuweu_ulsoiovz_pz_lp_prr_ft_bayqhku" 
    "ufgzsvpipjigrpe_bgs_uteeqdwdo_xuwfiinuaisyeolxrimrneab" 
    "atglmhssjccieqzhtiysmdserkpolc_tbdhtg_pyitmfzadnjgwxgj" 
    "yanjtcnwgdmujkyerb_cfewlmeyrws_dxbdjrsrxavl_cvlejuwchc" 
    "hjzrinhouhpcyykxvxmwcgeipuuorortvyrrdvzunxiuqyps_xzlxi" 
    "ekxrjvrjhdulzcxvfdhgytsaisiyguotlyhkpayvwetqgrkygpenuv" 
    "xhwmmzrzbqpppwozvezzcwqgfylaucomhiiqziolc_rwtxuiylvhi_" 
    "ygsufr__ncjqa_acvbqjpvnimadisfdxebzsqnwpxqjtrdrqzjvp_k" 
    "uecjxxxcohyh_oumeluqwaedarjlbiet_fxddaflwbvuahvwktqcvt" 
    "si_egsermkeykggvtywrcqcudszfxeunqoagczesujimlpttllqizg" 
    "fspthvzpltnsducdtbamuslvlwyjasbeipijbzirsdybfgbovnicgg" 
    "ieibdcqkajdbbwkqjhuagtkakegwxopqyxyqyzobilvldznrhqnmif" 
    "xlxsvzviochyecgqkkffmxbazxdythgeuporqejenobbbkhxaoztk_" 
    "n_jbnhgojjlovcqvivbjizouchphtmzb_w_mddgasn_jvoxtjxudaf" 
    "pltagbymkbmjclcetfydf_yntlpfnytvnfgionbtvgsrwzoefqgftz";

    assert(sarah2_validate_key(key));

    char *example = "attack_at_dawn";
    char *expected_encrypted = "kjtofsdxmcjdg_";

    size_t in_size = strlen(example);
    size_t out_size = sarah2_output_size(in_size);

    char encrypted[out_size + 1];
    char decrypted[out_size + 1];
    sarah2_encrypt(key, example, in_size, encrypted, Sarah2_Rounds_Maximal, 0);
    encrypted[out_size] = '\0';
    // printf("Encrypted: %s\n", encrypted);
    assert(strcmp(expected_encrypted, encrypted) == 0);

    sarah2_decrypt(key, encrypted, out_size, decrypted, Sarah2_Rounds_Maximal, 0);
    decrypted[out_size] = '\0';

    // printf("Decrypted: %s\n", decrypted);
    assert(strcmp(example, decrypted) == 0); // Not exactly the same since an underscore was added.
}

// Test encryption and decryption of long string that's odd length.
void test_odd_length_message() {
    char key[sarah2_key_size];
    sarah2_generate_key(key);

    char *example = "seth_is_a_dumb_dumb_hello_thisisanothertestofareallylongmessage_and_well_iadd_some_underscores_lets_go_sledding";
    size_t in_size = strlen(example);
    size_t out_size = sarah2_output_size(in_size);

    char encrypted[out_size + 1];
    char decrypted[out_size + 1];
    sarah2_encrypt(key, example, in_size, encrypted, Sarah2_Rounds_Maximal, 0);
    encrypted[out_size] = '\0';
    // printf("Encrypted: %s\n", encrypted);

    sarah2_decrypt(key, encrypted, out_size, decrypted, Sarah2_Rounds_Maximal, 0);
    decrypted[out_size] = '\0';

    // printf("Decrypted: %s\n", decrypted);
    assert(strcmp(example, decrypted) != 0); // Not exactly the same since an underscore was added.
    assert(strncmp(example, decrypted, in_size) == 0); // But the same up to the underscore.
    assert(decrypted[out_size - 1] == '_'); // And and underscore was added.
}

void test_output_size_calculation() {
    assert(sarah2_output_size(5) == 6);
    assert(sarah2_output_size(4) == 4);
}

void test_key_generation_validation() {
    // Test key generation and validation.
    char key[sarah2_key_size];
    assert(!sarah2_validate_key(key)); // Random bytes on the stack shouldn't be a valid key.

    sarah2_generate_key(key);
    // sarah2_print_key(key);
    assert(sarah2_validate_key(key));
}

int main()
{
    srand(time(NULL));

    for (int i = 0; i < 1000; ++i) {
        test_output_size_calculation();
        test_key_generation_validation();
        test_odd_length_message();
        test_example_from_website();
    }
}
