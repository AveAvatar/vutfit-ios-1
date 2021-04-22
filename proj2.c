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

// Reindeer function
void reindeer(int NR, int TR) {
    srand(time(NULL));

    int lower = TR / 2;
    int upper = TR;

    int ran;
    for(int i=0; i<NR; i++) 
    {   
        ran = (rand() % (upper - lower + 1) + lower);
        ran = ran * 1000;
        int A = fork();

        if(A == 0)
        {   
          
            sem_wait(mutex);
            ++(*RD_id);
            ++(*glob_var);
            printf("%d: RD %d: rstarted\n", *glob_var, *RD_id);
            sem_post(mutex); 
            usleep(ran);
            
            if (*RD_id >= NR ) {
                *RD_id = 0; 
            }
            sem_wait(mutex);
            ++(*glob_var);
            ++(*RD_id);
            printf("%d: RD %d: getback\n", *glob_var, *RD_id);
            ++(*RD_counter);
            sem_post(mutex); 
            if(*RD_counter == NR) {
                sem_post(santaSem);
            }

            exit(1);
        } 

    }
     
    while(wait(NULL)!=-1);
}
    

int elves(int NE){
    srand(time(NULL));
    int ran;
    for(int i=0; i<NE; i++) 
    {   
        ran = (rand() % 1000);
        ran = ran * 1000;
        int A = fork();

        if(A == 0)
        {   
            ++(*E_id);
            ++(*glob_var);
            printf("%d: Elf %d: started\n", *glob_var, *E_id);
        
        
            usleep(ran);
            

            if (*RD_id >= NE ) {
                *RD_id = 0; 
            }
            ++(*glob_var);
            if (*E_id >= NE ) {
                *E_id = 0; 
            }
            ++(*E_id);
            printf("%d: Elf %d: need help\n", *glob_var, *E_id);
            ++(*Elves_counter);

            if (*Elves_counter == 3) {
                sem_post(santaSem);
                
                printf("%d: Got helpted.\n", *glob_var);
                
            }
            ++(*glob_var);
            
            exit(1);
        } 
    }
    
    while(wait(NULL)!=-1);
    return 0;
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
}

args arguments(int argc, char **argv, args a){

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
   
    a.TR = strtol(argv[4], &ptr, 10); // max. time in milliseconds for which the reindeer works independently
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
        reindeer(a.NR, a.TR);
    } else {
        pid_t process2 = fork();
        if (process2 == 0) {
            elves(a.NE);
        } else {
            sem_wait(santaSem);
       
            
             printf("%d: Santa: closing workshop\n", *glob_var); *glob_var = *glob_var + 1;
            for(int i = 0; i < a.NR; i++) {
                printf("%d: Santa: get hitched\n", *glob_var); 
                *glob_var = *glob_var + 1;
            }
            printf("%d: Santa: Christmas started\n", *glob_var); 
            sem_unlink("sem");
            sem_close(santaSem);
           
        
        

        }
        while(wait(NULL)!=-1);
    }
        while(wait(NULL)!=-1);
    
    

   
    
   
   
    sem_unlink("mutex");
    sem_close(mutex);
    munmap(glob_var, sizeof *glob_var);
    munmap(RD_id, sizeof *RD_id);


   
    return 0;

}