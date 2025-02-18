#ifndef SPONGE_LIBSPONGE_BYTE_STREAM_HH
#define SPONGE_LIBSPONGE_BYTE_STREAM_HH

#include <string>
#include <vector>

//! \brief An in-order byte stream.

//! Bytes are written on the "input" side and read from the "output"
//! side.  The byte stream is finite: the writer can end the input,
//! and then no more bytes can be written.
class ByteStream {
  private:
    // Your code here -- add private members as necessary.

    // Hint: This doesn't need to be a sophisticated data structure at
    // all, but if any of your tests are taking longer than a second,
    // that's a sign that you probably want to keep exploring
    // different approaches.
    std::vector<char> buffer_;
    size_t capacity_;
    size_t read_off_;  // byte index of next read
    size_t write_off_;  // byte index of next write
    size_t buffer_size_;
    size_t total_nread_;
    size_t total_nwrite_;
    bool input_ended_;
    bool error_;  //!< Flag indicating that the stream suffered an error.

  public:
    //! Construct a stream with room for `capacity` bytes.
    ByteStream(const size_t capacity);

    //! \name "Input" interface for the writer
    //!@{

    //! Write a string of bytes into the stream. Write as many
    //! as will fit, and return how many were written.
    //! \returns the number of bytes accepted into the stream
    size_t write(const std::string &data);

    //! \returns the number of additional bytes that the stream has space for
    size_t remaining_capacity() const { return capacity_ - buffer_size_; }

    //! Signal that the byte stream has reached its ending
    void end_input() { input_ended_ = true; }

    //! Indicate that the stream suffered an error.
    void set_error() { error_ = true; }
    //!@}

    //! \name "Output" interface for the reader
    //!@{

    //! Peek at next "len" bytes of the stream
    //! \returns a string
    std::string peek_output(const size_t len) const;

    //! Remove bytes from the buffer
    void pop_output(const size_t len);

    //! Read (i.e., copy and then pop) the next "len" bytes of the stream
    //! \returns a string
    std::string read(const size_t len);

    //! \returns `true` if the stream input has ended
    bool input_ended() const { return input_ended_;  }

    //! \returns `true` if the stream has suffered an error
    bool error() const { return error_; }

    //! \returns the maximum amount that can currently be read from the stream
    size_t buffer_size() const { return buffer_size_; }

    //! \returns `true` if the buffer is empty
    bool buffer_empty() const { return buffer_size_ == 0; }

    //! \returns `true` if the output has reached the ending
    bool eof() const { return input_ended() && buffer_empty(); }
    //!@}

    //! \name General accounting
    //!@{

    //! Total number of bytes written
    size_t bytes_written() const { return total_nwrite_; }

    //! Total number of bytes popped
    size_t bytes_read() const { return total_nread_; }
    //!@}
};

#endif  // SPONGE_LIBSPONGE_BYTE_STREAM_HH
