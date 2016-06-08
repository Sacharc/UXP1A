
#include <gtest/gtest.h>
#include "../include/linda.h"

TEST(Functional_String_Test, any_string)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "test"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s", value));
    EXPECT_EQ(strcmp("test", value), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s", value1));
    EXPECT_EQ(strcmp("test", value1), 0);
    linda_end();
}

TEST(Functional_String_Test, any_string_neg)
{
    ASSERT_TRUE(linda_init());
    char value[128];
    EXPECT_FALSE(linda_read(1, "s", value));
    EXPECT_FALSE(linda_input(1, "s", value));
    linda_end();
}

TEST(Functional_String_Test, any_string_neg1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.0));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s", value));
    EXPECT_FALSE(linda_input(1, "s", value));
    linda_end();
}

TEST(Functional_String_Test, any_string_neg2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 0));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s", value));
    EXPECT_FALSE(linda_input(1, "s", value));
    linda_end();
}

TEST(Functional_String_Test, less_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s<c", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s<c", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s<b", value));
    EXPECT_FALSE(linda_input(1, "s<b", value));
    linda_end();
}

TEST(Functional_String_Test, less_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s<=c", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s<=c", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s<=b", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s<=b", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, less_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s<=a", value));
    EXPECT_FALSE(linda_input(1, "s<=a", value));
    linda_end();
}

TEST(Functional_String_Test, greater_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s>a", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s>a", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s>c", value));
    EXPECT_FALSE(linda_input(1, "s>c", value));
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s>=a", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s>=a", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s>=b", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s>=b", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, greater_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s>=c", value));
    EXPECT_FALSE(linda_input(1, "s>=c", value));
    linda_end();
}

TEST(Functional_String_Test, eq_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_TRUE(linda_read(1, "s==b", value));
    EXPECT_EQ(strcmp(value, "b"), 0);

    char value1[128];
    EXPECT_TRUE(linda_input(1, "s==b", value1));
    EXPECT_EQ(strcmp(value1, "b"), 0);
    linda_end();
}

TEST(Functional_String_Test, eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "b"));
    char value[128];
    EXPECT_FALSE(linda_read(1, "s==c", value));
    EXPECT_FALSE(linda_input(1, "s==c", value));
    linda_end();
}

TEST(Functional_String_Test, triple_output_test)
{
    ASSERT_TRUE(linda_init());
    for(int i = 0; i < 3; i++)
        EXPECT_TRUE(linda_output("s", "triple_output_test"));

    for(int i = 0; i < 3; i++)
    {
        char value[128];
        EXPECT_TRUE(linda_input(1, "s", value));
        EXPECT_EQ(strcmp("triple_output_test", value), 0);
    }

    linda_end();
}
