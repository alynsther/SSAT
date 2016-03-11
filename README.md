Main File:   ssat.cc
Author: Adela Yang, Venecia Xu, Son Ngo
Date:   March 2016

Description:
Implement a DPLL-based stochastic satisfiability (SSAT) solver to test the
effectiveness of unit clause propagation, pure variable elimination, and
three different student-devised splitting heuristics compare to the naive 
algorithm.
Given a file, it will run all 7 algorithms on it and report the results back on the terminal.

Running instructions:
Run this program with two arguments on the command line the input file and the algorithm

To compile the program:
make ssat 

To run the program:
./ssat [file name]

The file produces results on the terminal in the following format:

File Read successfully
====================================================================

====================================================================
RESULT OF SOLVESSAT -  [name of algorithm] :  [success probability]
NUM OF UCP: [number of UCP]
NUM OF PVE: [number of PVE]
NUM OF VS: [number of VS]
PERCENTAGE OF VS: [percent of VS]
SOLUTION TIME: [solutionTime] 
====================================================================


FOLDER CONTENTS:
newProblems: contains the newly created .ssat files using your ssat-generator file
e problems have the ordering 36 choice
r problems have the ordering 36 chance
er problems have the ordering 18 choice, 18 chance
re problems have the ordering 18 chance, 18 choice
erer problems have the ordering 9 choice, 9 chance, 9 choice, 9 chance
rere problems have the ordering 9 chance, 9 choice, 9 chance, 9 choice

oldProblems: contains the original .ssat files that you provided us

SSATGenerator: contains your code of generating SSAT problems and the docx file is the command line arguments we used and the 					terminal output of all tests

OTHER FILES:
newReport: contains all the relevant results of the new .ssat files
oldReport: contains all the relevant results of the old .ssat files
WRITE-UP: this is the write-up for Assignment 1b
