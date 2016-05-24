#### Multimedia Systems - Assignment

This is an assigment I completed as part of the Multimedia Systems module (MSc Web Science and Big Data Analytics) I undertook at UCL.

#####Assignment Brief

The aim of this assignment was to experiment with
* repairing packet loss with packet audio
* applying DCT (Discrete Cosine Transform)
* get a feel for the effect of discontinuities on how audio sounds

We were supplied with a simple packet loss simulator. It reads in a .wav file, splits it into packets, and simulates sending the packets over a lossy link. It then receives the packets that didn’t get lost and reconstructs a .wav file from them.

The assignment consisted of two tasks:

* Implement a repair strategy: interpolation
* Apply smoothing in order to fix clicks in the audio

#####Task 1

The example screenshot below shows the difference in audio waveform before and after interpolation was applied. Interpolation involves reconstructing the first packet in such a way that it sounds more like the packet from before the loss gap while reconstructing the last packet in a way that it sounds more like the packet from after the gap.

![alt text](https://github.com/SergiuTripon/multimedia-systems-coursework/blob/master/report-src/figures/interpolation.png "Interpolation")

#####Task 2

The example screenshot below shows the difference in audio waveform before and after smoothing was applied. I chose to smooth out the audio by "pulling" the two ends of a packet together.

![alt text](https://github.com/SergiuTripon/multimedia-systems-coursework/blob/master/report-src/figures/smoothing.png "Smoothing")
