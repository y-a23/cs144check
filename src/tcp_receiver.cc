#include "tcp_receiver.hh"
#include<optional>
using namespace std;

void TCPReceiver::receive( TCPSenderMessage message, Reassembler& reassembler, Writer& inbound_stream )
{
  // Your code here.
  //(void)message;
  //(void)reassembler;
  //(void)inbound_stream;
  if(receive_syn==false&&message.SYN==false)return;//没有syn直接返回
  if(message.SYN)
  {
    SYN_seqno = message.seqno;
    message.seqno = message.seqno + 1;//去掉syn位
    receive_syn=true;
  }
  uint64_t stream_index = message.seqno.unwrap(SYN_seqno, inbound_stream.bytes_pushed()+1) - 1;
  reassembler.insert(stream_index, message.payload.release(), message.FIN, inbound_stream);
  return;
}

TCPReceiverMessage TCPReceiver::send( const Writer& inbound_stream ) const
{
  // Your code here.
  //(void)inbound_stream;
  //uint64_t ack_stream_index = inbound_stream.bytes_pushed() + 1;
  //uint64_t ack_absolute_seqno = ack_stream_index + 1;
  TCPReceiverMessage sendmessage{};
  sendmessage.window_size = inbound_stream.available_capacity()>UINT16_MAX? UINT16_MAX:inbound_stream.available_capacity();
  //if(sendmessage.window_size>=UINT16_MAX) sendmessage.window_size=UINT16_MAX;

  if(receive_syn==false)return sendmessage;
  else
  {
    uint64_t ack_absolute_seqno = inbound_stream.bytes_pushed() + 1 + inbound_stream.is_closed();
    Wrap32 ackno = Wrap32::wrap(ack_absolute_seqno, SYN_seqno);
    sendmessage.ackno.emplace(ackno);
  }

  return sendmessage;
}
