//
// Created by sacharc on 08.06.16.
//

/**
		Sprawdza, czy info_string i match_string definiują taką samą krotkę
*/
#include <gtest/gtest.h>

bool info_string_match_string_equals(const char * info_string, const char * match_string)
{
    size_t info_string_position = 0;
    size_t match_string_position = 0;

    for(;;)
    {
        //Jeśli znaki nie są identyczne
        if(info_string[info_string_position] != match_string[match_string_position])
            return false;

        //Jeśli oba się kończą (to w zasadzie ma sens tylko dla pierwszego znaku)
        if(info_string[info_string_position] == NULL)
            return true;

        //Przesuwamy iterator info_string o jeden znak do przodu
        info_string_position++;

        //Przesuwamy iterator match_string na znak po przecinku (albo null)
        for(;;)
        {
            if(match_string[match_string_position] == ',' || match_string[match_string_position] == NULL)
                break;
            match_string_position++;
        }

        //Jeśli match-string się skończył - sprawdzamy czy info_string też się skończył
        if(match_string[match_string_position] == NULL)
            return info_string[info_string_position] == NULL;

        //Nie - jesteśmy na przecinku, przesuwamy się za niego
        match_string_position++;
    }
}

TEST(info_string_match_string_equals_test, test1)
{
    ASSERT_TRUE(info_string_match_string_equals("i", "i"));
    ASSERT_TRUE(info_string_match_string_equals("isf", "i,s,f"));
    ASSERT_TRUE(info_string_match_string_equals("is", "i,s"));
    ASSERT_TRUE(info_string_match_string_equals("sf", "s,f"));
}

TEST(info_string_match_string_equals_test, test2)
{
    ASSERT_TRUE(info_string_match_string_equals("i", "i>=4"));
    ASSERT_TRUE(info_string_match_string_equals("isf", "i<=4,s,f"));
    ASSERT_TRUE(info_string_match_string_equals("is", "i>4,s>s"));
    ASSERT_TRUE(info_string_match_string_equals("ss", "s>test,s>tost"));
}

TEST(info_string_match_string_equals_test, test3)
{
    ASSERT_FALSE(info_string_match_string_equals("i", "s"));
    ASSERT_FALSE(info_string_match_string_equals("fs", "i,s"));
    ASSERT_FALSE(info_string_match_string_equals("ss", "i,s"));
    ASSERT_FALSE(info_string_match_string_equals("sss", "i>0,s>tests"));
    ASSERT_FALSE(info_string_match_string_equals("fs", "i,s>4,i"));
}

