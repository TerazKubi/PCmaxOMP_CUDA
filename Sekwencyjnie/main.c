#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <omp.h>


typedef struct {
    int pcmax;
    unsigned int* array;
} Solution;

#define POPULATION_SIZE 100
#define MAX_ITERATIONS 5000000
#define MAX_TIME 300

int processorsNum;
int tasksNum;
int* taskTimes;


int generateRandomInt(int min, int max) {
    return min + rand() % (max - min + 1);
}

void evaluateCmax(Solution* solution){
    int processorsSum[processorsNum];

    for(int i = 0; i < processorsNum; i++){
        processorsSum[i] = 0;
    }

    for(int i = 0; i < tasksNum; i++){
        processorsSum[solution->array[i] - 1 ] += taskTimes[i];
    }

    int maxProcessorTime = 0;
    for(int i = 0; i < processorsNum; i++){
        if(processorsSum[i] > maxProcessorTime){
            maxProcessorTime = processorsSum[i];
        }
    }
    solution->pcmax = maxProcessorTime;
}

void load_instance(FILE* file){
    fscanf(file, "%d", &processorsNum);
    fscanf(file, "%d", &tasksNum);

    taskTimes = malloc(sizeof(int) * tasksNum);

    int res;
    for(int i=0;  i< tasksNum; i++) {
        res = fscanf(file, "%d", &taskTimes[i]);
        if (res != 1) exit(1);
    }

    printf("P: %d \n", processorsNum);
    printf("T: %d \n", tasksNum);

    printf("\n");

}

void generateFirstSolution(Solution* sol){
    for(int i=0; i<tasksNum; i++){
        sol->array[i] = generateRandomInt(1, processorsNum);
    }
    evaluateCmax(sol);
}

void generateFirstPopulation(Solution* pop){
    for(int i=0; i<POPULATION_SIZE; i++){
        generateFirstSolution(&pop[i]);
    }
}

void getBestSolution(Solution* population, Solution* bestSolution, int iterations){
    int isNewFound = 0;
    for(int i = 0; i < POPULATION_SIZE; i++){
        if(population[i].pcmax < bestSolution->pcmax){
            bestSolution->pcmax = population[i].pcmax;
            memcpy(bestSolution->array, population[i].array, sizeof(int) * tasksNum);
            isNewFound = 1;
            
        }
    }

    if(isNewFound) {
        printf("\nNew best found at %d: \n ", iterations);
        printSol(bestSolution);
    }
}

int getBestParentIndex(Solution* population){
    int best = __INT_MAX__;
    int index = 0;
    for(int i=0; i<POPULATION_SIZE; i++){
        if(population[i].pcmax < best){
            best = population[i].pcmax;
            index = i;
        }
    }
    return index;
}

int getSecondBestParentIndex(Solution* population, int bestParentIndex){
    int best = __INT_MAX__;
    int index = 0;
    for(int i=0; i<POPULATION_SIZE; i++){
        if(i == bestParentIndex) continue;

        if(population[i].pcmax < best){
            best = population[i].pcmax;
            index = i;
        }
    }
    return index;
}

void crossOver(Solution* parent1, Solution* parent2, Solution* child){
    int crossPoint = generateRandomInt(0, tasksNum-1);
    
    for(int i = 0; i < tasksNum; ++i){
        if(i >= crossPoint){
            child->array[i] = parent2->array[i];
        } else {
            child->array[i] = parent1->array[i];
        }
    }

    evaluateCmax(child);
}

void mutate(Solution* sol){
    int randomIndex1 = generateRandomInt(0, tasksNum-1);
    int randomIndex2;

    do {
        randomIndex2 = generateRandomInt(0, tasksNum-1);
    } while (randomIndex1 == randomIndex2);

    int temp = sol->array[randomIndex1];
    sol->array[randomIndex1] = sol->array[randomIndex2];
    sol->array[randomIndex2] = temp;

    evaluateCmax(sol);
}

void mutatePopulation(Solution* population){
    for(int i=0; i<POPULATION_SIZE; i++){
        
        if(generateRandomInt(0, 100) > 5) continue;
        mutate(&population[i]);

    }
}

void generateNextPopulation(Solution* population){
    int p1 = getBestParentIndex(population);
    int p2 = getSecondBestParentIndex(population, p1);

    int toDieCount = POPULATION_SIZE * 0.2;
    
    while(toDieCount >= 0){
        int child;
        do{
            child = generateRandomInt(0, POPULATION_SIZE-1);
        } while ((child == p1) || (child == p2));

        crossOver(&population[p1], &population[p2], &population[child]);
        toDieCount--;
    }
}


int main(int argc, char *argv[]){

    FILE *file;

    int iterations = 0;
    int limit;

    if (argc < 2) {
        printf("No limit argument.\n");
        return 1;
    }
    limit = atoi(argv[1]);
    printf("Limit: %d\n", limit);

    file = fopen("m25n198.txt","r");
    load_instance(file);
    fclose(file);

    int seed = time(NULL);
    srand(seed);

    
    
    //prepare first population
    int* taskMem = malloc(sizeof(int) * tasksNum * POPULATION_SIZE);
    Solution population[POPULATION_SIZE];
    for(int i = 0; i < POPULATION_SIZE; ++i){
        population[i].array = &taskMem[i * tasksNum];
    }
    generateFirstPopulation(population);
    
    //preapre first best solution
    Solution bestSolution;
    bestSolution.array = malloc(sizeof(int) * tasksNum);
    bestSolution.pcmax = __INT_MAX__;
    getBestSolution(population, &bestSolution, iterations);

    

    const double startTime = omp_get_wtime();
    const double endTime = startTime + MAX_TIME;
    while ( (iterations <  MAX_ITERATIONS) && (omp_get_wtime() <= endTime) && (bestSolution.pcmax > limit)) {

        generateNextPopulation(population);
        mutatePopulation(population);

        getBestSolution(population, &bestSolution, iterations);
        
        iterations++;
    }

    const double globalTime = omp_get_wtime() - startTime;

    printf("\n\n final best: \n");
    printSol(&bestSolution);

    printf("Global time: %.1fs\n", globalTime);

    return 0;
}