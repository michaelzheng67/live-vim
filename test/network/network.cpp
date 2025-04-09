#include "../../src/network/network.h"

#include <boost/interprocess/ipc/message_queue.hpp>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <optional>
#include <thread>

using namespace boost::interprocess;

class NetworkTest : public testing::Test {
protected:
  NetworkTest() {

    message_queue::remove(TO_SEND);
    message_queue::remove(TO_RECEIVE);

    send_q.emplace(create_only, TO_SEND, 100, sizeof(packet));
    receive_q.emplace(create_only, TO_RECEIVE, 100, sizeof(packet));

    std::filesystem::remove(LOG_PATH);
    std::ofstream log_file(LOG_PATH);
    if (!log_file.is_open()) {
      throw std::runtime_error("Failed to open log file: " +
                               std::string(LOG_PATH));
    }
    log_file << "";
    log_file.close();
  }

  const char *TO_SEND = "to_send";
  const char *TO_RECEIVE = "to_receive";
  char *LOG_PATH =
      "./test/network/test_data/log.txt"; // make file is in top-level directory
  std::optional<message_queue> send_q;
  std::optional<message_queue> receive_q;

  void TearDown() override {
    message_queue::remove(TO_SEND);
    message_queue::remove(TO_RECEIVE);
    std::filesystem::remove(LOG_PATH);
  }
};

TEST_F(NetworkTest, ConnectionTest) {

  std::thread server_thread([&]() {
    server_loop(8002, send_q.value(), receive_q.value(), LOG_PATH);
  });

  std::thread client_thread([&]() {
    client_loop("127.0.0.1", 8002, send_q.value(), receive_q.value(), LOG_PATH);
  });

  std::this_thread::sleep_for(std::chrono::milliseconds(500));

  packet shutdown_packet{};
  shutdown_packet.operation = SHUTDOWN;

  send_q.value().send(&shutdown_packet, sizeof(shutdown_packet), 0);
  send_q.value().send(&shutdown_packet, sizeof(shutdown_packet), 0);

  std::this_thread::sleep_for(std::chrono::milliseconds(200));

  client_thread.join();

  server_thread.join();

  std::fstream ifile(LOG_PATH);
  std::ostringstream ss;
  ss << ifile.rdbuf();

  std::this_thread::sleep_for(std::chrono::milliseconds(50));

  ASSERT_EQ(ss.str(), "ws connection made\n");
}