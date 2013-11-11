#include "of2g.h"
#include "he100.h"

// This function should inspect the given of2g frame and return
// a bool indicating whether it is valid. At a minimum this should
// mean verifying the checksum and making sure the frame id and ack
// id fields make sense.
//
// return true if `frame` is good and false if it is bad
bool of2g_valid_frame(of2g_frame_t * frame)
{
	// TODO: what do we need to know about FID and ACK for it to be valid?

	uint8_t length = *(frame[2]); // frames bytes are char, does this cause issues?

	// verify checksum
	HE100_checksum frame_checksum = HE100_fletcher16((char*)*frame, length+3);
	if(*(frame[length+3]) == frame_checksum.sum1 && *(frame[3+length+1]) == frame_checksum.sum2 )
	{
		return true;
	}
	else
	{
		return false;
	}

}

// This function should inspect the given of2g frame and return its
// type (either OF2G_DATA or OF2G_ACK) based on the frame id and ack id
// fields.
//
// `frame` is assumed to be a good frame (`valid_of2g_frame(frame)` returns true)
of2g_frametype_t of2g_get_frametype(of2g_frame_t * frame)
{

	// TODO: Need to know how to verify if DATA or ACK
	unsigned char fid = of2g_get_fid(frame);
	unsigned char ackid = of2g_get_ackid(frame);

	if(ackid == '0') {
		return OF2G_ACK;
	}

	if (fid == ackid) {
		return OF2G_ACK;
	}
	else {
		return OF2G_DATA;
	}

}

// This function should return the value of the fid field of `frame`
unsigned char of2g_get_fid(of2g_frame_t * frame)
{
	return *(frame[0]);

}

// This function should return the value of the ack id field of `frame`
unsigned char of2g_get_ackid(of2g_frame_t * frame)
{
	return *(frame[1]);
}

// This function should extract the raw data from `frame` and store it in
// `out`. It is assumed that `frame` is a valid OF2G data frame, and that `out`
// is large enough to store the maximum possible amount of data that an
// OF2G data frame can hold.
//
// This function should return the number of bytes stored into `out`
uint8_t of2g_get_data_content(of2g_frame_t * frame, unsigned char * out)
{

	// data length - FID, ack, length, 2 bytes for chksum
	uint8_t length = *(frame[3]) - 5;

	int i;
	for(i=3 ;i < length ;++i)
	{
		out[i] = *(frame[i+3]);
	}

	return length;

}

// This function should wrap the first `length` bytes of `buffer` in an
// OF2G data frame, which is stored in `out`. The result in `out` should
// be ready to send.
//
// This function should return true if the frame is built successfully, and
// should return false if the frame can't be built for any reason.
bool of2g_build_data_frame(unsigned char * buffer, uint8_t length, unsigned char fid, of2g_frame_t * out)
{
	// length -> size_t
	// wrap "length" bytes of buffer in of2g data frame
	int i;
	for(i = 0;i < length; ++i)
	{
		*(out[i]) = buffer[i];
	}

	// place FID, ACK, Length
	*(out[0]) = fid;
	*(out[2]) = length + 5;
	// TODO: ACK

	// place checksum
	HE100_checksum frame_checksum = HE100_fletcher16((char*)*out, length+3);	// TODO: fix checksum for int

	*(out[length+3]) = frame_checksum.sum1;
	*(out[3+length+1]) = frame_checksum.sum2;
	return true;

	// TODO: verify for false condition
}

// This function should create an OF2G ack frame to be used as a response for
// `data_frame`, storing it in `out`. It should return true as long as the frame
// was built successfully, and false otherwise.
bool of2g_build_ack_frame(of2g_frame_t * data_frame, of2g_frame_t * out)
{

	// TODO: does the FID become the ack?
	// does the ack frame contain data (between length and checksum)?

	// place fid,ackid,length
	*(out[0]) = of2g_get_fid(data_frame);
	*(out[1]) = of2g_get_ackid(data_frame);

	*(out[2]) = 5; // length of ACK is 5 assuming no data

	// place checksum
	HE100_checksum frame_checksum = HE100_fletcher16((char*)*out, 3);

	*(out[3]) = frame_checksum.sum1;
	*(out[4]) = frame_checksum.sum2;

	return true;

	// TODO: verify for false condition, Log if fails
	/*
	Log(FILE* lf, Priority ePriority, string process, string msg);
	get_filename(string folder, string prefix, string suffix);
	*/
}
