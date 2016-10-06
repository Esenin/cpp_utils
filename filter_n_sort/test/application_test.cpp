#include <gtest/gtest.h>

#include "../src/application.h"

using namespace app;

namespace {

class MockFilter : public FilterBase {
 public:
  int *has_next_cnt_ = nullptr;
  int *next_line_cnt_ = nullptr;
  mutable int has_next_resource_ = 1;

  MockFilter(int *has_next_cnt, int *next_line_cnt, int resource)
      : FilterBase("dummy", "dummy"),
        has_next_cnt_(has_next_cnt),
        next_line_cnt_(next_line_cnt),
        has_next_resource_(resource) {}

  bool HasNext() const override {
    (*has_next_cnt_)++;
    return has_next_resource_-- > 0;
  }

  std::string NextLine() override {
    (*next_line_cnt_)++;
    return "dummy";
  }
};

class MockStorage : public SortedStorageBase {
 public:
  int *add_line_counter_ = nullptr;
  int *flush_hook_counter_ = nullptr;
  MockStorage(int *add_line_cnt, int *flush_cnt) : SortedStorageBase("dummy"), add_line_counter_(add_line_cnt),
      flush_hook_counter_(flush_cnt) {}

  void AddLine(std::string &&) override {
    (*add_line_counter_)++;
  }
  void FlushHook() override {
    (*flush_hook_counter_)++;
  }
};


class ApplicationTest : public ::testing::TestWithParam<int> {};

INSTANTIATE_TEST_CASE_P(InstantiationName,
                        ApplicationTest,
                        ::testing::Values(0, 1, 2));

}

TEST_P(ApplicationTest, CallFiltersMethodCounter) {
  int has_next_called_ = 0;
  int next_line_called_ = 0;
  auto filter = UniqFilter(new MockFilter(&has_next_called_, &next_line_called_, GetParam()));

  int add_line_counter = 0;
  int flush_counter = 0;
  auto storage = UniqStorage(new MockStorage(&add_line_counter, &flush_counter));

  Application app(std::move(filter), std::move(storage));

  app.Run();

  EXPECT_EQ(GetParam() + 1, has_next_called_);
  EXPECT_EQ(GetParam(), next_line_called_);

  EXPECT_EQ(GetParam(), add_line_counter);
  EXPECT_EQ(1, flush_counter);
}

