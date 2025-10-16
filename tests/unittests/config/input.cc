#include "ctx/simulation_config.h"
#include "gtest/gtest.h"
#define private public

TEST(ConfigTest, set_defaults) {
  auto default_config = SimulationConfigBuilder().with_defaults().build();
  EXPECT_EQ(default_config.data_population_mode, SimulationConfig::PLUMMER);
  EXPECT_EQ(default_config.kHeadless, false);
  EXPECT_EQ(default_config.kNBodies, 100);
  EXPECT_EQ(default_config.integration_step, 1000);
  EXPECT_EQ(default_config.kTreeMaxDepth, 10);
  EXPECT_EQ(default_config.kDebug, true);
  EXPECT_EQ(default_config.random_seed, 42);
  EXPECT_EQ(default_config.kFpsDesired, 60);
}

TEST(ConfigTest, cli_overwriting) {
  char *argv[] = {(char *)"./test",  (char *)"-h",    (char *)"-n",
                  (char *)"1244",    (char *)"-v",    (char *)"-d",
                  (char *)"uniform", (char *)"--fps", (char *)"1000",
                  (char *)"-s",      (char *)"43",    (char *)"-is",
                  (char *)"422"};
  int argc = 13;
  auto overwriten_config = SimulationConfigBuilder()
                               .with_defaults()
                               .with_command_line(argc, argv)
                               .build();
  EXPECT_EQ(overwriten_config.data_population_mode, SimulationConfig::UNIFORM);
  EXPECT_EQ(overwriten_config.kNBodies, 1244);
  EXPECT_EQ(overwriten_config.integration_step, 422);
  EXPECT_EQ(overwriten_config.random_seed, 43);
  EXPECT_EQ(overwriten_config.kVerbose, true);
  EXPECT_EQ(overwriten_config.kDebug, true);
  EXPECT_EQ(overwriten_config.kFpsDesired, 1000);
}

TEST(ConfigTest, parse_file) {
  auto file_config = SimulationConfigBuilder()
                         .with_defaults()
                         .with_config_file("test.config.conf")
                         .build();
  EXPECT_EQ(file_config.data_population_mode, SimulationConfig::PLUMMER);
  EXPECT_EQ(file_config.kHeadless, true);
  EXPECT_EQ(file_config.kTreeMaxDepth, 45);
  EXPECT_EQ(file_config.kNBodies, 99999);
  EXPECT_EQ(file_config.kFpsDesired, 55);
  EXPECT_EQ(file_config.integration_step, 195);
  EXPECT_EQ(file_config.kVerbose, true);
}

// This is how usually we init the config. We check correct order of overwriting
TEST(ConfigTest, usual_flow) {
  char *argv[] = {(char *)"./test",  (char *)"-h",    (char *)"-n",
                  (char *)"1244",    (char *)"-v",    (char *)"-d",
                  (char *)"uniform", (char *)"--fps", (char *)"1000",
                  (char *)"-s",      (char *)"43",    (char *)"-is",
                  (char *)"422"};
  int argc = 13;
  auto general_config =
      SimulationConfigBuilder()
          .with_defaults()   // Here the kNBodies=100
          .with_config_file( // Here kNBodies=99999
              "test.config.conf")
          .with_command_line(argc, argv) // Here kNBodies is 1244
          .build();
  EXPECT_EQ(general_config.kNBodies, 1244);
}
