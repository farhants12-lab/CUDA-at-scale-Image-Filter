# GPU-Accelerated Image Filtering

## Overview
This project demonstrates **GPU-accelerated image processing using NVIDIA CUDA and NPP (NVIDIA Performance Primitives)**. The program processes multiple grayscale PGM (Portable Graymap) images stored in an input directory and applies a 5×5 box filter to each image.

The application automatically discovers all `.pgm` files in the specified input directory, processes them sequentially on the selected CUDA-enabled GPU, and saves the filtered images to an output directory.

The executable accepts two arguments: the input directory, which contains the `.pgm` images to be processed, and the output directory, where the program will create and store the processed images.

`./filter.exe --input data --output processed_data`

In this example, the program reads all `.pgm` images from the `data` directory, processes them, and saves the resulting images in the newly created `processed_data` directory.

## Code Organization
```bin/```
— Contains the executable produced when the project is built using the make build command.

```data/```
— Contains the input images in `.pgm` format. This directory is provided to the `--input` argument when running the program.

```lib/```
— Contains NVIDIA CUDA sample helper headers and NPP utility headers required by the project. These are separate from the standard C++ and CUDA Toolkit headers.


```src/```
— Contains the project's source code.

```README.md```
— Provides an overview and description of the project, including its purpose, functionality, and how it can be used.

```INSTALL```
— Contains instructions for installing, building, and running the project.

```Makefile```
— Provides commands for automatically building and cleaning the project.

