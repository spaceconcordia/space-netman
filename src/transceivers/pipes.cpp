#include <NamedPipe.h>

#include "../../include/of2g.h"

static bool initialized = false;
static NamedPipe hetx("trans_tx");
static NamedPipe herx("trans_rx");

// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool transceiver_read(of2g_frame_t frame){

   if(!initialized){
      if (!hetx.Exist()) hetx.CreatePipe();
      if (!herx.Exist()) herx.CreatePipe();
      herx.persist_open('r');
      initialized = true;
   }

   return 0 < herx.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(frame), OF2G_BUFFER_SIZE);

}

// Send a frame over the He. This function can (should?) block while the frame is
// being sent.
//
void transceiver_write(of2g_frame_t frame){

   if(!initialized){
      if (!hetx.Exist()) hetx.CreatePipe();
      if (!herx.Exist()) herx.CreatePipe();
      herx.persist_open('r');
      initialized = true;
   }

   hetx.WriteToPipe((char*)OF2G_FRAME_2_BUFFER(frame), of2g_get_frame_length(frame));

}
