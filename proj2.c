/**
 * @file    proj2.c
 * @brief   IOS - Operační systémy - 2. projekt - Santa Claus problem
 * @author  Tadeáš Kachyňa <xkachy00@stud.fit.vutbr.cz>
 * @date    4.5.2021
 * @version 1.0
 */

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

/***** Constants *****/

#define NUM_OF_ARGS 5
#define ARGS_FAIL 1
#define MAX_REINDEERS 19
#define MIN_REINDEERS 1
#define MAX_ELVES 999
#define MIN_ELVES 1
#define MIN_TIME 0
#define MAX_TIME 1000

/***** Semaphores *****/

#define SEM_SANTASEM    "/xkachy00-ios2-santaSem"
#define SEM_MUTEX       "/xkachy00-ios2-mutex"
#define SEM_REINDEERSEM "/xkachy00-ios2-reindeerSem"
#define SEM_ELFTEX      "/xkachy00-ios2-elfTex"
#define SEM_MUTEX2      "/xkachy00-ios2-mutex2"
#define SEM_ELFTEX2     "/xkachy00-ios2-elfTex2"

sem_t *santaSem = NULL;
sem_t *mutex = NULL;
sem_t *mutex2 = NULL;
sem_t *reindeerSem = NULL;
sem_t *elfTex = NULL;
sem_t *elfTex2 = NULL;


/***** Shared memories *****/
static int *glob_var;   
static int *RD_id;
static int *E_id;
static int *Elves_counter;
static int *RD_counter;
static int *elfHoliday;

FILE *file;

/** @struct args users arguments
 * 
 * @var NE number of elves
 * @var NR number of reeinders
 * @var TE max. time in milliseconds for which the elf works independently
 * @var TR max. time in milliseconds for which the reindeer works ind ependently
 */
typedef struct {
    int NE;
    int NR;
    int TE;
    int TR;
} args;

void destroy();
/**
 * @name Reindeer Fucntion
 * 
 * @brief reeinder process
 * 
 * @param NR number of reeindeers
 * @param TR max. time in milliseconds for which the reindeer works independently
 */
void processReindeer(int NR, int TR, int i) 
{   
    
    srand(time(NULL));
 
    int lower = TR / 2; // lower border
    int upper = TR; // upper border
    
    int ran;


        ran = (rand() % (upper - lower + 1) + lower); // gives you random value from interval <TR/2 , TR >
        ran = ran * 1000; 

       
            sem_wait(mutex);
                ++(*glob_var);
                fprintf(file, "%d: RD %d: rstarted\n", *glob_var, i);
                fflush(file);
            sem_post(mutex);

            usleep(ran);
            
            sem_wait(mutex);
                ++(*glob_var);
                ++(*RD_counter);
               // printf(" %d  \n", ran);
                fprintf(file, "%d: RD %d: return home\n", *glob_var, i);
                fflush(file);
            sem_post(mutex);

            
            if(*RD_counter == NR) 
            {     
                sem_post(santaSem);
               
            }
  
            sem_wait(reindeerSem);
                *RD_counter = *RD_counter - 1;
                *glob_var = *glob_var + 1;
                fprintf(file, "%d: RD %d: get hitched\n", *glob_var, i); 
                fflush(file);
                exit(1);
                
            
        
       
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
     
            
            /* Elves start working */
            sem_wait(mutex);
                ++(*glob_var);
                fprintf(file, "%d: Elf %d: started\n", *glob_var, i);
                fflush(file);
            sem_post(mutex);
            
            for(;;) {
            
            srand(time(NULL));
            ran = (rand() % TE);
            ran = ran * 1000; 

            usleep(ran);

            /* Elves need helps */
            sem_wait(elfTex);
            sem_wait(mutex);
                ++(*glob_var);
                ++(*Elves_counter);
                fprintf(file, "%d: Elf %d: need help\n", *glob_var, i);   
                fflush(file);
            
            /* Checking how many elves need help */
            if (*Elves_counter == 3) 
            {
                sem_post(santaSem);

            } else {

                sem_post(elfTex); 
            } 

            if (*elfHoliday == 1) {
                fprintf(file, "%d: Elf %d: Taking holidays\n", *glob_var, i);   
                fflush(file);
                exit(1);
                
            }

            sem_post(mutex);  

            sem_wait(elfTex2);

            sem_wait(mutex);
            ++(*glob_var);
            fprintf(file, "%d: Elf %d: Get help\n", *glob_var, i);
            fflush(file);
            
            
            (*Elves_counter)--;

            if (*Elves_counter == 0) {
                sem_post(elfTex);
            }
            sem_post(mutex);
            }
         
        

    return 0;
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
     ++(*glob_var);
    fprintf(file, "%d: Santa: going to sleep\n", *glob_var);
    fflush(file);
    sem_post(mutex);

    for(;;) {
    sem_wait(santaSem);
    if(*Elves_counter == 3) {
        //printf("test2\n");
        
        *glob_var = *glob_var + 1;
        fprintf(file, "%d: Santa: helping elves\n", *glob_var);
        fflush(file);
        sem_post(elfTex2);
        sem_post(elfTex2);
        sem_post(elfTex2);
       
        
        
        usleep(1000);
        *glob_var = *glob_var + 1;
        fprintf(file, "%d: Santa: going to sleep\n", *glob_var);
        fflush(file);
    } else if (*RD_counter == a.NR) {
        fprintf(file, "%d: Santa: closing workshop\n", *glob_var); *glob_var = *glob_var + 1;
        fflush(file);
        *elfHoliday = 1;
        for (int i = 0; i < a.NR; i++) {
        sem_post(reindeerSem);
        }
        usleep(10000);
        sem_wait(mutex);
        fprintf(file, "%d: Santa: Christmas started", *glob_var);  
        fflush(file);
        exit(1);    
    } else {
        
    }}

  
}

/**
 * @brief does initialization of shared memories and semaphores
 */
void init() 
{
    /* Initializations of shared memories */

    glob_var = mmap(NULL, sizeof(*(glob_var)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (glob_var == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *glob_var = 0;  

    RD_id= mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (RD_id == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *RD_id = 0;

    E_id= mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (E_id == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *E_id = 0;    

    Elves_counter = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (Elves_counter == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *Elves_counter = 0;

    RD_counter = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if ( RD_counter == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *RD_counter = 0; 

    elfHoliday = mmap(NULL, sizeof *elfHoliday, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    if (elfHoliday == MAP_FAILED) 
    {
        fprintf(stderr, "ERROR > Allocation of shared memory failed.");
    }

    *elfHoliday = 0;

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

     mutex2 = sem_open(SEM_MUTEX2, O_CREAT | O_EXCL, 0666, 0);
    if (mutex2 == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore. (mutex2) \n");
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
void destroy() 
{   
    sem_unlink(SEM_ELFTEX2);
    sem_close(elfTex2);

    sem_unlink(SEM_ELFTEX);
    sem_close(elfTex);

    sem_unlink(SEM_MUTEX);
    sem_close(mutex);

    sem_unlink(SEM_MUTEX2);
    sem_close(mutex2);

    sem_unlink(SEM_REINDEERSEM);
    sem_close(reindeerSem);

    sem_unlink(SEM_SANTASEM);
    sem_close(santaSem);

    munmap(glob_var, sizeof *glob_var);
    munmap(RD_id, sizeof *RD_id);
    munmap(elfHoliday, sizeof *elfHoliday);
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
 * @return EXIT_SUCCESS return sucess
 */
int main(int argc, char **argv) 
{      
    /***** Inicialization *****/

    args a = arguments(argc, argv, a);
    init();
   
    /***** Process generating *****/

    pid_t process = fork();

    if (process == 0) {

    processSanta(a);
     
    } else {
        for(int i = 0; i < a.NE; i++){
            pid_t process2 = fork();
            if (process2 == 0) {
    
            processElves(a.TE, i+1);
            } 
        }
        
        for(int i = 0; i < a.NR; i++){
            pid_t process3 = fork();
            if (process3 == 0) {
            
            processReindeer(a.NR, a.TR, i+1); 
            }  
        } 
    }
    
    /***** Cleaning *****/
    destroy();

    return EXIT_SUCCESS;
}
