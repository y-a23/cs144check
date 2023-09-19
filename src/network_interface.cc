#include "network_interface.hh"

#include "arp_message.hh"
#include "ethernet_frame.hh"

using namespace std;

// ethernet_address: Ethernet (what ARP calls "hardware") address of the interface
// ip_address: IP (what ARP calls "protocol") address of the interface
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  //以太网帧可以携带Internetdatagram，也可以携带arpmessage
  //(void)dgram;
  //(void)next_hop;
  uint32_t target_ip = next_hop.ipv4_numeric();
  //uint32_t source_address = dgram.header.src;
  //uint32_t destination_address = dgram.header.dst;
  //发送到next_hop, 而不是dgram的dst
  auto it = map_ip2mac_.find(target_ip);

  //nexthop的以太网地址已知，封装dgram发送
  if(it!=map_ip2mac_.end())
  {
    EthernetFrame send_frame
    {{it->second.first, ethernet_address_, EthernetHeader::TYPE_IPv4},serialize(dgram)};
    //send_frame.header.dst = it->second.first;
    send_frame_.push(send_frame);
    return;
  }

  //nexthop的以太网地址未知，如果没发过arp就发request arp，把dgram缓存起来
  if(map_arp_time_.find(target_ip)==map_arp_time_.end())
  {
    ARPMessage arp_send;
    arp_send.opcode = ARPMessage::OPCODE_REQUEST;
    arp_send.sender_ethernet_address = ethernet_address_;
    arp_send.sender_ip_address = ip_address_.ipv4_numeric();
    arp_send.target_ip_address = target_ip;
    EthernetFrame send_frame{{ETHERNET_BROADCAST, ethernet_address_, EthernetHeader::TYPE_ARP}, serialize(arp_send)};
    send_frame_.push(send_frame);
    map_arp_time_.insert(make_pair(target_ip, 0));//发完arp之后加入到map里面记录下来
  }
  //缓存dgram
  auto it_store_dgram = store_dgram_.find(target_ip);
  if(it_store_dgram == store_dgram_.end())
  {
    std::queue<InternetDatagram> q_dgram;
    store_dgram_.insert(make_pair(target_ip, q_dgram));
    //store_dgram_[target_ip].push(dgram);
  }
  store_dgram_[target_ip].push(dgram);
  return;
}

// frame: the incoming Ethernet frame
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  if(frame.header.dst!=ethernet_address_ && frame.header.dst!=ETHERNET_BROADCAST)
  return{};//有点疑惑为什么会出现这种发错的情况，但是测试样例里面有一个这样的例子
  //(void)frame;
  //收到arp，两种情况，收到的是request或者reply
  if(frame.header.type==EthernetHeader::TYPE_ARP)
  {
    ARPMessage rec_arp;
    parse(rec_arp, frame.payload);
    map_ip2mac_.insert(make_pair(rec_arp.sender_ip_address, make_pair(rec_arp.sender_ethernet_address, 0)));
    //收到的是request，如果request的是自己的以太网地址，就发送一个reply的arpmessage，发送给request方
    if(rec_arp.opcode==ARPMessage::OPCODE_REQUEST)
    {
      if(rec_arp.target_ip_address == ip_address_.ipv4_numeric())
      {
        ARPMessage arp_reply;
        arp_reply.opcode = ARPMessage::OPCODE_REPLY;
        arp_reply.sender_ethernet_address = ethernet_address_;
        arp_reply.sender_ip_address = ip_address_.ipv4_numeric();
        arp_reply.target_ethernet_address = rec_arp.sender_ethernet_address;
        arp_reply.target_ip_address = rec_arp.sender_ip_address;

        EthernetFrame reply_frame
        {{rec_arp.sender_ethernet_address, ethernet_address_, EthernetHeader::TYPE_ARP},serialize(arp_reply)};
        send_frame_.push(reply_frame);
        return{};
      }
      else return{};
    }

    //收到arp之后，无论arp是reply还是request都检查一下有没有可以发送的dgram
    auto it = store_dgram_.find(rec_arp.sender_ip_address);
    if(it!=store_dgram_.end())
    {
      while(it->second.empty()==false)
      {
        InternetDatagram dgram = it->second.front();
        it->second.pop();
        EthernetFrame send_frame{{rec_arp.sender_ethernet_address, ethernet_address_,EthernetHeader::TYPE_IPv4},serialize(dgram)};
        send_frame_.push(send_frame);
      }
      if(it->second.empty()) store_dgram_.erase(it);
    }
  }

  //frame携带的是internetdatagram
  else if(frame.header.type==EthernetHeader::TYPE_IPv4)
  {
    InternetDatagram dgram;
    parse(dgram, frame.payload);
    return dgram;
  }
  return {};
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  for(auto it = map_ip2mac_.begin();it!=map_ip2mac_.end();)
  {
    it->second.second += ms_since_last_tick;
    if(it->second.second >= 30000)it=map_ip2mac_.erase(it);
    else it++;
  }
  
  for(auto it = map_arp_time_.begin(); it!=map_arp_time_.end();)
  {
    it->second += ms_since_last_tick;
    if(it->second>=5000) it=map_arp_time_.erase(it);
    else it++;
  }

  //for(auto it = map)
  //(void)ms_since_last_tick;
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if(!send_frame_.empty())
  {
    auto frame = send_frame_.front();
    send_frame_.pop();
    return {frame};
  }
  else return{};
}
