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
    
    //all inputs in order of A, B, C, D
    int zero = fixedZeroWire(&garbledCircuit, &garblingContext);
    int one = fixedOneWire(&garbledCircuit, &garblingContext);
    int inp[n];
    int i, j, k, f, check;;

    for (i = 0; i < n; i++)
        if ((i+1)%(3)==0)
        	inp[i] = one;
        else if (i%2 == 0)
            inp[i] = zero;
        else
            inp[i] = one;

    
    //XOR circuit to reconstruct item ID
    int inxor[il*2];
    int outxor[il];
    int itemID[nu][qu][il];

    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
        {
            check = 0;
            for (k = 0; k < sh-1; k++)
            {
                if (check == 0)
                    for (f = 0; f < il; f++)
                    {
                        inxor[f] = inp[i*sh*il*qu+k*il+f];
                        inxor[f+il] = inp[i*sh*il*qu+(k+1)*il+f];
                    }
                else
                {
                    for (f = 0; f < il; f++)
                    {
                        inxor[f] = outxor[f];
                        inxor[f+il] = inp[i*sh*il*qu+(k+1)*il+f];
                    }
                }
                
                XORCircuit(&garbledCircuit, &garblingContext, il*2, inxor, outxor);
                check = 1;    
            }
            for (f = 0; f < il; f++)
                itemID[i][j][f] = outxor[f];
        }
   
    //multiplexer implementation for each query of each user
    //first comput all NOT gates for multiplexer
    int innot[nu*qu*il];
    int outnot[nu*qu*il];
    int itemIDNOT[nu][qu][il];
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < il; k++)
                innot[i*qu*il+j*il+k] = itemID[i][j][k];

    NOTCircuit(&garbledCircuit, &garblingContext, nu*qu*il, innot, outnot);
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < il; k++)
                itemIDNOT[i][j][k]= innot[i*qu*il+j*il+k];
   
    //all one bit AND gates for multiplexer
    int inand[2];
    int outand[1];
    int itemIDAND[nu][qu][ni][1];
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < ni; k++)
            {
                inand[0] = itemID[i][j][0]; // in the performance of the protocol, the contant of wires is not important and the performance is contant independent
                for (f = 1; f < il; f++)
                {
                    innot[1] = itemID[i][j][f];
                    ANDNewCircuit(&garbledCircuit, &garblingContext, 2, inand, outand);
                    inand[0] = outand[0];
                }
                itemIDAND[i][j][k][0] = outand[0];
            }
    
    //the second level of the AND gates
    int inand1[d*ie*2];
    int outand1[d*ie];
    int itemPM[nu][qu][ni][d*ie];
    //random item profile, in the performance of the protocol, the contant of wires is not important and the performance is contant independent
    for (i = 0; i < d*ie; i++)
        if ((i)%(3)==0)
            inand1[i] = one;
        else if (i%2 == 0)
            inand1[i] = zero;
        else
            inand1[i] = one;
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
            for (k = 0; k < ni; k++)
            {
                for (f = 0; f < d*ie; f++)
                {
                    inand1[d*ie+f] = itemIDAND[i][j][k][0];
                }
                ANDNewCircuit(&garbledCircuit, &garblingContext, d*ie*2, inand, outand);
                for (f = 0; f < d*ie; f++)
                {
                    itemPM[i][j][k][f] = outand[f];
                }
            }
  
    //the last step of multiplexer which is all XOR gates
    int inxor1[d*ie*2];
    int outxor1[d*ie];
    int itemP[nu][qu][d*ie];
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
        {
            for (f = 0; f < d*ie; f++)
            {
                inxor1[f] = itemPM[i][j][0][f];
            }
            for (k = 1; k < ni; k++)
            {
                for (f = 0; f < d*ie; f++)
                {
                    inxor1[d*ie+f] = itemPM[i][j][k][f];
                }
                XORCircuit(&garbledCircuit, &garblingContext, d*ie*2, inxor1, outxor1);
                for (f = 0; f < d*ie; f++)
                {
                    inxor1[f] = outxor1[f];
                }
            }
            for (f = 0; f < d*ie; f++)
            {
                itemP[i][j][f] = outxor1[f];
            }
        }
    
    //dot product circuits
    //first all dot product computations
    int inmul[ie*2];
    int outmul[ie*2];
    int DotP[nu][qu][ie]; //assume that l+ie = 2*ie
    //random user profile, in the performance of the protocol, the contant of wires is not important and the performance is contant independent
    for (i = 0; i < ie; i++)
        if ((i)%(3)==0)
            inmul[i] = one;
        else if (i%2 == 0)
            inmul[i] = zero;
        else
            inmul[i] = one;
    
    int inadd[ie*2];
    int outadd[ie];
    
    int insub[ie*2];
    int outsub[ie];
    for (i = 0; i < ie-1; i++)
        insub[i] = zero;
    insub[ie-1] = one;
    
    
    
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
        {
            for (k = 0; k < d; k++)
            {
                for (f = 0; f < ie; f++)
                    inmul[ie+f] = itemP[i][j][k*ie+f];
                MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);

                if (k == 0)
                {
                    for (f = 0; f < ie; f++)
                        inadd[f]= outmul[f];
                }
                else
                {
                    for (f = 0; f < ie; f++)
                        inadd[ie+f]= outmul[f];
                    ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd, outadd);
                    for (f = 0; f < ie; f++)
                        inadd[f]= outadd[f];
                }
            }
            
            for (f = 0; f < ie; f++)
                insub[ie+f] = outadd[f];
            
            SUBCircuit(&garbledCircuit, &garblingContext, ie*2, insub, outsub);
            
            for (f = 0; f < ie; f++)
                inmul[ie+f] = outsub[f];
            MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);
            for (f = 0; f < ie; f++)
                DotP[i][j][f] = outmul[f];
        }
        
    //scalar multiplication circuits
    int ScalarM[nu][qu][d*ie];
    for (i = 0; i < nu; i++)
        for (j = 0; j < qu; j++)
        {
            for (f = 0; f < ie; f++)
                inmul[f] = DotP[i][j][f];
            for (k = 0; k < d; k++)
            {
                for (f = 0; f < ie; f++)
                    inmul[ie+f] = itemP[i][j][k*ie+f];
                MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);
                for (f = 0; f < ie; f++)
                    ScalarM[i][j][i*ie+f] = outmul[f];
            }
        }
    
    //all add circuits to update user profiles
    int userPU[nu][d*ie];
    for (i = 0; i < nu; i++)
        for (j = 0; j < d; j++)
        {
            for (k = 0; k < qu; k++)
            {
                ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd, outadd);
            }
            for (k = 0; k < ie; k++)
                userPU[i][j*ie+k] = outadd[k];
        }
    
    
    
    //all add circuits to update item profiles
    //De-multiplexer can be implemented by multiplexer code, we can use the pre-computed first layer of NOT gates and AND gates and we only neet to add one layer of AND gates
    int itemPU[ni][d*ie];
    int inand2[ie*2];
    int outand2[ie];
    
    for (i = 0; i < ni; i++)
        for (j = 0; j < nu; j++)
            for (k = 0; k < qu; k++)
            {
                for (f = 0; f < ie; f++)
                    inand2[f] = itemIDAND[j][k][i][0];
                for (f = 0; f < ie; f++)
                    inand2[ie+f] = DotP[j][k][f];
                ANDNewCircuit(&garbledCircuit, &garblingContext, ie*2, inand2, outand2);
                
                for (f = 0; f < ie; f++)
                    inmul[f] = outand2[f];
                for (f = 0; f < d; f++)
                {
                    MULCircuit(&garbledCircuit, &garblingContext, ie*2, inmul, outmul);
                }
            }
    
    //all add circuits to update item profiles
    for (i = 0; i < ni; i++)
        for (j = 0; j < d; j++)
        {
            for (k = 0; k < qu; k++)
            {
                ADDCircuit(&garbledCircuit, &garblingContext, ie*2, inadd, outadd);
            }
            for (k = 0; k < ie; k++)
                itemPU[i][j*ie+k] = outadd[k];
        }

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

