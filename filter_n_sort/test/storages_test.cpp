#include <algorithm>
#include <fstream>
#include <gtest/gtest.h>

#include "../src/simple_storage.h"

using namespace app;
namespace {

template <typename T>
class StorageTest : public ::testing::Test {
 public:
  const std::string fname_ = "temp.txt";
  std::vector<std::string> lines_ =
      {"Line 1", "Line 2", "Test Line 3", "Line Test 4", "Line 5 Test", "Testfalse falseTest Testfalse Test falseTest"};

};

typedef ::testing::Types<SimpleStorage> StoragesImpl;

TYPED_TEST_CASE(StorageTest, StoragesImpl);

}

TYPED_TEST(StorageTest, OneLineTest) {
  TypeParam storage(this->fname_);
  storage.AddLine(std::string(this->lines_[0]));

  storage.Flush();

  std::ifstream file(this->fname_);
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line, this->lines_[0]);
  file.close();
}

TYPED_TEST(StorageTest, SortTest2Elemenets) {
  TypeParam storage(this->fname_);
  storage.AddLine(std::string(this->lines_[1]));
  storage.AddLine(std::string(this->lines_[0]));
  storage.Flush();

  std::ifstream file(this->fname_);
  ASSERT_TRUE(file.is_open());

  std::string line;
  std::getline(file, line);
  EXPECT_EQ(line, this->lines_[0]);
  std::getline(file, line);
  EXPECT_EQ(line, this->lines_[1]);
  file.close();
}

TYPED_TEST(StorageTest, AddAndSortTestFull) {
  TypeParam storage(this->fname_);
  for (auto &str : this->lines_)
    storage.AddLine(std::string(str));
  storage.Flush();

  std::sort(this->lines_.begin(), this->lines_.end(), [] (const std::string &lhs, const std::string &rhs) {
    auto left = lhs;
    std::transform(left.begin(), left.end(), left.begin(), ::tolower);
    auto right = rhs;
    std::transform(right.begin(), right.end(), right.begin(), ::tolower);

    return left < right;
  });

  std::ifstream file(this->fname_);
  ASSERT_TRUE(file.is_open());

  std::string line;
  for (auto &answer_line : this->lines_) {
    ASSERT_FALSE(file.eof());
    std::getline(file, line);

    EXPECT_EQ(answer_line, line);
  }
  EXPECT_TRUE(file.eof());
}

