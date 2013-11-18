#ifndef OF2G_H_
#define OF2G_H_

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
// Need to confirm what the actual max size for an OF2G frame is
// it should be less than 256 bytes though, since the He frames are
// max 256 bytes including their metadata (from what I remember anyway)
#define OF2G_BUFFER_SIZE 256
typedef unsigned char of2g_frame_t[OF2G_BUFFER_SIZE];

#define OF2G_FRAME_2_BUFFER(frame) ((unsigned char *) (frame))

// There's only two types of frames, DATA and ACK
typedef enum {
   OF2G_DATA,
   OF2G_ACK
} of2g_frametype_t;


// NB: In all cases it should be assumed that the inputs to these
// functions are non-NULL

// This function should inspect the given of2g frame and return
// a bool indicating whether it is valid. At a minimum this should
// mean verifying the checksum and making sure the frame id and ack
// id fields make sense.
//
// return true if `frame` is good and false if it is bad
bool of2g_valid_frame(of2g_frame_t frame);

// This function should inspect the given of2g frame and return its
// type (either OF2G_DATA or OF2G_ACK) based on the frame id and ack id
// fields.
//
// `frame` is assumed to be a good frame (`valid_of2g_frame(frame)` returns true)
of2g_frametype_t of2g_get_frametype(of2g_frame_t frame);

// This function should return the value of the fid field of `frame`
unsigned char of2g_get_fid(of2g_frame_t frame);

// This function should return the value of the ack id field of `frame`
unsigned char of2g_get_ackid(of2g_frame_t frame);

// This function should return the value of the length field of 'frame'
size_t of2g_get_length(of2g_frame_t frame);

// This function should extract the raw data from `frame` and store it in
// `out`. It is assumed that `frame` is a valid OF2G data frame, and that `out`
// is large enough to store the maximum possible amount of data that an
// OF2G data frame can hold.
//
// This function should return the number of bytes stored into `out`
size_t of2g_get_data_content(of2g_frame_t frame, unsigned char * out);

// This function should wrap the first `length` bytes of `buffer` in an
// OF2G data frame, which is stored in `out`. The result in `out` should
// be ready to send.
//
// This function should return true if the frame is built successfully, and
// should return false if the frame can't be built for any reason.
bool of2g_build_data_frame(unsigned char * buffer, size_t length, unsigned char fid, of2g_frame_t out);

// This function should create an OF2G ack frame to be used as a response for
// `data_frame`, storing it in `out`. It should return true as long as the frame
// was built successfully, and falsse otherwise.
bool of2g_build_ack_frame(of2g_frame_t data_frame, of2g_frame_t out);


// Should return the length, in bytes, of `frame`, INCLUDING header fields and checksum bytes.
size_t of2g_get_frame_length(of2g_frame_t frame);

#endif
