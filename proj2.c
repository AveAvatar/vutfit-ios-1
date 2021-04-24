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

sem_t *santaSem = NULL;
sem_t *mutex = NULL;
sem_t *mutex2 = NULL;
sem_t *reindeerSem = NULL;
sem_t *elfTex = NULL;
sem_t *elfTex2 = NULL;

// Shared memory variable
static int *glob_var;   
static int *RD_id;
static int *E_id;
static int *Elves_counter;
static int *RD_counter;

typedef struct {
    int NE;
    int NR;
    int TE;
    int TR;
} args;


/**
 * @name Reindeer Fucntion
 * 
 * @brief reeinder process
 * 
 * @param NR number of reeindeers
 * @param TR max. time in milliseconds for which the reindeer works independently
 */
void processReindeer(int NR, int TR) {
    srand(time(NULL));

    int lower = TR / 2; // lower border
    int upper = TR; // upper border
    
    int ran;

    for(int i=1; i<NR+1; i++) 
    {   
        ran = (rand() % (upper - lower + 1) + lower); // gives you random value from interval <TR/2 , TR >
        ran = ran * 1000; 

        pid_t reindeer = fork();

        if(reindeer == 0)
        {   
            sem_wait(mutex);
                ++(*glob_var);
                printf("%d: RD %d: rstarted\n", *glob_var, i);
            sem_post(mutex);

            usleep(ran);
            
            sem_wait(mutex);
                ++(*glob_var);
                ++(*RD_counter);
               // printf(" %d  \n", ran);
                printf("%d: RD %d: return home\n", *glob_var, i);
            sem_post(mutex);
        
            if(*RD_counter == NR) 
            {     
                sem_post(santaSem);
               
            }
  
            sem_wait(reindeerSem);
                *RD_counter = *RD_counter - 1;
                *glob_var = *glob_var + 1;
                printf("%d: RD %d: get hitched\n", *glob_var, i); 
            
            exit(0);
        } 
    }
     
    while(wait(NULL)!=-1);
}
    
/**
 * @name Elves function
 * 
 * @brief elves process
 * 
 * @param NE number of reeindeers
 * @param TE max. time in milliseconds for which the reindeer works independently
 */
int processElves(int NE, int TE){
    srand(time(NULL));
    int ran;
    for(int i=1; i<NE+1; i++) 
    {   
        ran = (rand() % TE);
        ran = ran * 1000; 
        pid_t elves = fork();

        if(elves == 0)
        {   
            
            /* Elves start working */
            sem_wait(mutex);
                ++(*glob_var);
                printf("%d: Elf %d: started\n", *glob_var, i);
            sem_post(mutex);
            
            for(;;) {
            usleep(ran);

            /* Elves need helps */
            sem_wait(elfTex);
            sem_wait(mutex);
                ++(*glob_var);
                ++(*Elves_counter);
                //printf(" %d  \n", ran);
                printf("%d: Elf %d: need help\n", *glob_var, i);   
            
           // printf("### %d ### \n", *Elves_counter);
            
            /* Checking how many elves need help */
            if (*Elves_counter == 3) {
                //printf(" %d cekam a klepam\n", *Elves_counter);
                 //printf("test\n");
                sem_post(santaSem);
                 //printf("test 3\n");

                
            } else {
                //printf(" %d cekam\n", *Elves_counter );
                sem_post(elfTex);
               
            } 


            
            sem_post(mutex);  

            sem_wait(elfTex2);
            ++(*glob_var);
            printf("%d: Elf %d: Get help\n", *glob_var, i);
            
            sem_wait(mutex);
            (*Elves_counter)--;
            //printf("### %d ### \n", *Elves_counter);
            if (*Elves_counter == 0) {
                sem_post(elfTex);
            }
            sem_post(mutex);
            }
         
            exit(1);
        } 
    }
    
    while(wait(NULL)!=-1);
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
   
    for(;;) {
    sem_wait(santaSem);
    if(*Elves_counter == 3) {
        //printf("test2\n");
        
        *glob_var = *glob_var + 1;
        printf("%d: Santa: helping elves\n", *glob_var);
        sem_post(elfTex2);
        sem_post(elfTex2);
        sem_post(elfTex2);
       
        
        
        usleep(1000);
        *glob_var = *glob_var + 1;
        printf("%d: Santa: going to sleep\n", *glob_var);
    } else if (*RD_counter == a.NR) {
        printf("%d: Santa: closing workshop\n", *glob_var); *glob_var = *glob_var + 1;
        for (int i = 0; i < a.NR; i++) {
        sem_post(reindeerSem);
        }
        usleep(10000);
        printf("%d: Santa: Christmas started\n", *glob_var);  
        fflush(stdout);
        exit(1);       
    } else {
        
    }}

  
}


void init() 
{
    /* Initializations of shared memories */

    glob_var = mmap(NULL, sizeof(*(glob_var)), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *glob_var = 0;  

    RD_id= mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *RD_id = 0;

    E_id= mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *E_id = 0;    

    Elves_counter = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *Elves_counter = 0;

    RD_counter = mmap(NULL, sizeof *glob_var, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    *RD_counter = 0; 

    /* Initializations of semaphores */

    santaSem = sem_open("sem", O_CREAT | O_EXCL, 0666, 0);
    if (santaSem == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }
   
    mutex = sem_open("mutex", O_CREAT | O_EXCL, 0666, 1);
    if (mutex == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }

     mutex2 = sem_open("mutex2", O_CREAT | O_EXCL, 0666, 0);
    if (mutex2 == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }

    reindeerSem = sem_open("reindeerSem", O_CREAT | O_EXCL, 0666, 0);
    if (reindeerSem == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }

    elfTex = sem_open("elftex", O_CREAT | O_EXCL, 0666, 1);
    if ( elfTex == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }

    elfTex2 = sem_open("elftex2", O_CREAT | O_EXCL, 0666, 0);
    if ( elfTex2 == SEM_FAILED) 
    {
        fprintf(stderr, "Error > Semaphore.\n");
        exit(EXIT_FAILURE);
    }
}

void destroy() 
{   
    sem_unlink("elftex2");
    sem_close(elfTex2);
    sem_unlink("elftex");
    sem_close(elfTex);
    sem_unlink("mutex");
    sem_close(mutex);
    sem_unlink("mutex2");
    sem_close(mutex2);
    sem_unlink("reindeerSem");
    sem_close(reindeerSem);
      sem_unlink("sem");
    sem_close(santaSem);
    munmap(glob_var, sizeof *glob_var);
    munmap(RD_id, sizeof *RD_id);
}


args arguments(int argc, char **argv, args a)
{

    char *ptr;
    /* small number of entered arguments */
    if (argc < 5) 
    {
        fprintf(stderr, "ERROR > Enter more arguments.\n");
        exit(1);
    } 
   
    /* checking entered arguments */
    a.NE = strtol(argv[1], &ptr, 10); // number of elves
    if (*ptr != '\0' || ptr == argv[1]) {
        printf("ERROR > '%s' is not a number.\n", argv[1]);
        exit(1);
    }
    
    a.NR = strtol(argv[2], &ptr, 10); // number of reeinders
    if (*ptr != '\0' || ptr == argv[2]) {
        printf("ERROR > '%s' is not a number.\n", argv[2]);
        exit(1);
    }
    
     a.TE = strtol(argv[3], &ptr, 10); // max. time in milliseconds for which the elf works independently
    if (*ptr != '\0' || ptr == argv[3]) {
        printf("ERROR > '%s' is not a number.\n", argv[3]);
        exit(1);
    }
   
    a.TR = strtol(argv[4], &ptr, 10); // max. time in milliseconds for which the reindeer works ind ependently
    if (*ptr != '\0' || ptr == argv[4]) {
        printf("ERROR > '%s' is not a number.\n", argv[4]);
        exit(1);
    }

     /* checking if the number is in the range */
    if (a.NE < 1 || a.NE > 999) 
    {
        fprintf(stderr, "ERROR > Number have to be 1 to 999.\n");
        exit(1);
    }  
    if (a.NR < 1 || a.NR > 19) 
    {
        fprintf(stderr, "ERROR > Number have to be 1 to 19.\n");
        exit(1);
    } 
    if (a.TE < 0 || a.TE > 1000) 
    {
        fprintf(stderr, "ERROR > Number have to be 1 to 999.\n");
        exit(1);
    } 
    if (a.TR < 0 || a.TR > 1000) 
    {
        fprintf(stderr, "ERROR > Number have to be 1 to 999.\n");
        exit(1);
    }

    return a;
}

int main(int argc, char **argv) 
{      


    args a = arguments(argc, argv, a);
    init();
   

    ++(*glob_var);
    printf("%d: Santa: going to sleep\n", *glob_var);

    
    pid_t process = fork();
    if (process == 0) {
        processReindeer(a.NR, a.TR);
    } else {
        pid_t process2 = fork();
        if (process2 == 0) {
            processElves(a.NE, a.TE);
        } else {
            processSanta(a);
      
        }

        while(wait(NULL)!=-1);
    }
        while(wait(NULL)!=-1);
    
    

   
    
    destroy();

   
    return 0;

}
