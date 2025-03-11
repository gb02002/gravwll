#include "gtest/gtest.h"
#define private public
#include "cfx/settings/settings.h"

TEST(SettingsTest, ReadInputParsesArgs) {
  char *argv[] = {(char *)"./test", (char *)"--headless", (char *)"--N",
                  (char *)"1000"};
  int argc = 4;

  Settings settings(argc, argv);

  EXPECT_TRUE(settings.HEADLESS);
  EXPECT_TRUE(!settings.DEBUG);
  EXPECT_EQ(settings.N, 1000);
}
