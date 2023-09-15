#include "tcp_sender.hh"
#include "tcp_config.hh"

#include <random>
#include<algorithm>
using namespace std;

/* TCPSender constructor (uses a random ISN if none given) */
TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
  : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
{}

uint64_t TCPSender::sequence_numbers_in_flight() const
{
  //这里有点搞不清楚outstandingdata的定义是什么，应该是只要push了就算outstanding，我刚开始以为要send但是没有ack才算
  return count_outstand_;
  // Your code here.
  //return {};
}

uint64_t TCPSender::consecutive_retransmissions() const
{
  // Your code here.
  return {count_retrans};
}

optional<TCPSenderMessage> TCPSender::maybe_send()
{
  if(buf_ready_send_.empty()==true)return{};
  if(timer.isrunnning==false)
  {
    timer.isrunnning=true;
    timer.cur_tick_time=0;
  }

  TCPSenderMessage sendmsg= buf_ready_send_.front();
  buf_ready_send_.pop();
  buf_send_not_ack_.push(sendmsg);
  //num_not_ack_ += sendmsg.sequence_length();
  //num_ready_send_ -= sendmsg.sequence_length();
  // Your code here.
  return {sendmsg};
}

void TCPSender::push( Reader& outbound_stream )
{
  // 有一个测试样例是在发送syn之前收到了一个ack，所以已经知道了window的大小，这里的代码改了很久
  // if(syn_send_==false)//现在不知道window大小，当做window的大小是1，只发送一个syn（错了）
  // {
  //   syn_send_=true;
  //   TCPSenderMessage syn_msg;
  //   syn_msg.SYN = true;
  //   syn_msg.FIN = false;
  //   syn_msg.seqno = isn_;
  //   buf_ready_send_.push(syn_msg);

  //   count_outstand_++;
  //   //num_ready_send_++;
  //   next_ab_seqno_++;
  //   return;
  // }
  if(fin_send_==true)return;
  if(buf_send_not_ack_.empty()==false)
  {
    if(syn_send_==true&&buf_send_not_ack_.front().SYN==true)return;
    //if(syn_send_==true)return;
  }
  //syn发送但是还没确认

  uint temp_window_size = windowsize_==0? 1:windowsize_;
  if(temp_window_size>0)
  {
    while(count_outstand_ < temp_window_size)
    {
      TCPSenderMessage msg;
      if(syn_send_==false)
      {
        syn_send_=true;
        msg.SYN=true;
        count_outstand_++;
      }
      msg.seqno = Wrap32::wrap(next_ab_seqno_, isn_);
      uint64_t msglenth = min(TCPConfig::MAX_PAYLOAD_SIZE, temp_window_size - count_outstand_);
      uint64_t streamlen = outbound_stream.bytes_buffered();
      msglenth = min(msglenth, streamlen);
      read(outbound_stream, msglenth, msg.payload);
      //num_ready_send_ += msglenth;
      count_outstand_ +=msglenth;
      

      if(!fin_send_ && outbound_stream.is_finished()==true && count_outstand_<temp_window_size)
      {
        msg.FIN=true;
        fin_send_=true;
        //num_ready_send_++;
        count_outstand_++;
        //msglenth++;
      }
      

      if(msg.sequence_length()==0)break;
      else
      {
        buf_ready_send_.push(msg);
        next_ab_seqno_+=msg.sequence_length();
        //if(msg.FIN==true)
      }

      //if(msg.FIN || outbound_stream.bytes_buffered()==0)break;
      //我认为不加这个break也能行，但是最后测试的时候发现序列号会有问题
    }
    return;
  }
  // // Your code here.
  // //(void)outbound_stream;
  // uint64_t num_stream = outbound_stream.bytes_buffered();
  // for(uint64_t i = 0 ; i<num_stream; i+=TCPConfig::MAX_PAYLOAD_SIZE)
  // {
  //   TCPSenderMessage temp;
  //   temp.seqno = Wrap32::wrap(ackno_+i);
  //   temp.payload = 
  // }
}

TCPSenderMessage TCPSender::send_empty_message() const
{
  //TCPSenderMessage msg;
  Wrap32 seqno = Wrap32::wrap(next_ab_seqno_,isn_);
  //msg.FIN=false;
  //msg.SYN=false;
  // Your code here.
  return {seqno, false, {}, false};
}

void TCPSender::receive( const TCPReceiverMessage& msg )
{
  windowsize_ = msg.window_size;
  if(msg.ackno.has_value()==false)return;
  uint64_t checkpoint = next_ab_seqno_ - (count_outstand_)/2;
  //收到的ackno应该是在next_ab_seqno_到next_ab_seqno_ - count_stand之间，所以我这里checkpoint取了一个中间值
  uint64_t rec_ab_ackno = msg.ackno.value().unwrap(isn_, checkpoint);
  //这里要求收到的ackno是可能出现的，也就是不能大于next seqno
  //ackno更新了才重启timer
  if(rec_ab_ackno>ab_ackno_ && rec_ab_ackno<=next_ab_seqno_)
  {
    ab_ackno_ = rec_ab_ackno;
    timer.cur_RTO_ms = initial_RTO_ms_;
    timer.cur_tick_time = 0;
    timer.isrunnning=false;
  }
  
  
  
  //if(buf_send_not_ack_.empty()==false)timer.isrunnning=true;
  count_retrans = 0;
  //更新ackno之后要将“发送未确认segment”的队列pop一下
  while(buf_send_not_ack_.empty()==false)
  {
    TCPSenderMessage tempmsg = buf_send_not_ack_.front();
    if(tempmsg.seqno.unwrap(isn_, checkpoint) + tempmsg.sequence_length() <= ab_ackno_)
    {
      //num_not_ack_ -= tempmsg.sequence_length();
      if(count_outstand_>=tempmsg.sequence_length())count_outstand_ -=tempmsg.sequence_length();
      buf_send_not_ack_.pop();
    }
    else break;
  }
  //还有outstandding的数据，启动timer
  if(count_outstand_!=0)
  {
    timer.isrunnning=true;
  }

  // Your code here.
  //(void)msg;
}

void TCPSender::tick( const size_t ms_since_last_tick )
{
  if(timer.isrunnning)
  timer.cur_tick_time += ms_since_last_tick;
  if(timer.cur_tick_time >= timer.cur_RTO_ms)
  {
    timer.cur_tick_time =0;
    buf_ready_send_.push(buf_send_not_ack_.front());
    if(windowsize_!=0)
    {
      timer.cur_RTO_ms *=2;
      count_retrans++;
    }
  }
  return;
  // Your code here.
  //(void)ms_since_last_tick;
}
