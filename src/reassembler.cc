#include "reassembler.hh"
#include <string>
using namespace std;

void Reassembler::insert( uint64_t first_index, string data, bool is_last_substring, Writer& output )
{
  // Your code here.
  //(void)first_index;
  //(void)data;
  //(void)is_last_substring;
  //(void)output;
  cur_captivity=output.available_capacity();
  //return之前都要检查一下子字符串有没有结束
  if(cur_captivity==0)
  {
    if(data.length()==0&&is_last_substring==true)
    {
      receive_end=true;
    }
    if(receive_end==true&&size_store_substring==0)
    {
      output.close();
    }
    return;
  }
  
  uint64_t start_index = first_index;
  uint64_t end_index = start_index+data.length();
  first_unaccepted_index=first_unassemble_index+cur_captivity;

  //这个字符串已经送到stream里面了
  if(end_index<=first_unassemble_index)
  {
    if(is_last_substring==true)
    {
      receive_end=true;
    }
    if(receive_end==true&&size_store_substring==0)
    {
      output.close();
    }
    return;
  }
  //收到了最后一个子字符串，并且完整装到了缓存里面才能receive end true
  if(end_index<=first_unaccepted_index&&(is_last_substring==true))receive_end=true;


  //if(end_index>first_unaccepted_index)
  
  //int begin_position;
  //int end_position;
  if(start_index<=first_unassemble_index)start_index=first_unassemble_index;
  auto it = store_substring.begin();
  //要找到插入的位置，同时还要裁剪一下避免重复，也就是确定开始和结束的字符串index
  while(it!=store_substring.end())
  {
    if(start_index<it->first)
    {
      if(end_index<=it->first)break;//无重叠，找到了it，直接在it前插入
      else if (end_index > it->first&&end_index < it->first+it->second.length())//部分重叠，裁剪掉多余的部分
      {
        end_index=it->first;
      }
      else if(end_index >= it->first+it->second.length())//完全覆盖,直接删除被覆盖的字符串
      {
        size_store_substring -= it->second.length();
        it=store_substring.erase(it);
        continue;
      }
    }

    else if(start_index >= it->first && start_index <= it->first+it->second.length())
    {
      if(end_index <= it->first+it->second.length())//当前的字符串和缓存里面的字符串重叠了，直接抛弃
      {
        if(is_last_substring==true)
        {
          receive_end=true;
        }
        if(receive_end==true&&size_store_substring==0)
        {
          output.close();
        }
        return;
      }
      else if(end_index > it->first+it->second.length())//部分重叠，而且需要插入在it之后
      {
        start_index=it->first + it->second.length();
        it++;
        continue;
      }
    }

    else if(start_index > it->first + it->second.length())
    {
      it++;
    }
  }
  
  if(end_index>first_unaccepted_index)end_index = first_unaccepted_index;
  //已经找到了无重叠的start_index和end_index，而且找到了it，在it前面插入
  string cut_data=data.substr(start_index-first_index, end_index-start_index);
  store_substring.insert(it,make_pair(start_index, cut_data));
  size_store_substring += cut_data.length();

  while(store_substring.empty()!=1)
  {
    if(store_substring.front().first==first_unassemble_index)
    {
      auto push_string = store_substring.front();
      size_store_substring -= push_string.second.length();
      output.push(store_substring.front().second);
      first_unassemble_index +=push_string.second.length();

      store_substring.pop_front();
    }
    else break;
  }
  
  if(store_substring.empty()&&receive_end==true)
  {
    output.close();
    return;
  }
}

uint64_t Reassembler::bytes_pending() const
{
  // Your code here.
  return {size_store_substring};
}
