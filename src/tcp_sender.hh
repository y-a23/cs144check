#pragma once

#include "byte_stream.hh"
#include "tcp_receiver_message.hh"
#include "tcp_sender_message.hh"
#include <queue>
class TCPSender
{
  Wrap32 isn_;
  uint64_t initial_RTO_ms_;

  bool syn_send_{false};
  bool fin_send_{false};

  uint64_t count_outstand_{0};
  uint16_t windowsize_ {1};
  uint64_t ab_ackno_{0};
  std::queue<TCPSenderMessage> buf_ready_send_{};//用队列装msg，删除插入都比较方便
  //uint64_t num_ready_send_{0};

  std::queue<TCPSenderMessage> buf_send_not_ack_{};//outstanding data
  //uint64_t num_not_ack_{0};
  uint64_t next_ab_seqno_{0};
  uint64_t count_retrans{0};

  struct Timer
  {
    bool isrunnning;
    uint64_t cur_tick_time;
    uint64_t cur_RTO_ms;
  };
  struct Timer timer={false,0,initial_RTO_ms_};
  

public:
  /* Construct TCP sender with given default Retransmission Timeout and possible ISN */
  TCPSender( uint64_t initial_RTO_ms, std::optional<Wrap32> fixed_isn );

  /* Push bytes from the outbound stream */
  void push( Reader& outbound_stream );

  /* Send a TCPSenderMessage if needed (or empty optional otherwise) */
  std::optional<TCPSenderMessage> maybe_send();

  /* Generate an empty TCPSenderMessage */
  TCPSenderMessage send_empty_message() const;

  /* Receive an act on a TCPReceiverMessage from the peer's receiver */
  void receive( const TCPReceiverMessage& msg );

  /* Time has passed by the given # of milliseconds since the last time the tick() method was called. */
  void tick( uint64_t ms_since_last_tick );

  /* Accessors for use in testing */
  uint64_t sequence_numbers_in_flight() const;  // How many sequence numbers are outstanding?
  uint64_t consecutive_retransmissions() const; // How many consecutive *re*transmissions have happened?
};
