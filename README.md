# Real-Time Embedded Systems
This repository includes the code for the projects of the course Real-Time Embedded Systems taken at the ECE department of the Aristotle University of Thessaloniki.

## rtes1.c
With the use of two threads (producer and consumer) we simulate the following process:
The first adds a job in a queue and the second executes and deletes it from the queue.
In this example, the job is a very simple task, but it can be replaced by any other function.

## rtes_final.c
Generates random Bluetooth MAC addresses and simulates a normal day where you have contact with people.
Every 2.5 minutes, you take a COVID test and if the test is positive, all contacts that are close to you for more than 12 seconds are informed (generates a file with these contacts' MAC addresses).

## rtes.m
Used to process the Latency.bin produced by the rtes_final.c and generate the final data.
