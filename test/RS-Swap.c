#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

//length of ellements
#define l	8
//length of input
#define n 	l*2+1
//length of output
#define m 	1

int *final;

int main(int argc, char **argv) {
    srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 165800;
    r = 300000;
   
    //Set up input and output tokens/labels.
    block *labels = (block*) malloc(sizeof(block) * 2 * n);
    block *outputbs = (block*) malloc(sizeof(block) * m);
    
    OutputMap outputMap = outputbs;
    InputLabels inputLabels = labels;

    //Actually build a circuit. Alternatively, this circuit could be read
    //from a file.
    createInputLabels(labels, n);
    createEmptyGarbledCircuit(&garbledCircuit, n, m, q, r, inputLabels);
    startBuilding(&garbledCircuit, &garblingContext);
    
    //define 0 and 1 wires
    int zero = fixedZeroWire(&garbledCircuit, &garblingContext);
    int one = fixedOneWire(&garbledCircuit, &garblingContext);

    //generate random inputs (q number of (c, t, r))
    int inp[n];
    int i, j;

    for (i = 0; i < n; i++)
        if (i%2 == 0)
		inp[i] = zero;
	else 
		inp[i] = one;
     

     //step 9
     int inpXOR[l*2];
     int outXOR[l];

     for (i = 0; i < n-1; i++)
	inpXOR[i] = inp[i];
     XORCircuit(&garbledCircuit, &garblingContext, n-1, inpXOR, outXOR);

     int inpAND[l*2];
     int outAND[l];

     for (i = 0; i < l; i++)
	inpAND[i] = one;
     for (i = 0 ; i < l; i++)
	inpAND[i+l] = zero;
     ANDNewCircuit(&garbledCircuit, &garblingContext, n-1, inpAND, outAND);

     for (i = 0; i < l; i++)
	inpXOR[i] = one;
     XORCircuit(&garbledCircuit, &garblingContext, n-1, inpXOR, outXOR);

     XORCircuit(&garbledCircuit, &garblingContext, n-1, inpXOR, outXOR);
     
    //report output
    final = &inp[0];
    finishBuilding(&garbledCircuit, &garblingContext, outputMap, final);
    int TIMES1 = 100;
    long int timeGarble[TIMES1];
    long int timeEval[TIMES1];
    double timeGarbleMedians[TIMES1];
    double timeEvalMedians[TIMES1];
    for (j = 0; j < TIMES1; j++) {
        for (i = 0; i < TIMES1; i++) {
            timeGarble[i] = garbleCircuit(&garbledCircuit, inputLabels, outputMap);
            timeEval[i] = timedEval(&garbledCircuit, inputLabels);
        }
        timeGarbleMedians[j] = ((double) median(timeGarble, TIMES1))/ garbledCircuit.q;
        timeEvalMedians[j] = ((double) median(timeEval, TIMES1))/ garbledCircuit.q;
    }
    double garblingTime = doubleMean(timeGarbleMedians, TIMES1);
    double evalTime = doubleMean(timeEvalMedians, TIMES1);
    printf("Garbling time:  %lf  Evaluation time:  %lf\n", garblingTime, evalTime);
    printf("Total number of gates:  %d\n", garbledCircuit.q);
    printf("Total number of wires:  %d\n", garbledCircuit.r);
    printf("Number of XOR gates:  %d\n", counter);
    return 0;
}

