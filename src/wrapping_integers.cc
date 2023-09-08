#include "wrapping_integers.hh"
//#include<algorithm>
using namespace std;

Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  // Your code here.
  //(void)n;
  //(void)zero_point;
  //uint32_t raw_value = zero_point + static_cast<uint32_t>n;


  return (zero_point + static_cast<uint32_t>(n));
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Your code here.
  //(void)zero_point;
  //(void)checkpoint; 
  //uint64_t all_1=0x 0000 0000 FFFF FFFF;
  //uint64_t num = checkpoint/(static<uint64_t>all_1);
  //Wrap32 wrap_checkpoint = wrap(checkpoint, zero_point);
  

  uint64_t num = checkpoint >> 32;
  
  //uint64_t absolute_seqno=0;
  //uint32_t all_1=0xFFFFFFFF;
  uint32_t zero2raw_distance = this->get_raw_value() - zero_point.get_raw_value() ;
  //zero2raw_distance = static_cast<uint64_t> (zero2raw_distance);

  //转换为64之后，在checkpoint附近有三个长度为2^32的段，每个段都有一个点，我把这三个点都检查一遍，看哪个点的距离最近
  uint64_t ans2=(num<<32) + zero2raw_distance;
  uint64_t distance2;
  if(ans2>=checkpoint)distance2 = ans2-checkpoint;
  else distance2 = checkpoint - ans2;

  //这里要判断一下num能不能减一
  uint64_t ans1=((num-1)<<32) + zero2raw_distance;
  if(num==0) ans1=ans2;
  uint64_t distance1 = checkpoint - ans1;

  uint64_t ans3=((num+1)<<32) + zero2raw_distance;
  uint64_t distance3 = ans3 - checkpoint;

  uint64_t mindis ;//= min(distance1, min(distance2, distance3));
  mindis = distance2>distance3? distance3:distance2;
  mindis = mindis>distance1? distance1:mindis;

  if(mindis==distance1)return ans1;
  else if(mindis==distance2)return ans2;
  else  return ans3;
}
