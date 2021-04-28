/****************************************************************************
 * @file    proj2.c
 * @brief   IOS - Operating Systems - 2nd project - Santa Claus Problem
 * @author  Tadeáš Kachyňa <xkachy00@stud.fit.vutbr.cz>
 * @date    4/5/2021
 * @version 1.0
 ****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/wait.h>
#include <sys/shm.h> 
#include <pthread.h>
#include <sys/mman.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

/********** Constants **********/

#define NUM_OF_ARGS 5
#define ARGS_FAIL 1
#define MAX_REINDEERS 19
#define MIN_REINDEERS 1
#define MAX_ELVES 999
#define MIN_ELVES 1
#define MIN_TIME 0
#define MAX_TIME 1000

/********** Semaphores **********/

#define SEM_SANTASEM    "/xkachy00-ios2-santaSem"
#define SEM_MUTEX       "/xkachy00-ios2-mutex"
#define SEM_REINDEERSEM "/xkachy00-ios2-reindeerSem"
#define SEM_ELFTEX      "/xkachy00-ios2-elfTex"
#define SEM_ELFTEX2     "/xkachy00-ios2-elfTex2"
#define SEM_ALL         "/xkachy00-ios2-all"
#define SEM_GETHITCHED  "/xkachy00-ios2-getHitched"
#define SEM_SANTAHELPED "/xkachy00-ios2-santaHelped"

sem_t *santaSem = NULL;
sem_t *mutex = NULL;
sem_t *reindeerSem = NULL;
sem_t *elfTex = NULL;
sem_t *elfTex2 = NULL;
sem_t *allProcesses = NULL;
sem_t *getHitched = NULL;
sem_t *santaHelped = NULL;

/********** Shared memories **********/

static int *actionCounter;   
static int *reindeerCounter;
static int *elvesCounter;
static int *elvesHolidays;

FILE *file;

/** @struct args users arguments
 * 
 * @var NE number of elves
 * @var NR number of reeinders
 * @var TE max. time in milliseconds for which the elf works independently
 * @var TR max. time in milliseconds for which the reindeer is on its vacation
 */
typedef struct {
    int NE;
    int NR;
    int TE;
    int TR;
} args;

void destroy();
/**
 * @name Reindeer function
 * 
 * @brief reeinder process
 * 
 * @param NR number of reeindeers
 * @param TR max. time in milliseconds for which the reindeer works independently
 * @param i index
 */
void processReindeer(int NR, int TR, int i) 
{   
    srand(time(NULL));
 
    int lower = TR / 2; // lower border
    int upper = TR;     // upper border 
    int ran;

    ran = (rand() % (upper - lower + 1) + lower); // gives you random value from interval <TR/2,TR >
    ran = ran * 1000; 

    sem_wait(mutex);
        fprintf(file, "%d: RD %d: rstarted\n", *actionCounter, i);
        fflush(file);
        (*actionCounter)++;
    sem_post(mutex);

    usleep(ran);

    sem_wait(mutex);
        fprintf(file, "%d: RD %d: return home\n", *actionCounter, i);
        fflush(file);
        (*actionCounter)++;
        (*reindeerCounter)++;
    sem_post(mutex);

    if(*reindeerCounter == NR) 
    {   
        sem_post(santaSem);    
    }
 
    sem_wait(reindeerSem);

    sem_wait(mutex);     
        fprintf(file, "%d: RD %d: get hitched\n", *actionCounter, i); 
        fflush(file);
        sem_post(getHitched);
        (*actionCounter)++;
        (*reindeerCounter)--;
       
    sem_post(mutex);

    sem_post(allProcesses);

    exit(0);        
}
    
/**
 * @name Elves function
 * 
 * @brief elves process
 * 
 * @param NE number of reeindeers
 * @param TE max. time in milliseconds for which the reindeer works independently
 */
int processElves(int TE, int i)
{  
    int ran;
       
    sem_wait(mutex);
        fprintf(file, "%d: Elf %d: started\n", *actionCounter, i);
        fflush(file);
        (*actionCounter)++;
    sem_post(mutex);
    
    for(;;) {
        
        if(TE != 0) 
        {
            srand(time(NULL));
            ran = (rand() % TE);
            ran = ran * 1000;      
            usleep(ran);
        }
          //fprintf(file, "test a = %d\n", i);
        sem_wait(elfTex); //-----------------------------------//

        sem_wait(mutex);
            fprintf(file, "%d: Elf %d: need help\n", *actionCounter, i);   
            (*actionCounter)++;
            (*elvesCounter)++;
            fflush(file);
        sem_post(mutex); 

        if (*elvesCounter == 3 && *elvesHolidays == 0) 
        {
            sem_post(santaSem);

        } else {

            sem_post(elfTex); 
        } 
      
        sem_wait(elfTex2);

        sem_wait(mutex);
       
         if (*elvesHolidays == 1) 
        {
            fprintf(file, "%d: Elf %d: taking holidays\n", *actionCounter, i);
            (*actionCounter)++;
            
            fflush(file);
            sem_post(elfTex);
            sem_post(mutex);
            sem_post(allProcesses);
            
            exit(1);   
        }


       
        sem_post(mutex);

        
        sem_wait(mutex);
        fprintf(file, "%d: Elf %d: get help\n", *actionCounter, i);
        fflush(file);
        (*actionCounter)++;
        
         sem_post(santaHelped);
         
        (*elvesCounter)--;

        if (*elvesCounter == 0) {
            sem_post(elfTex);
        }
       
        sem_post(mutex);
      
    }

}

/**
 * @name Elves function
 * 
 * @brief elves process
 * 
 * @param NE number of reeindeers
 * @param TE max. time in milliseconds for which the reindeer works independently
 */
void processSanta(args a)
{   
    sem_wait(mutex);
        fprintf(file, "%d: Santa: going to sleep\n", *actionCounter);
        fflush(file);
        (*actionCounter)++;
    sem_post(mutex);

    for(;;) {

        sem_wait(santaSem);

        if(*elvesCounter== 3  && *elvesHolidays== 0) {

            sem_wait(mutex);
                fprintf(file, "%d: Santa: helping elves\n", *actionCounter);
                fflush(file);
                (*actionCounter)++;
            sem_post(mutex);

            for(int i = 0; i < 3; i++) 
            {
                sem_post(elfTex2);
            }

            for(int i = 0; i < 3; i++) 
            {
                sem_wait(santaHelped);
            }   
            
            
            sem_wait(mutex);
            
            fprintf(file, "%d: Santa: going to sleep\n", *actionCounter);
            (*actionCounter)++;
            fflush(file);
            
            
           
            sem_post(mutex);
           

    
        } else if (*reindeerCounter == a.NR) {

            sem_wait(mutex);
                fprintf(file, "%d: Santa: closing workshop\n", *actionCounter);
                fflush(file);
                (*actionCounter)++;

                for(int i = 0; i < a.NE; i++) 
                {
                    sem_post(elfTex2);
                }
                
              
               // fprintf(file, "TEST\n");
                fflush(file);
                *elvesHolidays = 1;
                
                for (int i = 0; i < a.NR; i++) 
                {
                    sem_post(reindeerSem);
                }
            sem_post(mutex);

                
    
            for(int i = 0; i < a.NR; i++) 
            {
                sem_wait(getHitched);
            }
            
           
    
            sem_wait(mutex);
                


                fprintf(file, "%d: Santa: Christmas started\n", *actionCounter);  
                fflush(file);
                (*actionCounter)++;
            sem_post(mutex);
            sem_post(allProcesses);
           exit(1);
        } else {

        }
    }

}

/**
 * @brief does initialization of shared memories and semaphores
 */
void init() 
{
    /* Initializations of shared memories */

    actionCounter = mmap(NULL, sizeof *actionCounter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (actionCounter == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *actionCounter = 1;  

    reindeerCounter = mmap(NULL, sizeof *reindeerCounter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (reindeerCounter == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *reindeerCounter = 0;


    elvesCounter = mmap(NULL, sizeof *elvesCounter, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (elvesCounter == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *elvesCounter = 0;

    elvesHolidays = mmap(NULL, sizeof *elvesHolidays, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (elvesHolidays == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *elvesHolidays = 0;

    /* Initializations of semaphores */

    santaSem = sem_open(SEM_SANTASEM, O_CREAT | O_EXCL, 0666, 0);
    if (santaSem == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (santaSem) \n");
        exit(EXIT_FAILURE);
    }
   
    mutex = sem_open(SEM_MUTEX, O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (mutex) \n");
        exit(EXIT_FAILURE);
    }

    reindeerSem = sem_open(SEM_REINDEERSEM, O_CREAT | O_EXCL, 0666, 0);
    if (reindeerSem == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (reindeerSem)  \n");
        exit(EXIT_FAILURE);
    }

    elfTex = sem_open(SEM_ELFTEX, O_CREAT | O_EXCL, 0666, 1);
    if ( elfTex == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.(elfTex)  \n");
        exit(EXIT_FAILURE);
    }

    elfTex2 = sem_open(SEM_ELFTEX2, O_CREAT | O_EXCL, 0666, 0);
    if ( elfTex2 == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (elfTex2) \n");
        exit(EXIT_FAILURE);
    }

    allProcesses = sem_open(SEM_ALL, O_CREAT | O_EXCL, 0666, 0);
    if ( allProcesses == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (allProcesses) \n");
        exit(EXIT_FAILURE);
    }

    getHitched = sem_open(SEM_GETHITCHED, O_CREAT | O_EXCL, 0666, 0);
    if ( getHitched == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (getHitched) \n");
        exit(EXIT_FAILURE);
    }

    santaHelped = sem_open(SEM_SANTAHELPED, O_CREAT | O_EXCL, 0666, 0);
    if ( santaHelped  == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (getHitched) \n");
        exit(EXIT_FAILURE);
    }


    file = fopen("proj2.out", "w");
    if (file == NULL)
    {
        fprintf(stderr, "ERROR > Opening file!\n");
        exit(1);
    }

}

/**
 * @brief destroys semaphores and shared memories
 */
void clean() 
{   
    sem_unlink(SEM_SANTAHELPED);
    sem_close(santaHelped);

    sem_unlink(SEM_GETHITCHED);
    sem_close(getHitched);

    sem_unlink(SEM_ALL);
    sem_close(allProcesses);

    sem_unlink(SEM_ELFTEX2);
    sem_close(elfTex2);

    sem_unlink(SEM_ELFTEX);
    sem_close(elfTex);

    sem_unlink(SEM_MUTEX);
    sem_close(mutex);

    sem_unlink(SEM_REINDEERSEM);
    sem_close(reindeerSem);

    sem_unlink(SEM_SANTASEM);
    sem_close(santaSem);

    munmap(elvesHolidays, sizeof *elvesHolidays);
    munmap(actionCounter, sizeof *actionCounter);
    munmap(reindeerCounter, sizeof *reindeerCounter);
    munmap(elvesCounter, sizeof *elvesCounter);
}

/**
 * @brief loads arguments, check them and gives errors
 * 
 * @param argc number of arguments
 * @param argv argument vectors
 * @param a    structure
 */
args arguments(int argc, char **argv, args a)
{

    char *ptr;
    /* small number of entered arguments */
    if (argc != NUM_OF_ARGS) 
    {
        fprintf(stderr, "ERROR > Enter more arguments.\n");
        exit(ARGS_FAIL);
    } 
   
    /* checking entered arguments */
    a.NE = strtol(argv[1], &ptr, 10); // number of elves
    if (*ptr != '\0' || ptr == argv[1]) {
        printf("ERROR > '%s' is not a number.\n", argv[1]);
        exit(ARGS_FAIL);
    }
    
    a.NR = strtol(argv[2], &ptr, 10); // number of reeinders
    if (*ptr != '\0' || ptr == argv[2]) {
        printf("ERROR > '%s' is not a number.\n", argv[2]);
        exit(ARGS_FAIL);
    }
    
     a.TE = strtol(argv[3], &ptr, 10); // max. time in milliseconds for which the elf works independently
    if (*ptr != '\0' || ptr == argv[3]) {
        printf("ERROR > '%s' is not a number.\n", argv[3]);
        exit(ARGS_FAIL);
    }
   
    a.TR = strtol(argv[4], &ptr, 10); // max. time in milliseconds for which the reindeer works ind ependently
    if (*ptr != '\0' || ptr == argv[4]) {
        printf("ERROR > '%s' is not a number.\n", argv[4]);
        exit(ARGS_FAIL);
    }

     /****** Checking if the number is in the range *****/

    if (a.NE < MIN_ELVES || a.NE > MAX_ELVES) 
    {
        fprintf(stderr, "ERROR > Number has to be 1 to 999.\n");
        exit(ARGS_FAIL);
    }  
    if (a.NR < MIN_REINDEERS || a.NR > MAX_REINDEERS) 
    {
        fprintf(stderr, "ERROR > Number has to be 1 to 19.\n");
        exit(ARGS_FAIL);
    } 
    if (a.TE < MIN_TIME || a.TE > MAX_TIME) 
    {
        fprintf(stderr, "ERROR > Number has to be 1 to 999.\n");
        exit(ARGS_FAIL);
    } 
    if (a.TR < MIN_TIME || a.TR > MAX_TIME) 
    {
        fprintf(stderr, "ERROR > Number has to be 1 to 999.\n");
        exit(ARGS_FAIL);
    }

    return a;
}

/**
 * Main function
 * 
 * @param argc number of arguments
 * @param argv argument values
 * @return 0 return sucess
 */
int main(int argc, char **argv) 
{      
    /********** Inicialization **********/

    args a = arguments(argc, argv, a);
    init();
   
    /********** Process generating **********/

    pid_t process = fork();

    if (process == 0) {

    processSanta(a);
     
    } else {
        for(int i = 0; i < a.NE; i++)
        {   
            int j = a.NE;
            pid_t genElves[j];
            genElves[j] = fork();
            if (genElves[j] == 0)
            {

            processElves(a.TE, i+1);

            } else if (genElves[j] < 0) {

                fprintf(stderr, "ERROR > Fork failed (genElves).\n");
            }
        }
        
        for(int i = 0; i < a.NR; i++)
        {
            int j = a.NR;
            pid_t genReindeers[j]; 
            genReindeers[j] = fork();
            if (genReindeers[j] == 0) 
            {

            processReindeer(a.NR, a.TR, i+1); 

            } else if (genReindeers[j] < 0) {

                fprintf(stderr, "ERROR > Fork failed (genReindeers).\n");
            } 
        } 
    }
    
    /* Main process waits untill all other processes are finished */
    for(int i = 0; i < (a.NE + a.NR + 1); i++) 
    {
        sem_wait(allProcesses);
    }
    
    /*************** Cleaning ***************/
    clean();
    fclose(file);

    return 0;
}
