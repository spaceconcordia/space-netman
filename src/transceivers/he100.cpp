#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <NamedPipe.h>
#include <SC_he100.h>
#include <fletcher.h>
#include "../../include/of2g.h"

static bool initialized = false;
static int    he100_fd;
static NamedPipe datapipe("/var/log/he100/data.log");

#define print_error(msg) fprintf(stderr, msg " (%s:%d): %s\n", __FILE__, __LINE__, strerror(errno))

bool initialize(){

   fprintf(stderr, "Initializing %s\n", __FILE__);

    if (!datapipe.Exist()) datapipe.CreatePipe();
    datapipe.Open('r');
   if(0 == (he100_fd = HE100_openPort())){
      fprintf(stderr, "HE100_openPort returned 0 (%s:%d)\n",__FILE__,__LINE__);
      return false;
   }

   fprintf(stderr, "Initialization successful!\n");

   return true;
}

unsigned char*
hex_decode(unsigned char *in, size_t len,unsigned char *out)
{
    unsigned int i, t, hn, ln;

    for (t = 0,i = 0; i < len; i+=2,++t) {

        hn = in[i] > '9' ? in[i] - 'A' + 10 : in[i] - '0';
        ln = in[i+1] > '9' ? in[i+1] - 'A' + 10 : in[i+1] - '0';

        out[t] = (hn << 4 ) | ln;
    }

    return out;
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
   of2g_frame_t tmp_frame;
   unsigned char payload[256];

   if(!initialized){
      initialized = initialize();
   }

   printf("Reading from transceiver... (%s:%d)\n", __FILE__, __LINE__);
   HE100_read(he100_fd, 5, payload);

   // TODO - check EOF condition also and deal with it

    // Read 14 garbage
    // discard
    // read 3
    // check length
    // read length more
    // read 4 ending garbage bytes + endline

    // read garbage bytes first
    /*
    datapipe.ReadFromPipe((char*)tmp_frame, 14);

   // read the first 3 bytes, these are always all there, and contain the
   // length field.
   if(6 != datapipe.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(tmp_frame), 6)) 
   {
      printf("Could not get first 3 bytes for length\n");
      return false;
   }
   */

    unsigned char res[3];
    hex_decode(payload,6,res);
    printf("length byte: %d\n", res[2]);

    printf("Read first 3 bytes:\n");
    for(size_t i = 0; i < 3; ++i){
       unsigned char c = res[i]; 
            printf(" 0x%02X ", c);
    }
    putchar('\n');

   // Now we get the total length of the frame, so we know how many more
   // bytes to read.
   size_t bytes_to_read = res[2] + 2;
   bytes_to_read = bytes_to_read * 2;
   printf("bytes to read after length byte: %d!\n", (int)bytes_to_read);

   // We read the entire rest of the frame
   /*
   if(bytes_to_read !=
           datapipe.ReadFromPipe((char *)OF2G_FRAME_2_BUFFER(tmp_frame) + 6, bytes_to_read))
   {
      printf("Can't read rest of frame\n");
      return false;
   }
    // Dump garbage bytes after our frame
    datapipe.ReadFromPipe(garbage, 5);
    */

    // Convert full ASCII frame to HEX
    size_t frame_length = bytes_to_read + 6;
/*    hex_decode((unsigned char *)OF2G_FRAME_2_BUFFER(tmp_frame),frame_length+1,(unsigned char*)OF2G_FRAME_2_BUFFER(frame));

    printf("total frame length = %zu\n", of2g_get_frame_length(frame));
    printf("Unconverted frame: \n");
    for(size_t i = 0; i < frame_length; ++i){
       unsigned char c = OF2G_FRAME_2_BUFFER(tmp_frame)[i]; 
       if(c >= ' ' && c <= '~'){
            putchar(c);
        } else {
            printf(" 0x%02X ", c);
        }
    }
    printf("\nConverted frame: \n");
    for(size_t i = 0; i < of2g_get_frame_length(frame); ++i){
       unsigned char c = OF2G_FRAME_2_BUFFER(frame)[i]; 
            printf(" 0x%02X ", c);
    }
    printf("\n");
    */
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
