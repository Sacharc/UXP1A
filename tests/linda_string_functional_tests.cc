
#include <gtest/gtest.h>
#include "../include/linda.h"

TEST(Functional_String_Test, any_string)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "test"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s", &value));
    EXPECT_EQ(strcmp("test", value), 0);
    linda_end();
}

TEST(Functional_String_Test, any_string_neg)
{
    linda_init();
    char* value;
    EXPECT_FALSE(linda_input(1, "s", &value));
    linda_end();
}

TEST(Functional_String_Test, any_string_neg1)
{
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    char* value;
    EXPECT_FALSE(linda_input(1, "s", &value));
    linda_end();
}

TEST(Functional_String_Test, any_string_neg2)
{
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    char* value;
    EXPECT_FALSE(linda_input(1, "s", &value));
    linda_end();
}

TEST(Functional_String_Test, less_test)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s<c", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_test_neg)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_FALSE(linda_input(1, "s<b", &value));
    linda_end();
}

TEST(Functional_String_Test, less_eq_test1)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s<=c", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_eq_test2)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s<=b", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_eq_test_neg)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_FALSE(linda_input(1, "s<=a", &value));
    linda_end();
}

TEST(Functional_String_Test, greater_test)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s>a", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_test_neg)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_FALSE(linda_input(1, "s>c", &value));
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test1)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s>=a", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test2)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s>=b", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test_neg)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_FALSE(linda_input(1, "s>=c", &value));
    linda_end();
}

TEST(Functional_String_Test, eq_test)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_TRUE(linda_input(1, "s=b", &value));
    EXPECT_EQ(strcmp(value, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, eq_test_neg)
{
    linda_init();
    EXPECT_TRUE(linda_output("s", "b"));
    char* value;
    EXPECT_FALSE(linda_input(1, "s=c", &value));
    linda_end();
}
