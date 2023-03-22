#ifndef TEST_BDD
#define TEST_BDD


/*DEFINED TYPES*/

//node structure
typedef struct bdd_node{
    char *vector;    //vector
    unsigned long hash;    //vector hash
    int level;    //node level
    struct bdd_node *falseNode;   //pointer to node when var is false
    struct bdd_node *trueNode;    //pointer to node when var is true
    struct bdd_node *rightNeighbour;    //pointer to neighbour in the level
    struct bdd_node **parents;    //pointer to an array of parents
    int parentsAmmount;    //parents counter
} BDDNODE;

//bdd structure
typedef struct bdd{
    int variables;    //ammount of variables
    int nodes;    //ammount of nodes
    struct bdd_node *root;    //pointer to root node
} BDD;

//boolean function structure
typedef struct bf{
    char *vector;    //pointer to vector
} BF;


/*FUNCTIONS*/

void BDD_free(BDD *bdd);
BDD *BDD_create(BF *bfunction);
int BDD_reduce(BDD *bdd);
char BDD_use(BDD *bdd, char *input);

#endif