#pragma once

#include "byte_stream.hh"

#include <string>
//#include <vector>
//#include<unordered_map>
//#include <deque>
#include<list>

class Reassembler
{

private:
  uint64_t cur_captivity=1;//随便设置一个非0值就可以
  uint64_t first_unassemble_index=0;
  uint64_t first_unaccepted_index=1;//随便初始化一下
  std::list<std::pair<uint64_t,std::string>> store_substring={};
  uint64_t size_store_substring=0;
  //std::vector<char> store_char={};
  //std::vector<bool> is_ok={};
  bool receive_end=false;
  
  //uint64_t bias=0；
  //给字符的下标整体加一个偏置
  // std::deque
  // std::deque<char> stored_substrings;
  // //记录子字符串
  // std::deque<std::pair<uint64_t>> substrings_index;
  // //记录一个substring开始和结尾的index
  // uint64_t lastpush_index=0;
  // //记录最后一次push的最后一个字符的后一位的index
  // bool is_finished=false;


public:
  /*
   * Insert a new substring to be reassembled into a ByteStream.
   *   `first_index`: the index of the first byte of the substring
   *   `data`: the substring itself
   *   `is_last_substring`: this substring represents the end of the stream
   *   `output`: a mutable reference to the Writer
   *
   * The Reassembler's job is to reassemble the indexed substrings (possibly out-of-order
   * and possibly overlapping) back into the original ByteStream. As soon as the Reassembler
   * learns the next byte in the stream, it should write it to the output.
   *
   * If the Reassembler learns about bytes that fit within the stream's available capacity
   * but can't yet be written (because earlier bytes remain unknown), it should store them
   * internally until the gaps are filled in.
   *
   * The Reassembler should discard any bytes that lie beyond the stream's available capacity
   * (i.e., bytes that couldn't be written even if earlier gaps get filled in).
   *
   * The Reassembler should close the stream after writing the last byte.
   */
  void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );

  // How many bytes are stored in the Reassembler itself?
  uint64_t bytes_pending() const;
};
