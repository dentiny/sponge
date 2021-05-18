#include "tcp_sender.hh"

#include "tcp_config.hh"

#include <random>

// Dummy implementation of a TCP sender

// For Lab 3, please replace with a real implementation that passes the
// automated checks run by `make check_lab3`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! \param[in] capacity the capacity of the outgoing byte stream
//! \param[in] retx_timeout the initial amount of time to wait before retransmitting the oldest outstanding segment
//! \param[in] fixed_isn the Initial Sequence Number to use, if set (otherwise uses a random ISN)
TCPSender::TCPSender(const size_t capacity, const uint16_t retx_timeout, const std::optional<WrappingInt32> fixed_isn) :
  isn_(fixed_isn.value_or(WrappingInt32{random_device()()})),
  initial_retransmission_timeout_{retx_timeout},
  stream_(capacity),
  consecutive_retransmissions_(0),
  window_size_(1),
  latest_abs_ackno_(0),
  timer_starts_(false),
  timer_countdown_(initial_retransmission_timeout_),
  RTO_(initial_retransmission_timeout_),
  bytes_in_flight_(0) {}

/*
 * 1. Reads from its input ByteStream and sends as many bytes as possible in the form of
 * TCPSegments, as long as there are new bytes to be read and space available in the window.
 * 2. Make each individual TCPSegment as big as possible, but no bigger than the value given
 * by TCPConfig::MAX PAYLOAD SIZE.
 * 3. SYN and FIN flags also occupy a sequence number each, which means that they occupy space
 * in the window.
 * 4. If the receiver has announced a window size of zero, the fill window method should act
 * like the window size is one.
 */
void TCPSender::fill_window() {

}

//! \param ackno The remote receiver's ackno (acknowledgment number)
//! \param window_size The remote receiver's advertised window size
/*
 * 1. Check whether ack valid(ackno_64 < next_acknp_64).
 * 2. Update window size and window edge for future segment sending.
 * 3. Check whether the ack has been handled before.
 * 4. ACK an unacknowledged segment.
 * (1) Initialize retransmission-related bookkeeping.
 * (2) Deal with bytes in flight.
 */
void TCPSender::ack_received(const WrappingInt32 ackno, const uint16_t window_size) {
  // Check whether ack valid(ackno_64 < next_acknp_64).
  uint64_t abs_ack_seqno64 = unwrap(ackno, isn_, next_seqno_);
  if (abs_ack_seqno64 >= next_seqno_) {
    return;
  }

  // Update window size for future segment sending.
  window_size_ = window_size;

  // Check whether the ack has been handled before.
  if (abs_ack_seqno64 <= latest_abs_ackno_) {
    return;
  }

  // ACK an unacknowledged segment.
  // (1) Initialize retransmission-related bookkeeping.
  consecutive_retransmissions_ = 0;
  timer_countdown_ = initial_retransmission_timeout_;
  RTO_ = initial_retransmission_timeout_;

  // (2) Deal with bytes in flight.
  uint64_t acked_bytes = abs_ack_seqno64 - latest_abs_ackno_;
  bytes_in_flight_ -= acked_bytes;
  while (!flying_segments_.empty()) {
    TCPSegment& flying_segment = flying_segments_.front();
    uint64_t cur_ack_seqno = unwrap(flying_segment.header().seqno, isn_, next_seqno_);
    latest_abs_ackno_ = abs_ack_seqno64;
    if (cur_ack_seqno + flying_segment.length_in_sequence_space() <= abs_ack_seqno64) {  // the segment is fully acked
      flying_segments_.pop();
    } else {
      break;
    }
  }
  if (flying_segments_.empty()) {
    timer_starts_ = false;
  }
}

//! \param[in] ms_since_last_tick the number of milliseconds since the last call to this method
// 1. Retransmit the earliest (lowest sequence number) segment that hasn’t been fully
// acknowledged by the TCP receiver.
// 2. If the window size is nonzero:
// (1) Increment consecutive retransmissions #
// (2) exponential backoff: double the value of RTO
// 3. Reset the retransmission timer 
void TCPSender::tick(const size_t ms_since_last_tick) {
  if (!timer_starts_) {
    return;
  }

  timer_countdown_ -= ms_since_last_tick;
  if (timer_countdown_ > 0) {
    return;
  }

  segments_out_.push(flying_segments_.front());
  if (window_size_ > 0) {
    ++consecutive_retransmissions_;
    RTO_ *= 2;
  }
  timer_countdown_ = RTO_;
}

// For TCPConnection to send an empty ACK segment.
void TCPSender::send_empty_segment() {
  TCPSegment segment;
  segment.header().seqno = next_seqno();
  segments_out_.push(segment);
}
