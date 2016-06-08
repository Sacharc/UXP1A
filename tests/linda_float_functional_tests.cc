
#include <gtest/gtest.h>
#include "../include/linda.c"

TEST(Functional_Float_Test, any_float)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.1));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f", &value));
//    EXPECT_EQ(0.0, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f", &value1));
    EXPECT_EQ(0.1, value1);
    linda_end();
}

TEST(Functional_Float_Test, any_float_neg)
{
    ASSERT_TRUE(linda_init());
    double value;
//    EXPECT_FALSE(linda_read(1, "f", &value));
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, any_float_neg1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("i", 0));
    double value;
//    EXPECT_FALSE(linda_read(1, "f", &value));
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, any_float_neg2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("s", "test"));
    double value;
//    EXPECT_FALSE(linda_read(1, "f", &value));
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, less_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.1));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f<1.0", &value));
//    EXPECT_EQ(0.0, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f<1.0", &value1));
    EXPECT_EQ(0.1, value1);
    linda_end();
}

TEST(Functional_Float_Test, less_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 1.0));
    double value;
//    EXPECT_FALSE(linda_read(1, "f<0.0", &value));
    EXPECT_FALSE(linda_input(1, "f<0.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.1));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f<=1.0", &value));
//    EXPECT_EQ(0.1, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f<=1.0", &value1));
    EXPECT_EQ(0.1, value1);
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 0.1));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f<=0.1", &value));
//    EXPECT_EQ(0.1, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f<=0.1", &value1));
    EXPECT_EQ(0.1, value1);
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 1.0));
    double value;
//    EXPECT_FALSE(linda_read(1, "f<=0.0", &value));
    EXPECT_FALSE(linda_input(1, "f<=0.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, greater_test)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 3.0));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f>1.0", &value));
//    EXPECT_EQ(3.0, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f>1.0", &value1));
    EXPECT_EQ(3.0, value1);
    linda_end();
}

TEST(Functional_Float_Test, greater_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 1.0));
    double value;
//    EXPECT_FALSE(linda_read(1, "f>2.0", &value));
    EXPECT_FALSE(linda_input(1, "f>2.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test1)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 3.0));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f>=1.0", &value));
//    EXPECT_EQ(3.0, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f>=1.0", &value1));
    EXPECT_EQ(3.0, value1);
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test2)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 3.0));
//    double value;
//    EXPECT_TRUE(linda_read(1, "f>=3.0", &value));
//    EXPECT_EQ(3.0, value);

    double value1;
    EXPECT_TRUE(linda_input(1, "f>=3.0", &value1));
    EXPECT_EQ(3.0, value1);
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test_neg)
{
    ASSERT_TRUE(linda_init());
    EXPECT_TRUE(linda_output("f", 1.0));
    double value;
//    EXPECT_FALSE(linda_read(1, "f>=2.0", &value));
    EXPECT_FALSE(linda_input(1, "f>=2.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, triple_output_test)
{
    ASSERT_TRUE(linda_init());
    for(int i = 0; i < 3; i++)
        EXPECT_TRUE(linda_output("f", 3.0));

    for(int i = 0; i < 3; i++)
    {
        double value;
        EXPECT_TRUE(linda_input(1, "f", &value));
        EXPECT_EQ(3.0, value);
    }

    linda_end();
}