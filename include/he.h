#ifndef HE_H_
#define HE_H_

#include <stdbool.h>

#include "of2g.h"


// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool he_read(of2g_frame_t * frame);

// Send a frame over the He. This function can (should?) block while the frame is
// being sent.
//
void he_write(of2g_frame_t * frame);

#endif
