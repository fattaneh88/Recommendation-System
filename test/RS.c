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
#define qu  	1
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
    q = 165800000;
    r = 300000000;
   
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
    int i;

    for (i = 0; i < n; i++)
        if ((i+1)%(ul+il+1)==0)
        	inp[i] = one;
	else if (i%2 == 0)
		inp[i] = zero;
	else 
		inp[i] = one;

     //step 1: onstruct a matrix s
     int s1[s1_size*(nu+mi+qu)];
     for (i = 0; i < s1_size*(nu+mi+qu); i++)
	if (i%2 == 0)
		s1[i] = one;
	else
		s1[i] = zero;
     
     int s2[s2_size*(nu+mi+qu)];
     for (i = 0; i < s2_size*(nu+mi+qu); i++)
        if (i%2 == 0)
                s2[i] = one;
        else
                s2[i] = zero;

     int s3[s3_size*(nu+mi+qu)];
     for (i = 0; i < s3_size*(nu+mi+qu); i++)
        if (i < (nu+mi))
                s3[i] = zero;
        else
                s3[i] = one;

     int s4[s4_size*(nu+mi+qu)];
     for (i = 0; i < s4_size*(nu+mi+qu); i++)
        if (i < (nu+mi))
                s4[i] = zero;
        else
                s4[i] = one;

     int s5[s5_size*(nu+mi+qu)];
     for (i = 0; i < s5_size*(nu+mi+qu); i++)
        if (i%2 == 0)
                s5[i] = one;
        else
                s5[i] = zero;

     int s6[s6_size*(nu+mi+qu)];
     for (i = 0; i < s6_size*(nu+mi+qu); i++)
        if (i%2 == 0)
                s6[i] = one;
        else
                s6[i] = zero;

     //step 2: sort tuples with respect to rows 1 and 3
     //we have separate code for this step
     

     //step 3
     int inpXOR[s5_size*(qu+nu-1)*2];
     for (i = 0; i < s5_size*(qu+nu-1); i++)
	inpXOR[i] = s5[i];
     for (i = s5_size; i < s5_size*(qu+nu); i++)
	inpXOR[i+s5_size*(qu+nu-2)] = s5[i];
     
     int outXOR[s5_size*(qu+nu-1)];
     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpXOR, outXOR);

     int inpAND[s5_size*(qu+nu-1)*2];
     int j;
     for (i = 0; i < (qu+nu-1); i++)
	for (j = 0; j <s5_size; j++)
        	inpAND[i*s5_size+j] = s3[i+1];
     for (i = 0; i < s5_size*(qu+nu-1); i++)
        inpAND[i+s5_size*(qu+nu-1)] = outXOR[i];
     
     int outAND[s5_size*(qu+nu-1)];
     ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpAND, outAND);

     for (i = 0; i < s5_size*(qu+nu-1); i++)
        inpXOR[i] = one;//outAND[i];
     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+nu-1)*2, inpXOR, outXOR);

     for (i = 0; i < s5_size*(qu+nu-1); i++)
	s5[i+s5_size] = outXOR[i];

    //step 4: sort tuples with respect to rows 2 and 3
    //we have separate code for this step


    //step 5
    int inpXOR1[s6_size*(qu+mi-1)*2];
     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpXOR1[i] = s6[i];
     for (i = s6_size; i < s6_size*(qu+mi); i++)
        inpXOR1[i+s6_size*(qu+mi-2)] = s6[i];

     int outXOR1[s6_size*(qu+mi-1)];
     XORCircuit(&garbledCircuit, &garblingContext, s6_size*(qu+mi-1)*2, inpXOR1, outXOR1);

     int inpAND1[s6_size*(qu+mi-1)*2];
     for (i = 0; i < (qu+mi-1); i++)
        for (j = 0; j < s6_size; j++)
                inpAND1[i*s6_size+j] = s3[i+1];
     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpAND1[i+s6_size*(qu+mi-1)] = outXOR1[i];

     int outAND1[s6_size*(qu+mi-1)];
     ANDNewCircuit(&garbledCircuit, &garblingContext, s6_size*(qu+mi-1)*2, inpAND1, outAND1);

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpXOR1[i] = outAND1[i];

     XORCircuit(&garbledCircuit, &garblingContext, s6_size*(qu+mi-1)*2, inpXOR1, outXOR1);

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        s6[i+s6_size] = outXOR1[i];
    
    //step 6
    int inpMUL[ue*2];
    int outMUL[ue*2];
    
    int lambda[ue];
    for (i = 0; i < ue; i++)
	if (i%2 == 0)
		lambda[i] = one;
	else
		lambda[i] = zero;

    int inpADD[ue*2];
    int outADD[ue];
    for (i = 0; i < ue; i++)
	outADD[i] = zero;

    int inpSUB[ue*2];
    int outSUB[ue*2];

    int inpXOR2[s5_size*(qu+mi-1)*2];
    int outXOR2[s5_size*(qu+mi-1)];

    int inpXOR3[s6_size*(qu+mi-1)*2];
    int outXOR3[s6_size*(qu+mi-1)];
    
    int k;
    for (k = 1; k < qu+mi; k++)
    {
	for (i = 0; i < d; i++)
	{
		for (j = 0; j < ue; j++)
			inpMUL[j] = s5[(k-1)*s5_size+i*d+j];
		for (j = 0; j < ue; j++)
			inpMUL[j+ue] = s6[(k-1)*s6_size+i*d+j];
		MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);

		for (j = 0; j < ue; j++)
			inpADD[i] = outMUL[i];
		for (j = 0; j < ue; j++)
			inpADD[i+ue] = outADD[i];
		ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);
	}

	for (i = 0; i < ue; i++)
		if (i == 15)
			inpSUB[i] = s4[k];
		else
			inpSUB[i] = zero;
	for (i = 0; i < ue; i++)
		inpSUB[i+ue] = outADD[i];
	SUBCircuit(&garbledCircuit, &garblingContext, ue*2, inpSUB, outSUB); 

	for (i = 0; i < ue; i++)
		inpMUL[i] = lambda[i];
	for (i = 0; i < ue; i++)
		inpMUL[i+ue] = outSUB[i];	
	MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);

	for (i = 0; i < ue; i++)
		inpMUL[i] = outMUL[i];

        for (i = 0; i < d; i++)
        {
                for (j = 0; j < ue; j++)
                        inpMUL[j+ue] = s6[(k-1)*s6_size+i*d+j];
                MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);

                for (j = 0; j < ue; j++)
			inpXOR2[(k-1)*s5_size+i*d+j] = outMUL[j];
        }

        for (i = 0; i < d; i++)
        {
                for (j = 0; j < ue; j++)
                        inpMUL[j+ue] = s5[(k-1)*s5_size+i*d+j];
                MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);

                for (j = 0; j < ue; j++)
                        inpXOR3[(k-1)*s6_size+i*d+j] = outMUL[j];
        }
    }
    
     for (i = 0; i < s5_size*(qu+mi-1); i++)
        inpXOR2[i+s5_size*(qu+mi-1)] = s5[i];

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpXOR3[i+s6_size*(qu+mi-1)] = s6[i];

     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+mi-1), inpXOR2, outXOR2);
     XORCircuit(&garbledCircuit, &garblingContext, s6_size*(qu+mi-1), inpXOR3, outXOR3);

     for (i = 0; i < (qu+mi-1); i++)
        for (j = 0; j < s5_size; j++)
                inpAND1[i*s5_size+j] = s3[i];
     for (i = 0; i < s5_size*(qu+mi-1); i++)
        inpAND1[i+s5_size*(qu+mi-1)] = outXOR2[i];

     ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+mi-1), inpAND1, outAND1);

     for (i = 0; i < s5_size*(qu+mi-1); i++)
        inpXOR2[i] = outAND1[i];

     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+mi-1), inpXOR2, outXOR2);

     for (i = 0; i < s5_size*(qu+mi-1); i++)
        s5[i] = outXOR2[i];

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpAND1[i+s6_size*(qu+mi-1)] = outXOR3[i];
     
     ANDNewCircuit(&garbledCircuit, &garblingContext, s6_size*(qu+mi-1), inpAND1, outAND1);

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        inpXOR3[i] = outAND1[i];

     XORCircuit(&garbledCircuit, &garblingContext, s5_size*(qu+mi-1), inpXOR3, outXOR3);

     for (i = 0; i < s6_size*(qu+mi-1); i++)
        s6[i] = outXOR3[i];


     //step 7
     int inpAND2[s6_size*2];
     int outAND2[s6_size];

     int inpNOT[1];

     for (k = qu+mi-2; k >= 0; k--)
     {
	for (j = 0; j  < d; j++)
	{
		for (i = 0; i < ue; i++)
                	inpMUL[i] = lambda[i];
        	for (i = 0; i < ue; i++)
                	inpMUL[i+ue] = s6[k*s6_size+j*d+i];
        	MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);	
 		
		for (i = 0; i < ue; i++)
			inpAND2[j*d+i+s6_size] = outMUL[i];
	}

	inpNOT[0] = s3[k];
	NOTCircuit(&garbledCircuit, &garblingContext, 1, inpNOT, inpNOT);	
	for (i = 0; i < s6_size; i++)
		inpAND2[i] = inpNOT[0];

	ANDNewCircuit(&garbledCircuit, &garblingContext, s6_size*2, inpAND2, outAND2);

	for (j = 0; j < d; j++)
	{
		for (i = 0; i < ue; i++)
                        inpADD[i] = s6[k*s6_size+j*d+i];
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = outAND2[j*d+i];
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);
	
		for (i = 0; i < ue; i++)
			s6[k*s6_size+j*d+i] = outADD[i];
	}

        for (i = 0; i < s6_size; i++)
		inpAND2[i] = s6[(k+1)*s6_size+i];
        for (i = 0; i < s6_size; i++)
                inpAND2[i+s6_size] = s3[k+1];

        ANDNewCircuit(&garbledCircuit, &garblingContext, s6_size*2, inpAND2, outAND2);

        for (j = 0; j < d; j++)
        {
                for (i = 0; i < ue; i++)
                        inpADD[i] = s6[(k+1)*s6_size+j*d+i];
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = outAND2[j*d+i];
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);

                for (i = 0; i < ue; i++)
                        s6[k*s6_size+j*d+i] = outADD[i];
        }
     }
   

     //step 8: sort tuples with respect to rows 1 and 3
     //we have separate code for this step


     //step 9
     for (k = qu+nu-2; k >= 0; k--)
     {
        for (j = 0; j  < d; j++)
        {
                for (i = 0; i < ue; i++)
                        inpMUL[i] = lambda[i];
                for (i = 0; i < ue; i++)
                        inpMUL[i+ue] = s5[k*s5_size+j*d+i];
                MULCircuit(&garbledCircuit, &garblingContext, ue*2, inpMUL, outMUL);

                for (i = 0; i < ue; i++)
                        inpAND2[j*d+i+s5_size] = outMUL[i];
        }

        inpNOT[0] = s3[k];
        NOTCircuit(&garbledCircuit, &garblingContext, 1, inpNOT, inpNOT);
        for (i = 0; i < s6_size; i++)
                inpAND2[i] = inpNOT[0];

        ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*2, inpAND2, outAND2);

        for (j = 0; j < d; j++)
        {
                for (i = 0; i < ue; i++)
                        inpADD[i] = s5[k*s5_size+j*d+i];
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = outAND2[j*d+i];
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);

                for (i = 0; i < ue; i++)
                        s6[k*s5_size+j*d+i] = outADD[i];
        }

        for (i = 0; i < s5_size; i++)
                inpAND2[i] = s5[(k+1)*s5_size+i];
        for (i = 0; i < s5_size; i++)
                inpAND2[i+s5_size] = s3[k+1];
     
        ANDNewCircuit(&garbledCircuit, &garblingContext, s5_size*2, inpAND2, outAND2);

        for (j = 0; j < d; j++)
        { 
                for (i = 0; i < ue; i++)
                        inpADD[i] = s5[(k+1)*s5_size+j*d+i];
                for (i = 0; i < ue; i++)
                        inpADD[i+ue] = outAND2[j*d+i];
                ADDCircuit(&garbledCircuit, &garblingContext, ue*2, inpADD, outADD);

                for (i = 0; i < ue; i++)
                        s5[k*s5_size+j*d+i] = outADD[i];
        }
     }     


     //step 10: iteration of step 3 tp step 9


     //step 11: sort tuples with respect to rows 3 and 2
     //we have separate code for this step

    //report output;
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

