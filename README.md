# GPU-Accelerated Image Filtering

## Overview
This project demonstrates **GPU-accelerated image processing using NVIDIA CUDA and NPP (NVIDIA Performance Primitives)**. The program processes multiple grayscale PGM (Portable Graymap) images stored in an input directory and applies a 5×5 box filter to each image.

The application automatically discovers all `.pgm` files in the specified input directory, processes them sequentially on the selected CUDA-enabled GPU, and saves the filtered images to an output directory.

The executable accepts two arguments: the input directory, which contains the `.pgm` images to be processed, and the output directory, where the program will create and store the processed images.

`./filter.exe --input data --output processed_data`

In this example, the program reads all `.pgm` images from the `data` directory, processes them, and saves the resulting images in the newly created `processed_data` directory.

## Code Organization
```bin/```
This folder contains the executable code that is built with `make build` command. 

```data/```
This folder contains all the input images in the `.pgm`format. This forder is provided to the `--input` flag in the argument. 

```lib/```
NVIDIA CUDA Sample helper headers are included in this folder. These are not the standared C++ headers and the CUDA Toolkit headers.


```src/```
This folder contains the source code.

```README.md```
This file contains the description of the project so that anyone cloning or deciding if they want to clone this repository can understand its purpose to help with their decision.

```INSTALL```
This file should hold the human-readable set of instructions for installing the code so that it can be executed. If possible it should be organized around different operating systems, so that it can be done by as many people as possible with different constraints.

```Makefile or CMAkeLists.txt or build.sh```
This a scripts for building the project's code in an automatic fashion.

