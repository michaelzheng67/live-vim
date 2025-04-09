# Live Vim 📽️

Edit files with others in real time

Live Vim is a collaborative, terminal-based editor that brings the power of Vim to real-time editing sessions. Whether you're pair programming, running a remote interview, or just want to jam with a friend on code — Live Vim makes it seamless.

## Features 📝

🧑‍💻 Vim-style Editing: Built around familiar normal, insert, and visual modes.  
🔁 Real-Time Sync: Edits are synchronized across all connected users instantly.  
🌐 Client-Server Model: One user runs the server, others connect as clients through websockets.  
📦 Minimal Dependencies: Pure C++ with Boost and ncurses; no frameworks here.  
📄 File Loading & Saving: Open local files and write back changes collaboratively.

## Demo

https://github.com/user-attachments/assets/7873f2e1-6c36-4c29-a7b5-5d4b4c109b5d

## Collaboration Flow 👥

1. One user runs the server process opened up to a specific file they wish to edit
2. Others connect to the server process by indicating server IP and port, while also making sure they open the same file
3. That's it! Everyone can edit and see changes in real-time, like magic ✨

## Architecture

There are two processes that run when you spin up live vim:

- parent process running ncurses terminal gui
- child process managing websocket connection

The main reason for this design is that as of today, ncurses is not thread-safe. Thus, we opt for a multi-process approach to separate concerns and ensure that neither terminal or network ops block each other.

Packets are sent over the wire with a specific structure. Currently, there are two highlighted actions you can take: insertion and deletion. These operations are sent
on each edit and applied to the local copies of the file for each user. Given we are using a websocket connnection with TCP as the underlying network layer, we can
ensure that changes will reach each client at least once due to re-transmissions. Furthermore, websockets are a more "intuitive" fit for two-way communication as
opposed to a request - response based model, and don't require new connections on each transmission (older versions of http) and lower header overhead.

File content is essentially represented as a vector of strings, indexed by the row they belong to. This helps wit write contention compared to a naive buffer
implementation, as edits on two separate rows will not affect each other, and the only contention is per line. On each edit to file content, the vector is shrunk to use
only the necessary memory to hold exactly the rows of content present, thus reducing memory footprint.

## Getting Started 🧪

### Requirements

C++17 or greater  
boost 1.87 or greater  
ncurses 6.5 or greater  
cmake 3.31.4 or greater

Playing with the source code:

```
// clone this repo first
cd live-vim
make // builds the live-vim binary
make test // runs unit tests (gtest)
```

To compile binary and run in terminal:

```
chmod +x configure.sh
./configure.sh // compiles + places binary into usr/local/bin. likely requires sudo
```

Then you can run live-vim:

```
(base) MacBook-Pro-4:bin michaelzheng$ live-vim -h
Vim with your friends 📝


live-vim [OPTIONS]


OPTIONS:
  -h,     --help              Print this help message and exit
...
```
