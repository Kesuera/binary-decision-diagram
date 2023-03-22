/*
 * Autor: Samuel Hetteš
 * Predmet: Dátové šturktúry a algoritmy
 * Zadanie: Binárne rozhodovacie diagramy
 * Prostredie: Visual Studio Code
 * Dátum: 26.4.2021
*/


/*HEADERS*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "bdd.h"

//function to do partition
//source: https://www.geeksforgeeks.org/quick-sort/
int partition(double *array, int start, int end){
    double pivot = array[end];
    int smaller = start - 1;

    for(int i = start; i <= end - 1; i++){
        if(array[i] < pivot){
            smaller++;
            double temp = array[i];
            array[i] = array[smaller];
            array[smaller] = temp; 
        }
    }

    double temp = array[smaller + 1];
    array[smaller + 1] = array[end];
    array[end] = temp;

    return smaller + 1;
}


//function to quick sort given double array
//source: https://www.geeksforgeeks.org/quick-sort/
void quick_sort(double *array, int start, int end){
    if(start < end){
        int part = partition(array, start, end);

        quick_sort(array, start, part - 1);
        quick_sort(array, part + 1, end);
    }
}

/*TESTER*/

void tester(int variables, int bf_ammount){
    BDD *bdd = NULL;    //bdd pointer
    BF *bfunction = (BF *) malloc(sizeof(BF));    //boolean function struct pointer

    int vector_length = 1 << variables;    //length of vector

    clock_t start, end;    //variables to measure time
    double reduced = 0;    //reduction rate variable
    double create_time = 0;    //time taken by operations variables
    double reduce_time = 0;
    double use_time = 0;

    char *vector = (char *) malloc(vector_length + 1);    //allocate space for vector
    char *input = (char *) malloc(variables + 1);    //allocate space for input
    double *reduction_stats = (double *) calloc(bf_ammount, sizeof(double));    //reduction rate statistics array

    if(!bfunction || !vector || !input || !reduction_stats){    //check if allocation was succesful
        printf("Not enough memory to do testing\n");
        free(bfunction);
        free(vector);
        free(input);
        free(reduction_stats);
        return;
    }

    int originally_nodes = 0;

    for(int counter = 0; counter < bf_ammount; counter++){    //cycle that repeats depending on number of generated boolean functions
        for(int i = 0; i < vector_length; i++){    //generator of random vector
            int rand_bool = rand() & 1;
            if(rand_bool)
                vector[i] = '1';
            else   
                vector[i] = '0';
        }

        vector[vector_length] = 0;
        bfunction->vector = vector;    //set bfunction vector

        start = clock();
        bdd = BDD_create(bfunction);    //create bdd for the boolean function and track time
        end = clock();

        create_time += ((double) (end - start));    //add tracked time

        originally_nodes = bdd->nodes;    //ammount of nodes that originally were in bdd

        start = clock();
        int deleted_nodes = BDD_reduce(bdd);    //reduce bdd and save ammount of deleted nodes
        end = clock();

        reduce_time += ((double) (end - start));    //add tracked time

        if(deleted_nodes == -1){    //if deleted nodes is -1, there was an error, free and return
            printf("ERROR\n");
            BDD_free(bdd);
            free(vector);
            free(input);
            free(bfunction);
            return;
        }

        //generate inputs, starting from 0
        for(int index = 0; index < vector_length; index++){
            int decimal = index;    //decimal = input 
            int input_length = variables;    //length of input

            while(decimal){    //while decimal is not 0
                if(decimal & 1)    //add '1' or '0' based on last bit of decimal
                    input[input_length - 1] = '1';
                else
                    input[input_length - 1] = '0';
                
                decimal >>= 1;    //divide by two
                input_length--;    //decrement input length
            }

            while(input_length){    //if all symbols were not set, it means that rest of them are 0, set them
                input[input_length - 1] = '0';
                input_length--;
            }

            input[variables] = 0;

            start = clock();
            char output = BDD_use(bdd, input);    //call use for the input and save return value
            end = clock();

            use_time += ((double) (end - start));    //add tracked time

            if(output != bfunction->vector[index])    //if the output was not correct, print message
                printf("Wrong output\n");
            
        }

        BDD_free(bdd);    //free bdd
        reduction_stats[counter] = ((double)deleted_nodes / (double)originally_nodes) * 100;    //calculate reduction rate and add into array
        reduced += reduction_stats[counter];    //add reduce rate 
    }

    reduced = reduced / bf_ammount;   
    create_time = create_time / CLOCKS_PER_SEC;    //calculate time in seconds
    reduce_time = reduce_time / CLOCKS_PER_SEC;
    use_time = use_time / CLOCKS_PER_SEC;

    quick_sort(reduction_stats, 0, bf_ammount - 1);    //call quicksort to sort array

    //calculate median
    double median = (bf_ammount & 1) ? reduction_stats[bf_ammount/2] : ((reduction_stats[bf_ammount/2] + reduction_stats[bf_ammount/2 - 1]) / 2);

    //print results
    printf("---------------------------------------------------------\n");
    printf("-------------------- TEST PARAMETERS --------------------\n");
    printf("---------------------------------------------------------\n");
    printf("-> variables: %d\n", variables);
    printf("-> generated boolean functions: %d\n\n", bf_ammount);
    printf("---------------------------------------------------------\n");
    printf("--------------------- TEST RESULTS ----------------------\n");
    printf("---------------------------------------------------------\n");
    printf("REDUCTION RATE:\n");
    printf("-> AVERAGE: %.2lf%%\n", reduced);
    printf("-> MEDIAN: %.2lf%%\n", median);

    //cycle that prints ammount of bdds whose reduction rate integer base is the same
    int index = 0;
    while(index < bf_ammount){
        double rate = (int)reduction_stats[index];
        while(rate <= reduction_stats[index])
            rate += 0.05;
        
        rate -= 0.05;
        int bdds = 0;
        while(index < bf_ammount && reduction_stats[index] < rate + 0.05){
            bdds++;
            index++;
        }
        printf("-> [%.2lf%% - %.2lf%%]: %d\n", rate, rate + 0.05, bdds);
    }   

    printf("\n");
    printf("AVERAGE TIME:\n");
    printf("-> CREATE: %lfs\n", create_time / bf_ammount);
    printf("-> REDUCE: %lfs\n", reduce_time / bf_ammount);
    printf("-> USE: %lfs\n\n", use_time / (bf_ammount * vector_length));
    printf("TOTAL TIME:\n");
    printf("-> CREATE: %lfs\n", create_time);
    printf("-> REDUCE: %lfs\n", reduce_time);
    printf("-> USE: %lfs\n", use_time);
    printf("-> ALL: %lfs\n", create_time + reduce_time + use_time);
    printf("---------------------------------------------------------\n");

    free(vector);    //free allocated memory
    free(input);
    free(bfunction);
    free(reduction_stats);
}

int main(void){
    srand(time(NULL));    //initialise seed

    int variables = 13;
    int bf_ammount = 2000;

    tester(variables, bf_ammount);

    return 0;
}