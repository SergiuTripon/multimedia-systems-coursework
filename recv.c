#include "interpolate.h"
#include "assert.h"

/* write_silence writes num_samples of silent audio data to ofile */
void write_silence(FILE *ofile, int num_samples) {
  int16_t missing_pkt[num_samples];
  int i;
  for (i=0; i < num_samples; i++) {
    missing_pkt[i]=0;
  }
  fwrite(missing_pkt, 2, num_samples, ofile);
}

/* recv_packet processes a received audio packet.  The packet has
   sequence number seqno, and consists of len bytes of data.  In this
   case, it's just PCM, encoded in 16-bit linear format.  The
   "strategy" parameter determines which of several possible loss
   concealment techniques to apply */
void recv_packet(int seqno, int len, char *data, FILE *ofile, int strategy) {
  static int prev_seqno = -1;
  static int16_t* prev_packet_samples = 0;
  int16_t *samples = (int16_t*)data; /* we receive a bunch of bytes
					from the (simulated) net, but
					we know they're really 16 bit
					sample values.  In real life
					we'd convert them from network
					byte order to host byte order,
					but no need to do that here */
  int num_samples = len/2;  /* two bytes to a 16 bit integer */
  printf("recv_packet: seqno=%d\n", seqno);

  if (prev_seqno != -1 && (prev_seqno+1 != seqno)) {
    /* there was missing data - we need to replace it */
    int missing_seqno;

    switch(strategy) {
    case SILENCE: 
      {
	/* create as many packet containing silence as we need to fill the gap */
	missing_seqno = prev_seqno + 1;
	while (missing_seqno < seqno) {
	  write_silence(ofile, num_samples);
	  missing_seqno++;
	}
	break;
      }
    case REPEAT_PREV: 
      {
	/* repeat the previous packet once to fill the gap.  If the
	   gap is longer than one packet, fill the remainder with
	   silence */
	fwrite(prev_packet_samples, 2, num_samples, ofile);
	missing_seqno = prev_seqno + 2;
	while (missing_seqno < seqno) {
	  write_silence(ofile, num_samples);
	  missing_seqno++;
	}
	break;
      }
    case REPEAT_NEXT: 
      {
	/* play the next packet (twice) to fill the gap.  If the gap
	   is longer than one packet, first insert silence, then
	   insert the next packet */
	missing_seqno = prev_seqno + 1;
	while (missing_seqno < seqno) {
	  if (missing_seqno == seqno-1) {
	    /* only repeat back once */
	    /* write the current packet instead of the missing one */
	    fwrite(data, 2, num_samples, ofile);
	  } else {
	    write_silence(ofile, num_samples);
	  }
	  missing_seqno++;
	}
	break;
      }
    case REPEAT_BOTH: 
      {
	/* we'll fill the gap with data from both before and after the
	   gap.  If the gap is one packet long, repeat the last half
	   of the previous packet and the first half of the next
	   packet.  If the gap is two packets long, repeat the whole
	   previous packet, then the whole next packet.  If the gap is
	   three of more packets long, fill the remainder with
	   silence. */
	missing_seqno = prev_seqno + 1;
	if (missing_seqno == seqno-1) {
	  /* the gap is only one packet long */
	  /* fill with last half of prev packet, first half of current packet */

	  /* uncomment the next line to enable smoothing*/
/* #define SMOOTH */
#ifdef SMOOTH
	  /****************************************************************/
	  /******************* your code goes here ************************/
	  /* (add any extra functions you define to the top of this file) */
	  /****************************************************************/
#else
	  fwrite(prev_packet_samples+num_samples/2, 2, num_samples/2, ofile);
	  fwrite(samples, 2, num_samples/2, ofile);
#endif
	} else {
	  /* the gap is two or more packets long */
	  /* first write the prev packet a second time */
	  fwrite(prev_packet_samples, 2, num_samples, ofile);
	  missing_seqno++;
	  while (missing_seqno < seqno) {
	    if (missing_seqno == seqno-1) {
	      /* write the current packet instead of the missing one */
	      fwrite(samples, 2, num_samples, ofile);
	    } else {
	      write_silence(ofile, num_samples);
	    }
	    missing_seqno++;
	  }
	}
	break;
      }
    case INTERPOLATE:
      {
	/* We're going to interpolate a whole new packet (or several packets) in the frequency domain. */

	/****************************************************************/
	/******************* your code goes here ************************/
	/****************************************************************/

	break;
      }
    }
  }
  /* finally, don't forget to write out the packet that arrived after the gap */
  fwrite(samples, 2, num_samples, ofile);
  
  /* hold onto the last received packet - we may need it */
  if (prev_packet_samples != 0)
    free(prev_packet_samples);
  prev_packet_samples = samples;
  prev_seqno = seqno;
};
