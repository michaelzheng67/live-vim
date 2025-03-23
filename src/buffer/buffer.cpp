#include <fstream>

#include "buffer.h"

void create_buf(const std::string& path) {
    std::fstream ifile(path, std::ios::binary|std::ios::in|std::ios::out);
    BUF.assign(std::istreambuf_iterator<char>(ifile), std::istreambuf_iterator<char>());
    ifile.close();
}

void write_out_buf(const std::string& path, const std::string& buf) {
    std::ofstream ofile(path, std::ios::out | std::ios::trunc | std::ios::binary);
    ofile.write(buf.c_str(), buf.size());
    ofile.close();
}