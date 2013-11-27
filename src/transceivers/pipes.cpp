#include <string.h>
#include <errno.h>
#include <assert.h>

#include <NamedPipe.h>

#include "../../include/of2g.h"

static bool initialized = false;

#ifdef TRNSCVR_TX_PIPE
static NamedPipe hetx(TRNSCVR_TX_PIPE);
#else
#error "TRNSCVR_TX_PIPE not #define'd!"
#endif

#ifdef TRNSCVR_RX_PIPE
static NamedPipe herx(TRNSCVR_RX_PIPE);
#else
#error "TRNSCVR_RX_PIPE not #define'd!"
#endif

// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool transceiver_read(of2g_frame_t frame){

   if(!initialized){
      if (!hetx.Exist()) hetx.CreatePipe();
      if (!herx.Exist()) herx.CreatePipe();
      herx.ensure_open('r');
      initialized = true;
   }

   uint8_t bytes_read = herx.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(frame), OF2G_BUFFER_SIZE);

   uint8_t frame_len  = of2g_get_frame_length(frame);

   if(bytes_read == 0){
      return false;
   }else{
      printf("Completed read of %d bytes, acquiring a %d byte long OF2G frame\n", bytes_read, frame_len);
      return true;
   }

}

// Send a frame over the He. This function can (should?) block while the frame is
// being sent.
//
void transceiver_write(of2g_frame_t frame){

   if(!initialized){
      if (!hetx.Exist()) hetx.CreatePipe();
      if (!herx.Exist()) herx.CreatePipe();
      herx.ensure_open('r');
      initialized = true;
   }

   printf("TX'ing frame with length %d\n", of2g_get_frame_length(frame));

   hetx.WriteToPipe((char*)OF2G_FRAME_2_BUFFER(frame), of2g_get_frame_length(frame));

}
