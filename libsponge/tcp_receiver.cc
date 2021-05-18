#include "tcp_receiver.hh"

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
  const TCPHeader& header = seg.header();
  WrappingInt32 seqno = header.seqno;
  if (header.syn) {
    has_met_syn_ = true;
    isn_ = seqno;
    seqno = seqno + 1;
  }
  size_t checkpoint = reassembler_.stream_out().bytes_written();
  size_t abs_seqno_64 = unwrap(seqno, isn_, checkpoint);
  size_t stream_index = abs_seqno_64 - 1;
  string data = seg.payload().copy();
  reassembler_.push_substring(data, stream_index, header.fin);
}

// Note:
// (1) reassembler only knows stream index(ingore SYN and FIN).
// (2) result should be wrapped absolute seq #.
optional<WrappingInt32> TCPReceiver::ackno() const {
  if (!has_met_syn_) {
    return {};
  }
  size_t nwrite = reassembler_.stream_out().bytes_written();
  return reassembler_.stream_out().input_ended() ?
    WrappingInt32(wrap(nwrite + 2, isn_)) :  // considering SYN and FIN
    WrappingInt32(wrap(nwrite + 1, isn_));  // considering SYN
}

// Reference: https://cs144.github.io/assignments/lab1.pdf
size_t TCPReceiver::window_size() const {
  return reassembler_.stream_out().bytes_read() + capacity_ - reassembler_.stream_out().bytes_written();
}
