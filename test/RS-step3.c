#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

//length of user ID
#define ul  	8
//length of item ID
#define il  	8
//number of queries
#define qu  	128
//length of input
#define n 	(ul+il+1)*qu
//length of ellements in user profile
#define ue  	36
//length of ellemenst in item profile
#define ie  	36
//length of user and item profiles
#define d  	10
//number of users
#define nu  	100
//number of items
#define mi  	100
//length of output
#define m 	1
//length of ellements in  row 1 of s
#define s1_size  	ul
//length of ellements in row 2 of s
#define s2_size  	il
//length of ellements in row 3 of s
#define s3_size  	1
//length of ellements in row 4 of s
#define s4_size  	1
//length of ellements in row 5 of s
#define s5_size  	d*ue
//lebgth of ellemensts in row 6 of s
#define s6_size  	d*ie

int *final;

int main(int argc, char **argv) {
    srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 1658000;
    r = 3000000;
   
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
    int i,j;

    for (i = 0; i < n; i++)
        if ((i+1)%(ul+il+1)==0)
        	inp[i] = one;
	else if (i%2 == 0)
		inp[i] = zero;
	else 
		inp[i] = one;


     //step 3
     int inpXOR[s5_size*(qu+nu-1)*2];
     for (i = 0; i < s5_size*(qu+nu-1); i++)
	inpXOR[i] = zero;
     for (i = s5_size; i < s5_size*(qu+nu); i++)
	inpXOR[i+s5_size*(qu+nu-2)] = one;
     
     int outXOR[s5_size*(qu+nu-1)];
     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpXOR, outXOR);

     int inpAND[s5_size*(qu+nu-1)*2];
     for (i = 0; i < (qu+nu-1); i++)
	for (j = 0; j <s5_size; j++)
        	inpAND[i*s5_size+j] = one;
     for (i = 0; i < s5_size*(qu+nu-1); i++)
        inpAND[i+s5_size*(qu+nu-1)] = outXOR[i];
     
     int outAND[s5_size*(qu+nu-1)];
     ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpAND, outAND);

     for (i = 0; i < s5_size*(qu+nu-1); i++)
        inpXOR[i] = one;
     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpXOR, outXOR);



    //report output;
    final = &inp[0];
    finishBuilding(&garbledCircuit, &garblingContext, outputMap, final);
    int TIMES1 = 10;
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

