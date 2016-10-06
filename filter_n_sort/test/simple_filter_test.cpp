#include <gtest/gtest.h>

#include "../src/simple_filter.h"

using namespace app;

namespace {
class SimpleFilterTest : public ::testing::Test {
 protected:
  const std::string fname_ = "temp.txt";
  std::vector<std::string> lines_ = {"Line 1", "Line 2", "Test Line 3", "Line Test 4", "Line 5 Test",
      "Testfalse falseTest Testfalse Test falseTest"};
  SimpleFilterTest()  {
    std::ofstream file(fname_);
    for (auto &str : lines_)
      file << str << "\n";
    file.flush();
    file.close();
  }

  ~SimpleFilterTest() {
    remove(fname_.c_str());
  }

};

}


TEST_F(SimpleFilterTest, HasNextHandlesNoSuchFile) {
  SimpleFilter filter("no-such-file.txt", "word");
  EXPECT_FALSE(filter.HasNext());
}

TEST_F(SimpleFilterTest, HasNextHandlesOpen) {
  SimpleFilter filter(fname_, "word");
  EXPECT_TRUE(filter.HasNext());
}

TEST_F(SimpleFilterTest, HasNextHandlesAfterFirstRead) {
  SimpleFilter filter(fname_, "word");
  EXPECT_TRUE(filter.HasNext());
  filter.NextLine();
  EXPECT_TRUE(filter.HasNext());
}

TEST_F(SimpleFilterTest, HHasNextHandlesFullFile) {
  SimpleFilter filter(fname_, "word");
  for (size_t i = 0; i <= lines_.size(); i++) {
    ASSERT_TRUE(filter.HasNext());
    filter.NextLine();
  }
  EXPECT_FALSE(filter.HasNext());
}

TEST_F(SimpleFilterTest, NextLineHandlesFirstLine) {
  SimpleFilter filter(fname_, "word");
  EXPECT_EQ(lines_.front(), filter.NextLine());
}

TEST_F(SimpleFilterTest, NextLineHandlesFullFile) {
  SimpleFilter filter(fname_, "word");
  for (auto &str : lines_)
    EXPECT_EQ(str, filter.NextLine());

  EXPECT_EQ("", filter.NextLine());
}


TEST_F(SimpleFilterTest, NextLineFiltersFront) {
  SimpleFilter filter(fname_, "Test");
  for (size_t i = 0; i < 2; i++)
    EXPECT_EQ(lines_[i], filter.NextLine());

  EXPECT_EQ("Line 3", filter.NextLine());
}

TEST_F(SimpleFilterTest, NextLineFiltersBack) {
  SimpleFilter filter(fname_, "Test");
  for (size_t i = 0; i < 4; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ("Line 5", filter.NextLine());
}

TEST_F(SimpleFilterTest, NextLineFiltersMid) {
  SimpleFilter filter(fname_, "Test");
  for (size_t i = 0; i < 3; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ("Line 4", filter.NextLine());
}

TEST_F(SimpleFilterTest, NextLineSkipsFalsePositives) {
  SimpleFilter filter(fname_, "Test");
  auto falsepostive_answer = "Testfalse falseTest Testfalse falseTest";

  for (size_t i = 0; i < 5; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ(falsepostive_answer, filter.NextLine());


}

