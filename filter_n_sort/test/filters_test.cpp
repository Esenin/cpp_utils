#include <gtest/gtest.h>

#include "../src/regex_filter.h"
#include "../src/raw_filter.h"

using namespace app;
namespace {

template <typename T>
class FiltersTest : public ::testing::Test {
 public:
  const std::string fname_ = "temp.txt";
  std::vector<std::string> lines_ =
      {"Line 1", "Line 2", "Test Line 3", "Line Test 4", "Line 5 Test", "Testfalse falseTest Testfalse Test falseTest"};

  FiltersTest() {
    std::ofstream file(fname_);
    for (auto &str : lines_)
      file << str << "\n";
    file.flush();
    file.close();
  }

  virtual ~FiltersTest() {
    remove(fname_.c_str());
  }
};

typedef ::testing::Types<RegexFilter, RawFilter> FiltersImpl;

TYPED_TEST_CASE(FiltersTest, FiltersImpl);

}



TYPED_TEST(FiltersTest, HasNextHandlesNoSuchFile) {
  ASSERT_THROW(TypeParam filter("no-such-file.txt", "word"), std::runtime_error);
//  EXPECT_FALSE(filter.HasNext());
}

TYPED_TEST(FiltersTest, HasNextHandlesOpen) {
  TypeParam filter(this->fname_, "word");
  EXPECT_TRUE(filter.HasNext());
}

TYPED_TEST(FiltersTest, HasNextHandlesAfterFirstRead) {
  TypeParam filter(this->fname_, "word");
  EXPECT_TRUE(filter.HasNext());
  filter.NextLine();
  EXPECT_TRUE(filter.HasNext());
}

TYPED_TEST(FiltersTest, HHasNextHandlesFullFile) {
  TypeParam filter(this->fname_, "word");
  for (size_t i = 0; i <= this->lines_.size(); i++) {
    ASSERT_TRUE(filter.HasNext());
    filter.NextLine();
  }
  EXPECT_FALSE(filter.HasNext());
}

TYPED_TEST(FiltersTest, NextLineHandlesFirstLine) {
  TypeParam filter(this->fname_, "word");
  EXPECT_EQ(this->lines_.front(), filter.NextLine());
}

TYPED_TEST(FiltersTest, NextLineHandlesFullFile) {
  TypeParam filter(this->fname_, "word");
  for (auto &str : this->lines_)
    EXPECT_EQ(str, filter.NextLine());

  EXPECT_EQ("", filter.NextLine());
}


TYPED_TEST(FiltersTest, NextLineFiltersFront) {
  TypeParam filter(this->fname_, "Test");
  for (size_t i = 0; i < 2; i++)
    EXPECT_EQ(this->lines_[i], filter.NextLine());

  EXPECT_EQ("Line 3", filter.NextLine());
}

TYPED_TEST(FiltersTest, NextLineFiltersBack) {
  TypeParam filter(this->fname_, "Test");
  for (size_t i = 0; i < 4; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ("Line 5", filter.NextLine());
}

TYPED_TEST(FiltersTest, NextLineFiltersMid) {
  TypeParam filter(this->fname_, "Test");
  for (size_t i = 0; i < 3; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ("Line 4", filter.NextLine());
}

TYPED_TEST(FiltersTest, NextLineSkipsFalsePositives) {
  TypeParam filter(this->fname_, "Test");
  auto falsepostive_answer = "Testfalse falseTest Testfalse falseTest";

  for (size_t i = 0; i < 5; i++)
    EXPECT_EQ(filter.NextLine().find("Test"), std::string::npos);

  EXPECT_EQ(falsepostive_answer, filter.NextLine());
}

