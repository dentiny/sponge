#include "wrapping_integers.hh"

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

using namespace std;

// //! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
// //! \param n The input absolute 64-bit sequence number
// //! \param isn The initial sequence number
// WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
//   uint64_t seqno64 = isn.raw_value() + n;
//   uint32_t seqno32 = seqno64 % (1ul << 32);
//   return WrappingInt32{seqno32};
// }

// //! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
// //! \param n The relative sequence number
// //! \param isn The initial sequence number
// //! \param checkpoint A recent absolute 64-bit sequence number
// //! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
// //!
// //! \note Each of the two streams of the TCP connection has its own ISN. One stream
// //! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
// //! and the other stream runs from the remote TCPSender to the local TCPReceiver and
// //! has a different ISN.
// uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
//   uint32_t offset = n - wrap(checkpoint, isn);
//   uint64_t res = checkpoint + offset;
//   if (offset >= (1u << 31) && res >= (1ul << 32)) {
//     res -= (1ul << 32);
//   }
//   return res;
// }

WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    uint32_t tmp = (n << 32) >> 32;
    return isn + tmp;
}

uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    uint64_t absolute_seqno_64 = n.raw_value() - isn.raw_value();
    if (checkpoint <= absolute_seqno_64)    // 比较少见的情况
        return absolute_seqno_64;
    else {
        uint64_t size_period = 1ul << 32, quotient, remainder;   
        quotient = (checkpoint - absolute_seqno_64) >> 32;
        remainder = ((checkpoint - absolute_seqno_64) << 32) >> 32;
        if (remainder < size_period / 2)
            return absolute_seqno_64 + quotient * size_period;
        else
            return absolute_seqno_64 + (quotient + 1) * size_period;
    }
}