
#include <gtest/gtest.h>
#include "../include/linda.h"

TEST(Functional_Float_Test, anyFloat) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f", &value));
    EXPECT_EQ(0.0, value);
    linda_end();
}

TEST(Functional_Float_Test, anyFloatNeg) {
    linda_init();
    float value;
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, anyFloatNeg1) {
    linda_init();
    EXPECT_TRUE(linda_output("i", 0));
    float value;
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, anyFloatNeg2) {
    linda_init();
    EXPECT_TRUE(linda_output("s", "test"));
    float value;
    EXPECT_FALSE(linda_input(1, "f", &value));
    linda_end();
}

TEST(Functional_Float_Test, less_test) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f<1.0", &value));
    EXPECT_EQ(0.0, value);
    linda_end();
}

TEST(Functional_Float_Test, less_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 1.0));
    float value;
    EXPECT_FALSE(linda_input(1, "f<0.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test1) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f<=1.0", &value));
    EXPECT_EQ(0.0, value);
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test2) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 0.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f<=0.0", &value));
    EXPECT_EQ(0.0, value);
    linda_end();
}

TEST(Functional_Float_Test, less_eq_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 1.0));
    float value;
    EXPECT_FALSE(linda_input(1, "f<=0.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, greater_test) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 3.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f>1.0", &value));
    EXPECT_EQ(3.0, value);
    linda_end();
}

TEST(Functional_Float_Test, greater_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 1.0));
    float value;
    EXPECT_FALSE(linda_input(1, "f>2.0", &value));
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test1) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 3.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f>=1.0", &value));
    EXPECT_EQ(3.0, value);
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test2) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 3.0));
    float value;
    EXPECT_TRUE(linda_input(1, "f>=3.0", &value));
    EXPECT_EQ(3.0, value);
    linda_end();
}

TEST(Functional_Float_Test, greater_eq_test_neg) {
    linda_init();
    EXPECT_TRUE(linda_output("f", 1.0));
    float value;
    EXPECT_FALSE(linda_input(1, "f>=2.0", &value));
    linda_end();
}