#include "stream_reassembler.hh"

#include <cassert>
#include <iostream>

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) :
  bitmap_(capacity, false),
  buffer_(capacity),
  unassembled_bytes_(0),
  first_unassembled_index_(0),
  eof_index_(0),
  has_met_eof_(false),
  output_(capacity),
  capacity_(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
  // Segment's effective substring index is [start_index, end_index) in total order.
  size_t start_index = max(first_unassembled_index_, index);
  size_t end_index = min(first_unassembled_index_ + output_.remaining_capacity(), index + data.length());

  // Set bitmap and buffer.
  for (size_t idx = start_index; idx < end_index; ++idx) {
    size_t buffer_index = idx % capacity_;
    buffer_[buffer_index] = data[idx - index];
    if (!bitmap_[buffer_index]) {
      ++unassembled_bytes_;
      bitmap_[buffer_index] = true;
    }
  }

  // Get the end index of possible submission.
  size_t submit_length = 0;
  for (int submit_start_index = first_unassembled_index_ % capacity_;
      submit_start_index + submit_length < capacity_ &&
      bitmap_[submit_start_index + submit_length];
      ++submit_length);

  // Submit all possible characters.
  if (submit_length > 0) {  // there's character to submit
    // Note: considering buffer index is circular, cannot assign string value as:
    // size_t start_buffer_index = first_unassembled_index_ % capacity_;
    // size_t end_buffer_index = end_buffer_index % capacity_;
    // string submit_data(buffer_.begin() + first_unassembled_index_, buffer_.begin() + submit_end_index);
    // size_t submit_length = submit_end_index - first_unassembled_index_;

    string submit_data(submit_length, 0);
    size_t submit_end_index = first_unassembled_index_ + submit_length;
    for (size_t idx = first_unassembled_index_; idx < submit_end_index; ++idx) {
      submit_data[idx - first_unassembled_index_] = buffer_[idx % capacity_];
    }

    size_t nwrite = output_.write(submit_data);
    assert(nwrite == submit_length);
    for (size_t idx = first_unassembled_index_; idx < first_unassembled_index_ + nwrite; ++idx) {
      bitmap_[idx % capacity_] = false;  // clear bitmap for next round's reuse
    }
    first_unassembled_index_ = submit_end_index;
    unassembled_bytes_ -= nwrite;
  }

  // Handle EOF.
  if (eof) {
    eof_index_ = index + data.length();
    has_met_eof_ = true;
  }
  if (has_met_eof_ && output_.bytes_written() == eof_index_) {  // all bytes have been placed into ByteStream
      output_.end_input();
  }
}
