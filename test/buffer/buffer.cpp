#include "../../src/buffer/buffer.h"
#include <gtest/gtest.h>

class BufferTest : public testing::Test {
protected:
  BufferTest() {
    test_buf.initialize("hello world");
    two_line_test_buf.initialize("hello\nworld");
  }

  buffer test_buf;
  buffer two_line_test_buf;
};

TEST_F(BufferTest, GetStringRepresentation) {
  ASSERT_EQ(test_buf.get_str_repr(), "hello world");
  ASSERT_EQ(two_line_test_buf.get_str_repr(), "hello\nworld");
}

TEST_F(BufferTest, GetNumLines) {
  ASSERT_EQ(test_buf.get_num_lines(), 1);
  ASSERT_EQ(two_line_test_buf.get_num_lines(), 2);
}

TEST_F(BufferTest, InsertionTest) {
  test_buf.insert_at(0, 0, 'h');
  ASSERT_EQ(test_buf.get_str_repr(), "hhello world");
  ASSERT_EQ(test_buf.get_num_lines(), 1);

  test_buf.insert_at(6, 0, '\n');
  ASSERT_EQ(test_buf.get_str_repr(), "hhello\n world");
  ASSERT_EQ(test_buf.get_num_lines(), 2);
}

TEST_F(BufferTest, DeletionTest) {
  two_line_test_buf.delete_at(1, 0);
  ASSERT_EQ(two_line_test_buf.get_str_repr(), "ello\nworld");
  ASSERT_EQ(two_line_test_buf.get_num_lines(), 2);

  two_line_test_buf.delete_at(0, 1);
  ASSERT_EQ(two_line_test_buf.get_str_repr(), "elloworld");
  ASSERT_EQ(two_line_test_buf.get_num_lines(), 1);
}