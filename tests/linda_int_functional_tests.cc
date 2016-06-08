
#include <gtest/gtest.h>
#include "../include/linda.h"

TEST(Functional_Int_Test, any_int)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 2));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i", &value));
//    EXPECT_EQ(2, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i", &value1));
    EXPECT_EQ(2, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, any_int_neg)
{
    int seg_id = linda_init();
    int value;
//    EXPECT_FALSE(linda_read(1, "i", &value));
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, any_int_neg1)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("f", 0.1));
    int value;
//    EXPECT_FALSE(linda_read(1, "i", &value));
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, any_int_neg2)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("s", "test"));
    int value;
//    EXPECT_FALSE(linda_read(1, "i", &value));
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, less_test)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 2));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i<1", &value));
//    EXPECT_EQ(2, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i<1", &value1));
    EXPECT_EQ(2, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, less_test_neg)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
//    EXPECT_FALSE(linda_read(1, "i<0", &value));
    EXPECT_FALSE(linda_input(1, "i<0", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, less_eq_test1)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i<=2", &value));
//    EXPECT_EQ(1, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i<=2", &value1));
    EXPECT_EQ(1, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, less_eq_test2)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
//    int value;
//    EXPECT_TRUE(linda_read(1, "f<=1", &value));
//    EXPECT_EQ(1, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "f<=1", &value1));
    EXPECT_EQ(1, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, less_eq_test_neg)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
//    EXPECT_FALSE(linda_read(1, "f<=0", &value));
    EXPECT_FALSE(linda_input(1, "f<=0", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, greater_test)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 3));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i>1", &value));
//    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i>1", &value1));
    EXPECT_EQ(3, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, greater_test_neg)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
//    EXPECT_FALSE(linda_read(1, "i>2", &value));
    EXPECT_FALSE(linda_input(1, "i>2", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, greater_eq_test1)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 3));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i>=1", &value));
//    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i>=1", &value1));
    EXPECT_EQ(3, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, greater_eq_test2)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 3));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i>=3", &value));
//    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i>=3", &value1));
    EXPECT_EQ(3, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, greater_eq_test_neg)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
//    EXPECT_FALSE(linda_read(1, "i>=2", &value));
    EXPECT_FALSE(linda_input(1, "i>=2", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, eq_test)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 3));
//    int value;
//    EXPECT_TRUE(linda_read(1, "i=3", &value));
//    EXPECT_EQ(3, value);

    int value1;
    EXPECT_TRUE(linda_input(1, "i==3", &value1));
    EXPECT_EQ(3, value1);
    linda_end(seg_id);
}

TEST(Functional_Int_Test, eq_test_neg)
{
    int seg_id = linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
//    EXPECT_FALSE(linda_read(1, "i=2", &value));
    EXPECT_FALSE(linda_input(1, "i==2", &value));
    linda_end(seg_id);
}

TEST(Functional_Int_Test, triple_output_test)
{
    int seg_id = linda_init();
    for(int i = 0; i < 3; i++)
        EXPECT_TRUE(linda_output("i", 1));

    for(int i = 0; i < 3; i++)
    {
        int value;
        EXPECT_TRUE(linda_input(1, "i", &value));
        EXPECT_EQ(1, value);
    }

    linda_end(seg_id);
}