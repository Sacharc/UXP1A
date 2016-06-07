
#include <gtest/gtest.h>
#include "../include/linda.h"

TEST(Functional_Int_Test, anyInt) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    int value;
    EXPECT_TRUE(linda_input(1, "i", &value));
    EXPECT_EQ(0, value);
    linda_end();
}

TEST(Functional_Int_Test, anyIntNeg) {
    linda_init();
    int value;
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, anyIntNeg1) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    int value;
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, anyIntNeg2) {
    linda_init();
    EXPECT_TRUE(linda_output("s", "test"));
    int value;
    EXPECT_FALSE(linda_input(1, "i", &value));
    linda_end();
}

TEST(Functional_Int_Test, less_test) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    int value;
    EXPECT_TRUE(linda_input(1, "i<1", &value));
    EXPECT_EQ(0, value);
    linda_end();
}

TEST(Functional_Int_Test, less_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_input(1, "i<0", &value));
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test1) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    int value;
    EXPECT_TRUE(linda_input(1, "i<=1", &value));
    EXPECT_EQ(0, value);
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test2) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    int value;
    EXPECT_TRUE(linda_input(1, "f<=0", &value));
    EXPECT_EQ(0, value);
    linda_end();
}

TEST(Functional_Int_Test, less_eq_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_input(1, "f<=0", &value));
    linda_end();
}

TEST(Functional_Int_Test, greater_test) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_input(1, "i>1", &value));
    EXPECT_EQ(3, value);
    linda_end();
}

TEST(Functional_Int_Test, greater_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_input(1, "i>2", &value));
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test1) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_input(1, "i>=1", &value));
    EXPECT_EQ(3, value);
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test2) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_input(1, "i>=3", &value));
    EXPECT_EQ(3, value);
    linda_end();
}

TEST(Functional_Int_Test, greater_eq_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_input(1, "i>=2", &value));
    linda_end();
}

TEST(Functional_Int_Test, eq_test) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 3));
    int value;
    EXPECT_TRUE(linda_input(1, "i=3", &value));
    EXPECT_EQ(3, value);
    linda_end();
}

TEST(Functional_Int_Test, eq_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 1));
    int value;
    EXPECT_FALSE(linda_input(1, "i=2", &value));
    linda_end();
}