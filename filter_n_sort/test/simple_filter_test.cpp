#include <gtest/gtest.h>

#include "../src/simple_filter.h"

TEST(SimpleFilterTest, HasNext) {
  app::SimpleFilter filter("no-such-file.txt", "word");
  EXPECT_FALSE(filter.HasNext());
}

