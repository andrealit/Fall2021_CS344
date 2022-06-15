Author: Andrea Tongsak
The submission tongsaka_program4.zip is found in Canvas.
To compile and create an executable named line_processor,

gcc --std=gnu99 -pthread -o line_processor main.c


To start the program without redirecting stdin, the input comes from the keyboard:

./line_processor


To start a program with input redirection,

./line-processor < input1.txt


To start a program with stdout redirected,

./line-processor > output1.txt


To get both input and output redirection:

./line-processor < input1.txt > output1.txt