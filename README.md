### Multimedia Systems - Packet Loss - Assignment Code

This is the code of the assignment I completed as part of the [COMPGZ05 - Multimedia Systems](http://www.cs.ucl.ac.uk/teaching_learning/syllabus/mscncs/gz05_multimedia_systems/) module (MSc Web Science and Big Data Analytics) which I undertook at University College London.

---

#### Repository contents

* Source code in C
* output folder storing output files after running the code
* [Assignment report](https://github.com/SergiuTripon/multimedia-systems-packet-loss/blob/master/report/tripon_sergiu_compm034_cw.pdf) in PDF format

---

#### Assignment Brief

The aim of this assignment was to experiment with:
* repairing packet loss with packet audio
* applying DCT (Discrete Cosine Transform)
* get a feel for the effect of discontinuities on how audio sounds

We were supplied with a simple packet loss simulator. It reads in a .wav file, splits it into packets, and simulates sending the packets over a lossy link. It then receives the packets that didn’t get lost and reconstructs a .wav file from them.

The assignment consisted of two tasks:

* Implement a repair strategy: interpolation
* Apply smoothing in order to fix clicks in the audio

---

#### Task 1

The example screenshot below shows the difference in audio waveform before and after interpolation was applied. Interpolation involves reconstructing the first packet in such a way that it sounds more like the packet from before the loss gap while reconstructing the last packet in a way that it sounds more like the packet from after the gap.

![alt text](https://github.com/SergiuTripon/multimedia-systems-packet-loss-report/blob/master/figures/interpolation.png "Interpolation")

[Enlarge image](https://github.com/SergiuTripon/multimedia-systems-packet-loss-report/blob/master/figures/interpolation.png?raw=true)

---

#### Task 2

The example screenshot below shows the difference in audio waveform before and after smoothing was applied. I chose to smooth out the audio by "pulling" the two ends of a packet together.

![alt text](https://github.com/SergiuTripon/multimedia-systems-packet-loss-report/blob/master/figures/smoothing.png "Smoothing")

[Enlarge image](https://github.com/SergiuTripon/multimedia-systems-packet-loss-report/blob/master/figures/smoothing.png?raw=true)

---

#### Instructions

To run the source code, follow the steps below:

```bash
# navigate to multimedia-systems-coursework folder
cd multimedia-systems-coursework

# compile the source code
make

# run compiled program loss_sim and specify parameters
loss_sim --param

```
