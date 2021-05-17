#include "byte_stream.hh"

#include <iostream>

// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) :
  buffer_(vector<char>(capacity)),
  capacity_(capacity),
  read_off_(0),
  write_off_(0),
  buffer_size_(0),
  total_nread_(0),
  total_nwrite_(0),
  input_ended_(false),
  error_(false) {}

size_t ByteStream::write(const string &data) {
  size_t write_size = min(remaining_capacity(), data.length());
  if (write_off_ + write_size < capacity_) {
    for (size_t cur_write_size = 0; cur_write_size < write_size; ++cur_write_size) {
      buffer_[write_off_++] = data[cur_write_size];
    }
  } else {
    size_t cur_write_size = 0;
    for (; write_off_ < capacity_; ++write_off_) {
      buffer_[write_off_] = data[cur_write_size++];
    }
    write_off_ = 0;
    for (; cur_write_size < write_size; ++cur_write_size) {
      buffer_[write_off_++] = data[cur_write_size];
    }
  }
  buffer_size_ += write_size;
  total_nwrite_ += write_size;
  return write_size;
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
  size_t peek_size = min(buffer_size(), len);
  string content;
  if (read_off_ + peek_size <= capacity_) {
    content += string(buffer_.begin() + read_off_, buffer_.begin() + read_off_ + peek_size);
  } else {
    content += string(buffer_.begin() + read_off_, buffer_.end());
    size_t already_read_size = capacity_ - read_off_;
    content += string(buffer_.begin(), buffer_.begin() + (peek_size - already_read_size));
  }
  return content;
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) {
  size_t pop_size = min(buffer_size(), len);
  read_off_ = (read_off_ + pop_size) % capacity_;
  buffer_size_ -= pop_size;
  total_nread_ += pop_size;
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
string ByteStream::read(const size_t len) {
  string content = peek_output(len);
  pop_output(len);
  return content;
}
