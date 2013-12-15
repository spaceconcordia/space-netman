#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <NamedPipe.h>
#include <SC_he100.h>
#include "../../include/of2g.h"

static bool initialized = false;
static FILE * he100_pipe;
static int    he100_fd;
static NamedPipe datapipe("/var/log/he100/data.log");

#define print_error(msg) fprintf(stderr, msg " (%s:%d): %s\n", __FILE__, __LINE__, strerror(errno))

bool initialize(){

   fprintf(stderr, "Initializing %s\n", __FILE__);

    if (!datapipe.Exist()) datapipe.CreatePipe();
    datapipe.ensure_open('r');
   if(0 == (he100_fd = HE100_openPort())){
      fprintf(stderr, "HE100_openPort returned 0 (%s:%d)\n",__FILE__,__LINE__);
      return false;
   }

   fprintf(stderr, "Initialization successful!\n");

   return true;
}

void transceiver_init(){
   initialize();
}

// Read a new frame from the He transceiver. This function may block
// briefly, but not indefinitely. TODO - timeout arg??
//
// should return true and write data into `frame` if new data was read,
// otherwise should return false.
bool transceiver_read(of2g_frame_t frame){
   char garbage[5];
   if(!initialized){
      initialized = initialize();
   }

   printf("About to fread he100_pipe (%s:%d)\n", __FILE__, __LINE__);
   HE100_read(he100_fd, 5);

   // TODO - check EOF condition also and deal with it

    // Read 15 garbage
    // discard
    // read 3
    // check length
    // read length more
    // read 4 ending garbe


    size_t bytes = datapipe.ReadFromPipe((char*)frame, 14);
    bytes = datapipe.ReadFromPipe((char*)frame, 1); // change this to 3
    unsigned char c = ((unsigned char*)frame)[0];
    printf("Data: %c\n", c);


    printf("Read %d bytes:\n", bytes);
    if(bytes >= 3) {
        for(size_t i = 14; i < bytes-5; ++i){
           unsigned char c = ((unsigned char*)frame)[i];
           if(c >= ' ' && c <= '~'){
                putchar(c);
            } else {
                printf(" 0x%02X ", c);
            }
        }
    }
    datapipe.ReadFromPipe(garbage, 5);
    putchar('\n');
    return false;
   // read the first 3 bytes, these are always all there, and contain the
   // length field.
   if(3 != datapipe.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(frame), 3)) 
   {
      printf("length not 3rd byte\n");
      return false;
   }

   // Now we get the total length of the frame, so we know how many more
   // bytes to read.
   size_t bytes_to_read = of2g_get_frame_length(frame) - 3;

   // We read the entire rest of the frame
   if(bytes_to_read !=
           datapipe.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(frame) + 3, bytes_to_read))
   {
      printf("Can't read rest of frame\n");
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
