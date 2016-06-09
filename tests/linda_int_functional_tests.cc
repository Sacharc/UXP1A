
#include <gtest/gtest.h>
#include "tests.h"

TEST(Functional_Int_Test, any_int)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 2));
    int value;
    EXPECT_TRUE(linda_read(t, "i", &value));
    EXPECT_EQ(2, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i", &value1));
    EXPECT_EQ(2, value1);
    EXPECT_FALSE(linda_read(t, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, any_int_neg)
{
    ASSERT_TRUE(linda_init());
    int value;
    EXPECT_FALSE(linda_read(t, "i", &value));
    EXPECT_FALSE(linda_input(t, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, any_int_neg1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.1));
    int value;
    EXPECT_FALSE(linda_read(t, "i", &value));
    EXPECT_FALSE(linda_input(t, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, any_int_neg2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "test"));
    int value;
    EXPECT_FALSE(linda_read(t, "i", &value));
    EXPECT_FALSE(linda_input(t, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, less_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 2));
    int value;
    EXPECT_TRUE(linda_read(t, "i<3", &value));
    EXPECT_EQ(2, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i<3", &value1));
    EXPECT_EQ(2, value1);
    linda_end();
}

TEST(Functional_Int_Test, less_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_read(t, "i<0", &value));
    EXPECT_FALSE(linda_input(t, "i<0", &value));
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_TRUE(linda_read(t, "i<=2", &value));
    EXPECT_EQ(1, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i<=2", &value1));
    EXPECT_EQ(1, value1);
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_TRUE(linda_read(t, "i<=1", &value));
    EXPECT_EQ(1, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i<=1", &value1));
    EXPECT_EQ(1, value1);
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_read(t, "f<=0", &value));
    EXPECT_FALSE(linda_input(t, "f<=0", &value));
    linda_end();
}

TEST(Functional_Int_Test, greater_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_read(t, "i>1", &value));
    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i>1", &value1));
    EXPECT_EQ(3, value1);
    linda_end();
}

TEST(Functional_Int_Test, greater_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_read(t, "i>2", &value));
    EXPECT_FALSE(linda_input(t, "i>2", &value));
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_read(t, "i>=1", &value));
    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i>=1", &value1));
    EXPECT_EQ(3, value1);
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_read(t, "i>=3", &value));
    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i>=3", &value1));
    EXPECT_EQ(3, value1);
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_read(t, "i>=2", &value));
    EXPECT_FALSE(linda_input(t, "i>=2", &value));
    linda_end();
}

TEST(Functional_Int_Test, eq_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_read(t, "i==3", &value));
    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(t, "i==3", &value1));
    EXPECT_EQ(3, value1);
    linda_end();
}

TEST(Functional_Int_Test, eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_read(t, "i==2", &value));
    EXPECT_FALSE(linda_input(t, "i==2", &value));
    linda_end();
}

TEST(Functional_Int_Test, triple_output_test)
{
    ASSERT_TRUE(linda_init());
    for(int i = 0; i < 3; i++)
        EXPECT_TRUE(linda_output("i", 1));

    for(int i = 0; i < 3; i++)
    {
        int value;
        EXPECT_TRUE(linda_input(t, "i", &value));
        EXPECT_EQ(1, value);
    }

    linda_end();
}