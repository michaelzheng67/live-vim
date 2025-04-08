#include "network.h"
#include "../util/utils.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/websocket.hpp>

#include <atomic>
#include <fstream>
#include <iostream>
#include <thread>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

void do_session(tcp::socket socket, message_queue &send_q,
                message_queue &receive_q, std::atomic<bool> &stop_server,
                std::shared_ptr<net::io_context> ioc) {

#ifdef DEBUG
  std::ofstream ofile("./data/output.txt", std::ios::app);
#else
  std::shared_ptr<std::ofstream> ofile = nullptr;
#endif

  websocket::stream<tcp::socket> ws(std::move(socket));
  ws.accept();

  // async fetches packets from peer in background
  std::function<void(message_queue &, websocket::stream<tcp::socket> &)>
      get_packet;
  get_packet = [&](message_queue &receive_q,
                   websocket::stream<tcp::socket> &ws) {
    auto buf = std::make_shared<boost::beast::flat_buffer>();
    ws.async_read(*buf, [buf, &receive_q, &ws, &ofile,
                         &get_packet](boost::system::error_code ec,
                                      std::size_t bytes_transferred) {
      if (!ec) {
        packet incoming;
        std::memcpy(&incoming, buf->data().data(), sizeof(packet));

        receive_q.send(&incoming, sizeof(packet), 0);

        buf->consume(buf->size());
        get_packet(receive_q, ws);
      } else {
#ifdef DEBUG
        ofile << "async_read error: " << ec.message() << std::endl;
#endif
      }
    });
  };

  get_packet(receive_q, ws);

  std::thread io_thread([ioc]() { ioc->run(); });

#ifdef DEBUG
  ofile << "ws connection made" << std::endl;
#endif

  while (true) {

    message_queue::size_type recvd_size;
    unsigned int priority;
    packet outgoing;

    // outgoing changes
    if (send_q.try_receive(&outgoing, sizeof(outgoing), recvd_size, priority)) {

      switch (outgoing.operation) {
      case SHUTDOWN:
        goto exit;

      default:
        ws.write(net::buffer(&outgoing, sizeof(outgoing)));
#ifdef DEBUG
        ofile << "server sending" << std::endl;
#endif
      }
    }
  }
exit:

#ifdef DEBUG
  ofile.close();
#endif
  ws.close(websocket::close_code::normal);
  io_thread.join();
  stop_server = true;
}

void server_loop(message_queue &send_q, message_queue &receive_q) {

  auto ioc = std::make_shared<net::io_context>();
  tcp::acceptor acceptor(*ioc, tcp::endpoint(tcp::v4(), SERVER_PORT));

  std::atomic<bool> stop_server = false;
  while (!stop_server) {
    tcp::socket socket(*ioc);
    acceptor.accept(socket);
    std::thread(&do_session, std::move(socket), std::ref(send_q),
                std::ref(receive_q), std::ref(stop_server), ioc)
        .detach();
  }
  ioc->stop();
}

void client_loop(const char *server_ip, message_queue &send_q,
                 message_queue &receive_q) {

#ifdef DEBUG
  std::ofstream ofile("./data/output.txt", std::ios::app);
#else
  std::shared_ptr<std::ofstream> ofile = nullptr;
#endif

  // setup connection
  net::io_context ioc;
  tcp::resolver resolver{ioc};
  websocket::stream<tcp::socket> ws{ioc};
  auto const results = resolver.resolve(SERVER_IP, std::to_string(SERVER_PORT));
  net::connect(ws.next_layer(), results);
  ws.handshake(SERVER_IP, "/");

  // async fetches packets from peer in background
  std::function<void(message_queue &, websocket::stream<tcp::socket> &)>
      get_packet;
  get_packet = [&](message_queue &receive_q,
                   websocket::stream<tcp::socket> &ws) {
    auto buf = std::make_shared<boost::beast::flat_buffer>();
    ws.async_read(*buf, [buf, &receive_q, &ws, &ofile,
                         &get_packet](boost::system::error_code ec,
                                      std::size_t bytes_transferred) {
      if (!ec) {
        packet incoming;
        std::memcpy(&incoming, buf->data().data(), sizeof(packet));

        receive_q.send(&incoming, sizeof(packet), 0);

        buf->consume(buf->size());
        get_packet(receive_q, ws);
      } else {
#ifdef DEBUG
        ofile << "async_read error: " << ec.message() << std::endl;
#endif
      }
    });
  };

  get_packet(receive_q, ws);

  std::thread io_thread([&ioc]() { ioc.run(); });

  while (true) {

    message_queue::size_type recvd_size;
    unsigned int priority;
    packet outgoing;

    // outgoing changes
    if (send_q.try_receive(&outgoing, sizeof(outgoing), recvd_size, priority)) {

      switch (outgoing.operation) {
      case SHUTDOWN:
        goto exit;

      default:
        ws.write(net::buffer(&outgoing, sizeof(outgoing)));
#ifdef DEBUG
        ofile << "client sending" << std::endl;
#endif
      }
    }
  }

exit:

#ifdef DEBUG
  ofile.close();
#endif
  ws.close(websocket::close_code::normal);
  io_thread.join();
}