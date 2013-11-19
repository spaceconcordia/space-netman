#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <SC_he100.h>
#include "../../include/of2g.h"

static bool initialized = false;
static FILE * he100_pipe;
static int    he100_fd;

#define print_error(msg) fprintf(stderr, msg " (%s:%d): %s\n", __FILE__, __LINE__, strerror(errno))

bool initialize(){

   fprintf(stderr, "Initializing %s\n", __FILE__);

   int he100_pipe_fd = open("/var/log/he100/data.log", O_NONBLOCK);
   if(he100_pipe_fd == -1){
      print_error("Couldn't open he100 pipe fd");
      return false;
   }

   he100_pipe = fdopen(he100_pipe_fd, "rb");

   if(!he100_pipe){
      print_error("Failed to open he100 pipe");
      return false;
   }

   if(0 == (he100_fd = HE100_openPort())){
      fprintf(stderr, "HE100_openPort returned 0 (%s:%d)\n",__FILE__,__LINE__);
      return false;
   }

   fprintf(stderr, "Initialization successful!\n");

   return true;
}

// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool transceiver_read(of2g_frame_t frame){

   if(!initialized){
      initialized = initialize();
   }

   printf("About to fread he100_pipe (%s:%d)\n", __FILE__, __LINE__);
   HE100_read(he100_fd, 15);

   // TODO - check EOF condition also and deal with it

   // read the first 3 bytes, these are always all there, and contain the
   // length field.
   if(3 != fread((void *)frame, 1, 3, he100_pipe)
      && ferror(he100_pipe)
   ){
      print_error();
      return false;
   }

   // Now we get the total length of the frame, so we know how many more
   // bytes to read.
   size_t bytes_to_read = of2g_get_frame_length(frame) - 3;

   // We read the entire rest of the frame
   if(bytes_to_read !=
           fread(((char*)frame)+3, 1, bytes_to_read, he100_pipe)
      && ferror(he100_pipe)
   ){
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
      initialized = initialize();
   }

   HE100_transmitData(
      he100_fd,
      (unsigned char*)OF2G_FRAME_2_BUFFER(frame),
      of2g_get_frame_length(frame));

}
