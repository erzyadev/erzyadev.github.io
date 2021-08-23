#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include <array>
#include <bitset>
// напишите эту функцию
inline bool DecodeRLE(const std::string &src_name, const std::string &dst_name)
{
    std::ifstream in(src_name, std::ios::binary | std::ios::in);
    if (!in)
        return false;

    std::ofstream out(dst_name, std::ios::binary | std::ios::out);
    if (!out)
        return false;

    static const int BUFF_SIZE = 128;
    std::array<char, BUFF_SIZE> buffer;
    for (unsigned char header = in.get(); in; header = in.get())
    {
        int block_type = (header & 1);
        int data_size = (header >> 1) + 1;
        if (block_type == 1)
        {
            char c = static_cast<char>(in.get());
            std::string char_repeated(data_size, c);
            out.write(char_repeated.data(), data_size);
        }
        else
        {
            in.read(buffer.data(), data_size);
            out.write(buffer.data(), data_size);
        }
    }
    return true;
}
