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
#define SMOOTH
#ifdef SMOOTH

	  /* Beginning of code by Sergiu Tripon */

	  // calculate the gap
	  int gap = (seqno - 1) - prev_seqno;

	  // calculate steps
	  // if there is one sample, step is 0.5
	  float one_sample = ((float) 1 / (gap + 1));
	  // if there are two samples, steps are 0.3333 and 0.6667
	  float two_samples1 = ((float) 1 / (gap + 2));
	  float two_samples2 = ((float) 1 - two_samples1);

	  // fill the gap with the last half of the previous packet, except the penultimate and last sample
	  fwrite(prev_packet_samples + 160, 2, 158, ofile);

	  // calculate the last sample of the previous packet
	  short int last_sample = ((prev_packet_samples[319] * two_samples2) + (samples[0] * two_samples1));
	  // calculate the penultimate sample from the previous packet, based on the last sample
	  short int penultimate_sample = ((prev_packet_samples[318] * one_sample) + (last_sample * one_sample));

	  // fill the gap with the penultimate sample of the previous packet
	  fwrite(&penultimate_sample, 2, 1, ofile);
	  // fill the gap with the last sample of the previous packet
	  fwrite(&last_sample, 2, 1, ofile);

	  // calculate first sample of the current packet
	  short int first_sample = ((prev_packet_samples[319] * two_samples1) + (samples[0] * two_samples2));
	  // calculate second sample of the current packet, based on the first sample
	  short int second_sample = ((first_sample * one_sample) + (samples[1] * one_sample));

	  // fill the gap with the first sample of the current packet
	  fwrite(&first_sample, 2, 1, ofile);
	  // fill the gap with the second sample of the current packet
	  fwrite(&second_sample, 2, 1, ofile);

	  // fill the gap with the first half of the current packet, except the first and second sample
	  fwrite(samples, 2, 158, ofile);

	  /* End of code by Sergiu Tripon */

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

	/* Beginning of code by Sergiu Tripon */

	// calculate the gap
	int gap = (seqno - 1) - prev_seqno;
	// if the gap is shorter/equal than/to three packets
	if (gap <= 3) {
	  // calculate dct of the packet before the gap
	  int *dct_before = dct(prev_packet_samples, num_samples);
	  // calculate dct of the packet after the gap
	  int *dct_after = dct(samples, num_samples);
	  // if the gap is exactly one packet long
	  if (gap == 1) {

	    // test
	    //printf("gap: %d\n", gap);

	    // calculate the missing sequence number
	    missing_seqno = prev_seqno + 1;

	    // more tests
	    //printf("missing_seqno: %d\n", missing_seqno);
	    //printf("seqno: %d\n", seqno);

	    // while the missing sequence number is less than the current sequence number
	    while (missing_seqno < seqno) {
	      // declare a new set of coefficients
	      int new_set[320];
	      // loop as many times as the number of samples in a packet
	      for(int i = 0; i < 320; i++) {
	        // test
	        //printf("num_samples: %d\n", num_samples);

	        // average the DCT coefficients and add the result to the new set of coefficients
	        new_set[i] = (dct_before[i] + dct_after[i]) / 2;
	      }
	      // calculate the IDCT of the new set of coefficients
	      short int *idct_new_set = idct(new_set, 320);
	      // fill the gap with the IDCT of the interpolated DCT coefficients
	      fwrite(idct_new_set, 2, 320, ofile);
	      // increment the missing sequence number
	      missing_seqno++;
	    }
	  // if the gap is exactly two or three packets long
	  } else if (gap == 2 || gap == 3) {

	    // test
	    //printf("gap: %d\n", gap);

	    // calculate the missing sequence number
	    missing_seqno = prev_seqno + 1;

	    // more tests
	    //printf("missing_seqno: %d\n", missing_seqno);
	    //printf("seqno: %d\n", seqno);

	    // while the missing sequence number is less than the current sequence number
	    while (missing_seqno < seqno) {
	      // declare a new set of coefficients
	      int new_set[320];
	      // calculate the weighted average before the gap
	      double weighted_avg_before = (double) (missing_seqno - prev_seqno) / (seqno - prev_seqno);
	      // calculate the weighted average after the gap
	      double weighted_avg_after = (double) (seqno - missing_seqno) / (seqno - prev_seqno);
	      // loop as many times as the number of samples in a packet
	      for(int i = 0; i < 320; i++) {
	        /* sum the products of the weighted averages and DCT coefficients and add the result to
	           the new set of coefficients */
	        new_set[i] = (weighted_avg_before * dct_before[i]) + (weighted_avg_after * dct_after[i]);
	      }
	      // calculate the IDCT of the new set of coefficients
	      short int *idct_new_set = idct(new_set, 320);
	      // fill the gap with the IDCT of the interpolated DCT coefficients
	      fwrite(idct_new_set, 2, 320, ofile);
	      // increment the missing sequence number
	      missing_seqno++;
	    }
	  }
	// if the gap is longer than three packets
	} else if (gap > 3) {

	  // test
	  //printf("gap: %d\n", gap);

	  // calculate the missing sequence number
	  missing_seqno = prev_seqno + 1;

	  // more tests
	  //printf("missing_seqno: %d\n", missing_seqno);
	  //printf("seqno: %d\n", seqno);

	  // while the missing_sequence number is less than the current sequence number
	  while (missing_seqno < seqno) {
	    // fill the gap with silence
	    write_silence(ofile, 320);
	    // increment the missing sequence number
	    missing_seqno++;
	  }
	}

	/* End of code by Sergiu Tripon */

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
