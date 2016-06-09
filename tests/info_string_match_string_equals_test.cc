
/**
		Sprawdza, czy info_string i match_string definiują taką samą krotkę
*/
#include <gtest/gtest.h>
#include "tests.h"


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

