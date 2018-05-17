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
    q = 16580000;
    r = 30000000;
   
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
    int i, j , k;

    for (i = 0; i < n; i++)
        if ((i+1)%(ul+il+1)==0)
        	inp[i] = one;
	else if (i%2 == 0)
		inp[i] = zero;
	else 
		inp[i] = one;
     

     //step 9
     int inpAND1[s5_size*(qu+mi-1)*2];
     int outAND1[s5_size*(qu+mi-1)];

     int inpXOR3[s6_size*(qu+mi-1)*2];
     int outXOR3[s6_size*(qu+mi-1)];

     int inpMUL[ue*2];
     int outMUL[ue*2];

     int inpADD[ue*2];
     int outADD[ue];

     int inpAND2[s6_size*2];
     int outAND2[s6_size];

     int inpNOT[1];

     for (k = 0; k >= 0; k--)//qu+nu-2
     {
        for (j = 0; j  < d; j++)
        {
                for (i = 0; i < ue; i++)
                        inpMUL[i] = one;
                for (i = 0; i < ue; i++)
                        inpMUL[i+ue] = zero;
                MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);
        }

        inpNOT[0] = zero;
        NOTCircuit(&garbledCircuit, &garblingContext, 1, inpNOT, inpNOT);
        for (i = 0; i < s6_size*2; i++)
                inpAND2[i] = inpNOT[0];

        ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*2, inpAND2, outAND2);

        for (j = 0; j < d; j++)
        {
                for (i = 0; i < ue; i++)
                        inpADD[i] = one;
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = zero;
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);
        }

        for (i = 0; i < s5_size; i++)
                inpAND2[i] = zero;
        for (i = 0; i < s5_size; i++)
                inpAND2[i+s5_size] = one;
     
        ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*2, inpAND2, outAND2);

        for (j = 0; j < d; j++)
        { 
                for (i = 0; i < ue; i++)
                        inpADD[i] = one;
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = zero;
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);
        }
     }     

    //report output
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

