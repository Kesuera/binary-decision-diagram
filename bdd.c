/*
 * Autor: Samuel Hetteš
 * Predmet: Dátové šturktúry a algoritmy
 * Zadanie: Binárne rozhodovacie diagramy
 * Prostredie: Visual Studio Code
 * Dátum: 26.4.2021
*/


/*HEADERS*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "bdd.h"


/*FUNCTIONS*/


//function to recursively free allocated memory of nodes
void free_diagram(BDDNODE *node){
    if(!node)    //if node is NULL, return  
        return;

    for(int i = 0; i < node->parentsAmmount; i++){    //disconnect node from its parents
        if(node->parents[i]){
            if(node->parents[i]->falseNode == node)
                node->parents[i]->falseNode = NULL;
            if(node->parents[i]->trueNode == node)
                node->parents[i]->trueNode = NULL;
        }
    }

    free_diagram(node->falseNode);    //free left sub-tree
    free_diagram(node->trueNode);    //free right sub-tree

    free(node->parents);    //free parents
    free(node->vector);    //free vector
    free(node);    //free the node
    node = NULL;
}


//function to free allocated memory of BDD
void BDD_free(BDD *bdd){
    if(!bdd)    //if bdd is NULL, return
        return;

    free_diagram(bdd->root);   //call function to free nodes
    free(bdd);    //free bdd
    bdd = NULL;
}


//function to check if boolean function is valid
int check_function(BF *bfunction){
    if(!bfunction || !bfunction->vector){    //if bf is NULL or its vector is NULL, return 0
        printf("Invalid boolean function\n");
        return 0;
    }

    int length = 0;    //length of the vector
    int variables = 0;    //ammount of vars of boolean function
    int c;

    while((c = bfunction->vector[length])){    //check all the symbols of vector, have to be 0 or 1
        if(c != '0' && c != '1'){
            printf("Invalid boolean function\n");
            return 0;
        }
        length++;
    }

    while(length != 1){    //if length is not 1
            if(length & 1){    //check if length is even
                printf("Invalid boolean function\n");
                return 0;
            }
            length >>= 1;    //divide length by 2
            variables++;    //increment ammount of vars
        }

    return variables;    //return vars
}


//function to recursively connect nodes that are on the same level
void connect_neighbours(BDDNODE *root){
    if(!root)    //if node is NULL, return
        return;
    
    if(root->falseNode)    //connect left node with right one
        root->falseNode->rightNeighbour = root->trueNode;

    if(root->trueNode){    //connect right node with left node of next neighbour of parent
        if(root->rightNeighbour)
            root->trueNode->rightNeighbour = root->rightNeighbour->falseNode;
    }

    connect_neighbours(root->falseNode);    //connect left sub-tree
    connect_neighbours(root->trueNode);    //connect right sub-tree
}


//function to create new node
//hash function source: http://www.cse.yorku.ca/~oz/hash.html
BDDNODE *create_node(char *vector, int level, unsigned long hash){
    BDDNODE *node = (BDDNODE *) malloc(sizeof(BDDNODE));    //allocate memory for node

    if(!node){    //if allocation fails, return NULL
        printf("Not enough memory to create bdd node\n");
        return NULL;
    }

    node->vector = vector;    //set parameters
    node->trueNode = NULL;
    node->falseNode = NULL;
    node->rightNeighbour = NULL;
    node->level = level;

    node->parents = (BDDNODE **) malloc(sizeof(BDDNODE *));    //allocate memory for parents pointers

    if(!node->parents){
        printf("Not enough memory to create bdd node\n");
        free(node);
        return NULL;
    }

    node->parents[0] = NULL;
    node->hash = hash;
    node->parentsAmmount = 0;

    return node;    //return new node
}


//function to recursively create bdd tree
BDDNODE *create_diagram(BDDNODE *root, char *vector, int level){
    int new_length = 1 << (level - 1);    //calculate length of vector for left and right nodes
    char *vector_false, *vector_true;    
    
    if(level != 0){    //if node is not a leaf
        vector_false = (char *) malloc(new_length + 1);    //allocate memory for left vector
        vector_true = (char *) malloc(new_length + 1);    //allocate memory for right vector

        if(!vector_false || !vector_true){    //check if allocation was succesful
            printf("Not enough memory to create next node\n");
            free(vector_true);
            free(vector_false);
            return root;
        }

        vector_false[new_length] = 0;    //set end terminators
        vector_true[new_length] = 0;
    }

    unsigned long hash = 5381;    //starting hash number
    int c, i = 0, j = 0;    //temp variables
    int counter = 0;    //counter of vector index

    //cycle to hash original vector and split vector
    while ((c = vector[counter])){
        if(level != 0){    //if node is not a leaf
            if(counter < new_length)
                vector_false[i++] = c;    //copy first half of vector to vector false
            else
                vector_true[j++] = c;    //copy second half of vector to vector true
        }
        counter++;    //increment counter
        hash = ((hash << 5) + hash) + c;    /* hash * 33 + c */
    }

    root = create_node(vector, level, hash);    //create node
    level--;    //decrement level

    if(!root){    //if creation fails, free vector and return NULL
        free(vector);
        return NULL;
    }

    if(level == -1)    //if leaf was reached, return root
        return root;

    root->falseNode = create_diagram(root->falseNode, vector_false, level);    //create bdd for left sub-tree
    root->trueNode = create_diagram(root->trueNode, vector_true, level);    //create bdd for right sub-tree

    root->falseNode->parents[root->falseNode->parentsAmmount++] = root;    //set first parent of new nodes
    root->trueNode->parents[root->trueNode->parentsAmmount++] = root;

    return root;    //return root
}


//function to create new BDD
BDD *BDD_create(BF *bfunction){
    int variables = check_function(bfunction);    //check boolean function

    if(!variables)    //if vars equals 0, return NULL
        return NULL;

    BDD *bdd = (BDD *) malloc(sizeof(BDD));    //allocate memory for bdd struct

    if(!bdd){    //check if allocation was successful
        printf("Not enough memory to create BDD\n");
        return NULL;
    }

    bdd->variables = variables;    //set vars
    bdd->nodes = (2 << variables) - 1;    //calculate ammount of nodes = 2^vars - 1
    bdd->root = NULL;

    int length = 1 << variables;    //length of root vector

    char *vector = (char *) malloc(length + 1);    //allocate space for root vector
        
    if(!vector){    //check allocation
        printf("Not enough memory to create node\n");
        free(bdd);
        return NULL;
    }

    memcpy(vector, bfunction->vector, length);    //copy root vector
    vector[length] = 0;    //set end terminator

    bdd->root = create_diagram(bdd->root, vector, variables);    //create bdd tree

    if(bdd->root){    //if root is NULL, return NULL, else connect neighbouring nodes
        connect_neighbours(bdd->root);
    }
    else{
        free(bdd);
        return NULL;
    }

    return bdd;    //return bdd
}


int reduce_I(BDDNODE *comparedNode, BDDNODE *redundantNode){
    int redundant_parent = 0;    //variable to determine whether redudant nodes parent is not redundant as well

    if(redundantNode->parents[0]->falseNode == redundantNode)    //if parents left child is redundant node
        redundantNode->parents[0]->falseNode = comparedNode;    //set new child for parent  

    if(redundantNode->parents[0]->trueNode == redundantNode)    //if parents right child is redundant node, do the same
        redundantNode->parents[0]->trueNode = comparedNode;

    //check if redundant node parents vector is not the same as parents vector of compared node
    for(int i = 0; i < comparedNode->parentsAmmount; i++){
        if(comparedNode->parents[i]->hash == redundantNode->parents[0]->hash){
            if(!strcmp(comparedNode->parents[i]->vector, redundantNode->parents[0]->vector)){
                redundant_parent = 1;
                break;
            }
        }
    }
    
    //if the parent is unique, reallocate memory and add him, else we dont add this parent because it will get removed anyway
    if(!redundant_parent){
        comparedNode->parents = (BDDNODE **) realloc(comparedNode->parents, (comparedNode->parentsAmmount + 1) * sizeof(BDDNODE *));
                        
        if(!comparedNode->parents){
            printf("Reduction of bdd failed\n");
            return 0;
        }

        comparedNode->parents[comparedNode->parentsAmmount++] = redundantNode->parents[0];
    }

    free(redundantNode->vector);    //free vector
    free(redundantNode->parents);    //free parents
    free(redundantNode);    //free redundant node
    redundantNode = NULL;

    return 1;
}


int reduce_S(BDD *bdd, BDDNODE *redundantNode){
    if(redundantNode == bdd->root)    //if redundant node was the root node
        bdd->root = redundantNode->falseNode;    //set the new root

    //find and replace redundant nodes child parent
    for(int i = 0; i < redundantNode->falseNode->parentsAmmount; i++){
        if(redundantNode->falseNode->parents[i] == redundantNode){
            redundantNode->falseNode->parents[i] = redundantNode->parents[0];
            break;
        }
    }

    if(redundantNode->parents[0]){    //set new child for the redundant nodes parent
        if(redundantNode->parents[0]->falseNode == redundantNode)
            redundantNode->parents[0]->falseNode = redundantNode->falseNode;
        if(redundantNode->parents[0]->trueNode == redundantNode)   
            redundantNode->parents[0]->trueNode = redundantNode->falseNode;
    }

    for(int i = 1; i < redundantNode->parentsAmmount; i++){    //disconnect redundant node from the rest of the parents

        //disconnect redundant node from parent and set new child
        if(redundantNode->parents[i]->falseNode == redundantNode)
            redundantNode->parents[i]->falseNode = redundantNode->falseNode;
        if(redundantNode->parents[i]->trueNode == redundantNode)   
            redundantNode->parents[i]->trueNode = redundantNode->falseNode;

        //realloc memory for new parent and add him
        redundantNode->falseNode->parents = (BDDNODE **) realloc(redundantNode->falseNode->parents, (redundantNode->falseNode->parentsAmmount + 1) * sizeof(BDDNODE *));
                            
        if(!redundantNode->falseNode->parents){
            printf("Reduction of bdd failed\n");
            return 0;
        }

        redundantNode->falseNode->parents[redundantNode->falseNode->parentsAmmount++] = redundantNode->parents[i];
    }

    free(redundantNode->vector);    //free vector            
    free(redundantNode->parents);    //free parents
    free(redundantNode);    //free redundant node
    redundantNode = NULL;

    return 1;
}


//function to reduce ammount of nodes in bdd
int BDD_reduce(BDD *bdd){
    if(!bdd || !bdd->root || !bdd->root->vector){    //check if bdd is valid
        printf("Reduction of bdd failed\n");
        return -1;
    }

    BDDNODE *leftNode = bdd->root;    //bottom left node

    while(leftNode->falseNode)    //move in tree until bottom left node is encountered
        leftNode = leftNode->falseNode;

    BDDNODE *comparedNode, *redundantNode, *temp, *temp_2;

    int deleted_nodes = 0;    //variable to keep track of deleted nodes
    int level_moved = 0;    //variable to determine whether level was already moved or not

    //reduction of nodes
    while(leftNode){    //while left node is not NULL
        comparedNode = leftNode;
        temp_2 = NULL;

        //reduction of whole level
        while(comparedNode){    //while whole level was not reducted
            temp = comparedNode;

            //reduction of type I
            while(temp->rightNeighbour){    //compare node with all its neighbours, till we reach right end node
                if(comparedNode->hash == temp->rightNeighbour->hash){    //if vector hashes are the same
                    if(!strcmp(comparedNode->vector, temp->rightNeighbour->vector)){    //check if vectors are the same
                        redundantNode = temp->rightNeighbour;    //save redundant node
                        temp->rightNeighbour = redundantNode->rightNeighbour;    //disconnect redundant node in level
                            
                        //call function to do reduction of type I
                        if(!reduce_I(comparedNode, redundantNode)){    //if return value was 0, there was a mistake, return -1
                            bdd->nodes -= deleted_nodes;
                            return -1;
                        }

                        deleted_nodes++;    //increment ammount of deleted nodes
                    }
                }
                else
                    temp = temp->rightNeighbour;    //else compare node with next node in level
            }

            //reduction of type S
            if(comparedNode->falseNode && comparedNode->falseNode == comparedNode->trueNode){    //if childrens are equal
                redundantNode = comparedNode;    //save redundant node
                comparedNode = redundantNode->rightNeighbour;    //move to next neighbour

                if(temp_2)    //disconnect redundant node in level and change neighbour of the previous
                    temp_2->rightNeighbour = redundantNode->rightNeighbour;

                if(leftNode == redundantNode){    //if redundant node is the left node move level
                    leftNode = leftNode->parents[0]; 
                    level_moved = 1;    //set variable to 1, to determine whether level was already moved
                }

                //call function to do reduction of type S
                if(!reduce_S(bdd, redundantNode)){    //if return value was 0, there was a mistake, return -1
                    bdd->nodes -= deleted_nodes;
                    return -1;
                }

                deleted_nodes++;    //increment ammount of deleted nodes
            }
            else{
                temp_2 = comparedNode;    //save previous node
                comparedNode = comparedNode->rightNeighbour;    //move to next node
            }
        }
        if(level_moved)    //if level was moved set variable to 0
            level_moved = 0;
        else    //else move level
            leftNode = leftNode->parents[0];
    }
    
    bdd->nodes -= deleted_nodes;    //set new ammount of nodes in bdd
    
    return deleted_nodes;    //return ammount of deleted nodes
}


//function to use bdd based on input
char BDD_use(BDD *bdd, char *input){
    if(!input || !bdd || !bdd->root || !bdd->root->vector)    //check pointers, if not valid return -1
        return -1;

    int inputLength = bdd->variables;    //what input length should be
    
    BDDNODE *root = bdd->root;    //get root
    int counter = 0;    //input variable counter
    
    while(inputLength != 0){    //while leaf was not reached
        if(!root)   //if node is NULL, return -1
            return -1;

        if(root->level != inputLength){    //if level is not equal with remaining input length, skip variable
            if(input[counter] != '0' && input[counter] != '1')    //if input is not correct return -1
                return -1;
            counter++;    //move to next variable
        }
        else{    //else check the value
            if(input[counter] == '0'){    //if value is 0, move to left sub-tree
                root = root->falseNode;
                counter++;   //move to next variable
            }
            else if(input[counter] == '1'){    //else move to right sub-tree
                root = root->trueNode;
                counter++;    //move to next variable
            }
            else    //if input was not '0' or '1' return -1
                return -1;
        }
        inputLength--;    //decrement remaining input length
    }

    if(input[counter] != 0)    //if end of input was not reached, return -1
        return -1;

    return root->vector[0];
}