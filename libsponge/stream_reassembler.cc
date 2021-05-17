#include "stream_reassembler.hh"

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
    buffer_[idx] = data[idx - index];
    if (!bitmap_[idx]) {
      ++unassembled_bytes_;
      bitmap_[idx] = true;
    }
  }

  // Get the end index of possible submission.
  size_t submit_end_index = first_unassembled_index_;
  for (; submit_end_index < capacity_ && bitmap_[submit_end_index]; ++submit_end_index);

  cout << "current input substring is " << data << " with index = " << index << endl;
  cout << "submit start index = " << first_unassembled_index_ << endl;
  cout << "submit end index = " << submit_end_index << endl;

  // Submit all possible characters.
  if (submit_end_index > first_unassembled_index_) {  // there's character to submit
    string submit_data(buffer_.begin() + first_unassembled_index_, buffer_.begin() + submit_end_index);
    size_t nwrite = output_.write(submit_data);
    for (size_t idx = first_unassembled_index_; idx < first_unassembled_index_ + nwrite; ++idx) {
      bitmap_[idx] = false;
    }
    first_unassembled_index_ += nwrite;
    unassembled_bytes_ -= nwrite;

    cout << "write " << nwrite << " bytes to ByteStream:" << submit_data << endl;
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
