#include "../../src/io/io_utils.h"
#include <gtest/gtest.h>

class IOUtilsTest : public testing::Test {
protected:
  IOUtilsTest() {
    test_buf.initialize("hello world");
    x = 0;
    y = 0;
  }

  buffer test_buf;
  int x;
  int y;
};

TEST_F(IOUtilsTest, AdjustXTest) {
  x = -1;
  adjust_x(x, test_buf, y);
  ASSERT_EQ(x, 0);

  x = test_buf.get_str_repr().size();
  adjust_x(x, test_buf, y);
  ASSERT_EQ(x, test_buf.get_str_repr().size());
}

TEST_F(IOUtilsTest, AdjustYTest) {
  y = -1;
  adjust_y(y, test_buf);
  ASSERT_EQ(y, 0);

  y = test_buf.get_num_lines();
  adjust_y(y, test_buf);
  ASSERT_EQ(y, test_buf.get_num_lines() - 1);
}

TEST_F(IOUtilsTest, DeleteCharFromStringTest) {
  std::string test = test_buf.get_str_repr();
  int prev_length = test.size();
  delete_char_back(test);
  ASSERT_EQ(test, test.substr(0, prev_length - 1));
}

TEST_F(IOUtilsTest, PrintTest) {
  std::stringstream buffer;
  std::streambuf *old_cout = std::cout.rdbuf(buffer.rdbuf());

  print("hello world");

  std::cout.rdbuf(old_cout);

  EXPECT_EQ(buffer.str(), "hello world\n");
}