#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include <SC_he100.h>
#include "../../include/of2g.h"

static bool initialized = false;
static int  fdin;

#define print_error() fprintf(stderr, "I/O error (%s:%d): %s\n", __FILE__, __LINE__, strerror(errno))

// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool transceiver_read(of2g_frame_t frame){

   if(!initialized){
      if (0 != (fdin = HE100_openPort())){
         initialized = true;
      }else{
         return false;
      }
   }

   // read the first 3 bytes, these are always all there, and contain the
   // length field.
   if(-1 == read(fdin, (void *)frame, 3)){
      print_error();
      return false;
   }

   // Now we get the total length of the frame, so we know how many more
   // bytes to read.
   size_t length = of2g_get_frame_length(frame);

   // We read the entire rest of the frame
   //
   // TODO - Does the value in the length field include the 3 bytes worth
   // of headers? Does it include the 2 bytes of checksum?
   if(-1 == read(fdin, ((char*)frame)+3, length-3)){
      print_error();
      return false;
   }

   return true;

}

// Send a frame over the He. This function can (should?) block while the frame is
// being sent.
//
void transceiver_write(of2g_frame_t frame){

   if(!initialized){
      if (0 != (fdin = HE100_openPort())){
         initialized = true;
      }else{
         // TODO - signal failure?
         return;
      }
   }

   HE100_transmitData(
      fdin,
      (unsigned char*)OF2G_FRAME_2_BUFFER(frame),
      of2g_get_frame_length(frame));

}
