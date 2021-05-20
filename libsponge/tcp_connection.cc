#include "tcp_connection.hh"

#include <iostream>

// For Lab 4, please replace with a real implementation that passes the
// automated checks run by `make check`.

/*
 * For sending a segment:
 * 1. Any time the TCPSender has pushed a segment onto its outgoing queue, having set
 * the fields it’s responsible for on outgoing segments: (seqno, syn , payload, and fin).
 * 2. Before sending the segment, the TCPConnection will ask the TCPReceiver for the fields
 * it’s responsible for on outgoing segments: ackno and window size. If there is an ackno,
 * it will set the ack flag and the fields in the TCPSegment.
 */

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

/* 
 * For receiving a segment:
 * 1. If the rst (reset) flag is set, sets both the inbound and outbound streams to the error
 * state and kills the connection permanently.
 * 2. Otherwise, gives the segment to the TCPReceiver so it can inspect the fields it cares
 * about on incoming segments: seqno, syn , payload, and fin.
 * 3. If the ack flag is set, tells the TCPSender about the fields it cares about on incoming
 * segments: ackno and window size.
 * 4. If the incoming segment occupied any sequence numbers, the TCPConnection makes sure
 * that at least one segment is sent in reply, to reflect an update in the ackno and
 * window size.
 */
void TCPConnection::segment_received(const TCPSegment &seg) {
  if (seg.header().rst) {
    sender_.stream_in().set_error();
    receiver_.stream_out().set_error();
    return;
  }
  receiver_.segment_received(seg);
  if (seg.header().ack) {
    sender_.ack_received(seg.header().ackno, seg.header().win);
  }
  if (seg.header().seqno != WrappingInt32{0} /* initial seq # */) {
    sender_.fill_window();
  }
}

/*
 * Clean shutdown:
 * Prereq #1 The inbound stream has been fully assembled and has ended.
 * Prereq #2 The outbound stream has been ended by the local application and fully sent
 * (including the fact that it ended, i.e. a segment with fin ) to the remote peer.
 * Prereq #3 The outbound stream has been fully acknowledged by the remote peer.
 * Prereq #4 The local TCPConnection is confident that the remote peer can satisfy
 * prerequisite #3. There are two alternative ways this can happen:
 * Option A: lingering after both streams end. Prerequisites #1 through #3 are true,
 * and the remote peer seems to have gotten the local peer’s acknowledgments of the
 * entire stream.
 * Option B: passive close. Prerequisites #1 through #3 are true, and the local peer
 * is 100% certain that the remote peer can satisfy prerequisite #3, since remote peer
 * was the first one to end its stream.
 * 
 * Unclean shutdown:
 * Both sender's and receiver's ByteStream are at error state.
 */
bool TCPConnection::active() const {
  bool is_shutdown_cleanly = receiver_.stream_out().eof() && sender_.stream_in().eof()
    && sender_.bytes_in_flight() == 0;  // TODO: prereq 4
  bool is_shutdown_uncleanly = sender_.stream_in().error() && receiver_.stream_out().error();
  return !(is_shutdown_cleanly && is_shutdown_uncleanly);
}

size_t TCPConnection::write(const string &data) {
  size_t nwriten = sender_.stream_in().write(data);
  // TODO: ask sender to send
  return nwriten;
}

//! \param[in] ms_since_last_tick number of milliseconds since the last call to this method
void TCPConnection::tick(const size_t ms_since_last_tick) {
  sender_.tick(ms_since_last_tick);
  time_since_last_segment_received_ += ms_since_last_tick;
  if (sender_.consecutive_retransmissions() > TCPConfig::MAX_RETX_ATTEMPTS) {
    reset();
  }
}

void TCPConnection::end_input_stream() {}

void TCPConnection::connect() {}

TCPConnection::~TCPConnection() {
  try {
    if (active()) {
      cerr << "Warning: Unclean shutdown of TCPConnection\n";
      reset();
    }
  } catch (const exception &e) {
    std::cerr << "Exception destructing TCP FSM: " << e.what() << std::endl;
  }
}

// Kill connections on both endpoints.
// Two possible causes:
// 1. consecutive retransmissions exceeds the threshold
// 2. break the current TCP connection(dtor invoked)
void TCPConnection::reset() {
  TCPSegment segment;
  segment.header().rst = true;
  segment.header().seqno = sender_.next_seqno();  // no need to increment the seq #
  segments_out_.push(segment);
  sender_.stream_in().set_error();
  receiver_.stream_out().set_error();
}
