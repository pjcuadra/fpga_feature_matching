# Feature Matching FPGA optimization

This repository holds a feature matching algorithm boosted using
an FPGA design. The implementation was done using the Arty Z7 board
from Xilinx.

The entire implemented stack is composed by the following components;

* Linux OS Image: A linux image to run on top of the Arty Z7 ARM 
  processor. This image enables the usage of OpenCV as computer 
  vision framework. The image also provides a lightweight desktop
  environment.
* OpenCV Module: This modules extends OpenCV itself adding a feature
  matching algorithm that makes use of the FPGA implementation.
* FPGA Implementation: The FPGA design that implements a hardware
  version of a feature matching algorithm based on the Euclidean
  distance.
* Benchmark Application: An application that runs the FPGA 
  implementation and profiles it.

