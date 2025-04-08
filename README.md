# Live Vim 📽️

Edit files with others in real time

Live Vim is a collaborative, terminal-based editor that brings the power of Vim to real-time editing sessions. Whether you're pair programming, running a remote interview, or just want to jam with a friend on code — Live Vim makes it seamless.

## Features 📝

🧑‍💻 Vim-style Editing: Built around familiar normal, insert, and visual modes.  
🔁 Real-Time Sync: Edits are synchronized across all connected users instantly.  
🌐 Client-Server Model: One user runs the server, others connect as clients through websockets.  
📦 Minimal Dependencies: Pure C++ with Boost and ncurses; no frameworks here.  
📄 File Loading & Saving: Open local files and write back changes collaboratively.

## Collaboration Flow 👥

1. One user runs the server with a file path.
2. Others connect using the same binary with the server IP as second arg.
3. That's it! All clients see changes in real-time, like magic ✨

## Architecture

There are two processes that run when you spin up live vim:

- parent process running ncurses terminal gui
- child process managing websocket connection

The main reason for this design is that as of today, ncurses is not thread-safe. Thus, we opt for a multi-process approach to separate concerns and ensure that neither terminal or network ops block each other.

## Getting Started 🧪

### Requirements

C++17 or greater
boost 1.87 or greater
ncurses 6.5 or greater
cmake 3.31.4 or greater

`// clone this repo first  
cd live-vim  
make // builds the live-vim binary  
make test // runs unit tests (gtest)  
`
