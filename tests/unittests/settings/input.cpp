#include "gtest/gtest.h"
#define private public
#include "cfx/settings/settings.h"

int sum_2_ints(int a, int b) { return a + b; }

TEST(SettingsTest, ReadInputParsesArgs) {
  char *argv[] = {(char *)"./test", (char *)"--headless", (char *)"--N",
                  (char *)"10"};
  int argc = 4;

  Settings settings;
  settings.ReadInput(argc, argv);

  EXPECT_TRUE(settings.HEADLESS);
  EXPECT_TRUE(!settings.DEBUG);
  EXPECT_EQ(settings.N, 10);
}

TEST(TEST_SUM_INT, BASIC_CHECK) { EXPECT_EQ(sum_2_ints(1, 2), 3); }
