#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/justGarble.h"

//number of shares
#define sh      2
//length of user ID
#define ul  	4
//length of item ID
#define il  	4
//number of queries per user
#define qu  	10
//length of ellements in user profile
#define ue  	36
//length of ellemenst in item profile
#define ie  	36
//length of user and item profiles
#define d  	10
//number of users
#define nu  	16
//number of items
#define ni  	16
//length of input, fo simplicity we assume that all users have the same number of quesries
#define n 	(sh*il*qu)*nu
//length of output
#define m 	1


int *final;

int main(int argc, char **argv) {
     srand(time(NULL));
    GarbledCircuit garbledCircuit;
    GarblingContext garblingContext;
    
    //Set up circuit parameters
    int q, r;
    q = 9000000;
    r = 9000000;
    
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
    
    int zero = fixedZeroWire(&garbledCircuit, &garblingContext);
    int one = fixedOneWire(&garbledCircuit, &garblingContext);

    int inp[1];
    inp[0] = one;
    int i, j, k, f;
   
    //XOR circuit to reconstruct item ID
    int inxor[il*2];
    int outxor[il];
    for (i = 0; i < il*2; i++)
        inxor[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < sh-1; k++)
                XORCircuit(&garbledCircuit, &garblingContext, il*2, inxor, outxor);

   
    //multiplexer implementation for each query of each user
    //first comput all NOT gates for multiplexer
    int innot[nu*qu*il];
    int outnot[nu*qu*il];
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < il; k++)
                innot[i*qu*il+j*il+k] = zero;

    NOTCircuit(&garbledCircuit, &garblingContext, nu*qu*il, innot, outnot);
   
    //all one bit AND gates for multiplexer
    int inand[2];
    int outand[1];
    inand[0] = zero;
    inand[1] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < ni; k++)
                for (f = 1; f < il; f++)
                    ANDNewCircuit(&garbledCircuit, &garblingContext, 2, inand, outand);

    //the second level of the AND gates
    int inand1[d*ie*2];
    int outand1[d*ie];
    for (i = 0; i < d*ie*2; i++)
        inand1[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < ni; k++)
                ANDNewCircuit(&garbledCircuit, &garblingContext, d*ie*2, inand1, outand1);

    //the last step of multiplexer which is all XOR gates
    int inxor1[d*ie*2];
    int outxor1[d*ie];
    for (i = 0; i < d*ie*2; i++)
        inxor1[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 1; k < ni; k++)
                XORCircuit(&garbledCircuit, &garblingContext, d*ie*2, inxor1, outxor1);
    
    //dot product circuits
    //first all dot product computations
    int inmul[ie*2];
    int outmul[ie*2];
    for (i = 0; i < ie*2; i++)
        inmul[i] = one;
    
    int inadd[ie*2];
    int outadd[ie];
    for (i = 0; i < ie*2; i++)
        inadd[i] = one;
    
    int insub[ie*2];
    int outsub[ie];
    for (i = 0; i < ie*2; i++)
        insub[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
        {
            for (k = 0; k < d; k++)
            {
                MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);
                if (k > 0)
                    ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd, outadd);
            }
            SUBCircuit(&garbledCircuit, &garblingContext, ie*2, insub, outsub);
            MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);
        }
    
   
    //scalar multiplication circuits
    int inmul1[ie*2];
    int outmul1[ie*2];
    for (i = 0; i < ie*2; i++)
        inmul1[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < d; k++)
                MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul1, outmul1);
    
    //all add circuits to update user profiles
    int inadd1[ie*2];
    int outadd1[ie];
    for (i = 0; i < ie*2; i++)
        inadd1[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < d; j++)
            for (k = 0; k < qu; k++)
                ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd1, outadd1);
   
    //all add circuits to update item profiles
    //De-multiplexer can be implemented by multiplexer code, we can use the pre-computed first layer of NOT gates and AND gates and we only neet to add one layer of AND gates
    int inand2[ie*2];
    int outand2[ie];
    for (i = 0; i < ie*2; i++)
        inand2[i] = one;
    
    int inmul2[ie*2];
    int outmul2[ie*2];
    for (i = 0; i < ie*2; i++)
        inmul2[i] = one;
    
    for (i = 0; i < ni; i++)
        for (j = 0; j < nu; j++)
            for (k = 0; k < qu; k++)
                ANDNewCircuit(&garbledCircuit, &garblingContext, ie*2, inand2, outand2);
                for (f = 0; f < d; f++)
                    MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul2, outmul2);
   
    //all add circuits to update item profiles
    int inadd2[ie*2];
    int outadd2[ie];
    for (i = 0; i < ie*2; i++)
        inadd2[i] = one;
    for (i = 0; i < ni; i++)
        for (j = 0; j < d; j++)
            for (k = 0; k < qu*nu; k++)
                ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd2, outadd2);
    
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

