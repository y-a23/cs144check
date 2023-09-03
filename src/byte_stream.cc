#include <stdexcept>

#include "byte_stream.hh"
#include <string_view>
using namespace std;

ByteStream::ByteStream( uint64_t capacity ) : capacity_( capacity ) {}

void Writer::push( string data )
{
  for(auto a:data)
  {
    if(bytebuffer.size()>=capacity_)break;
    bytebuffer.push(a);
    
    //cur_size++;
  }
  // Your code here.
  //(void)data;
}

void Writer::close()
{
  is_close=true;
  // Your code here.
}

void Writer::set_error()
{
  is_error=true;
  // Your code here.
}

bool Writer::is_closed() const
{
  // Your code here.
  return {is_close};
}

uint64_t Writer::available_capacity() const
{
  // Your code here.
  return {capacity_-bytebuffer.size()};
}

uint64_t Writer::bytes_pushed() const
{
  // Your code here.
  return {bytebuffer.size()+total_pop};
}

string_view Reader::peek() const
{
  // Your code here.
  std::string_view myview(&bytebuffer.front(),1);
  return {myview};
}

bool Reader::is_finished() const
{
  // Your code here.
  return {is_close&&(bytebuffer.size()==0)};
}

bool Reader::has_error() const
{
  // Your code here.
  return {is_error};
}

void Reader::pop( uint64_t len )
{
  // Your code here.
  (void)len;
  for(uint64_t i=0;i<len;i++)
  {
    bytebuffer.pop();
    total_pop++;
  }
}

uint64_t Reader::bytes_buffered() const
{
  // Your code here.
  return {bytebuffer.size()};
}

uint64_t Reader::bytes_popped() const
{
  // Your code here.
  return {total_pop};
}
