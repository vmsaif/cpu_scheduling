//-----------------------------------------
// NAME: Saif Mahmud
// Date: 12-02-2023
// 
// PURPOSE: Implementing scheduling policies, and write programs that use threads, locks, and condition variables.
//
//-----------------------------------------
#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/queue.h>
#include <string.h>
#include <errno.h>

#define NANOS_PER_USEC 1000
#define USEC_PER_SEC   1000000
#define MAX_LENGTH 256
#define TOP_PRIORITY 3
#define BOTTOM_PRIORITY 0
#define TYPE_COUNT 4

// the times below are microseconds
#define QUANTUM_LENGTH 50
#define PRIORITY_ALLOTMENT_TIME 200

static int threadCount = 0; 
static int boostTime = 0;
static char* fileName;
static int fileEnded = 0;

FILE *fd;

TAILQ_HEAD (job_list, _JOB) all_jobs;
struct job_list *job_list_head;

TAILQ_HEAD (job_obj_list, _JOB) all_obj;
struct job_obj_list *job_obj_list_head;


static pthread_t threadArray[MAX_LENGTH];
static pthread_mutex_t mutexQueuelock;
static pthread_mutex_t mutexCommonlock;
static pthread_mutex_t newLock;
static pthread_cond_t condQueueNotEmpty;
static int shouldWorkerThreadExit = 0;
static double avgTT[TYPE_COUNT];
static double avgTT_TypeCount[TYPE_COUNT];
static double avgRT[TYPE_COUNT];
static double avgRT_TypeCount[TYPE_COUNT];

struct timespec diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
    
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

static void microsleep(unsigned int usecs)
{
    long seconds = usecs / USEC_PER_SEC;
    long nanos   = (usecs % USEC_PER_SEC) * NANOS_PER_USEC;
    struct timespec t = { .tv_sec = seconds, .tv_nsec = nanos };
    int ret;
    do
    {
        ret = nanosleep( &t, &t );
        // need to loop, `nanosleep` might return before sleeping
        // for the complete time (see `man nanosleep` for details)
    } while (ret == -1 && (t.tv_sec || t.tv_nsec));
}

void split(char* inputArr, char** outputArr, char* delim) {
    
    char *temp;
    temp = strtok(inputArr, delim);

    for(int i = 0; temp != NULL; i++) {
        outputArr[i] = temp;
        temp = strtok(NULL, " ");
    }
}

typedef struct _JOB
{
    pthread_mutex_t objLock;
    TAILQ_ENTRY(_JOB) pointers;
    TAILQ_ENTRY(_JOB) objPointers;
    char task_name[MAX_LENGTH];
    int task_type;
    int task_length;
    int odds_of_IO;
    int priority;
    long arrivalTime;
    long completionTime;
    long firstRunTime;
    int isRunning;
} JOB;

//constructor
/** @brief consturcts a new job
 *
 *  @param task_name The name of the task.
 *  @param task_type The type of the task.
 *  @param task_length The length of the task.
 *  @param odds_of_IO The I/O odds of the task.
 *  @return A Job object.
 */
JOB* createJOB( char* task_name, int task_type, int task_length, int odds_of_IO)
{
    JOB* output = (JOB*)malloc(sizeof(JOB));
    struct timespec arrvTime;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &arrvTime);
    output->arrivalTime = arrvTime.tv_nsec;
    output->firstRunTime = 0;
    strcpy(output->task_name, task_name);
    output->task_type = task_type;
    output->task_length = task_length;
    output->odds_of_IO = odds_of_IO;
    output->isRunning = 0;

   // MLFQ RULE 3, Job enters at the highest priority. So Setting top 
   // priority at the creation.
    output->priority = TOP_PRIORITY;


    pthread_mutex_init(&output->objLock, NULL);
    return output;
}



void *readingTasks( void *args )
{
    (void) args;

    //---------read the txt file------------------------//

    char buffer[MAX_LENGTH];
    if((fd = fopen(fileName, "r")) == NULL)
    {
        fprintf(stderr, "can't open %s: %s\n", fileName, strerror(errno));
        exit(EXIT_FAILURE);
    }

    while(fgets(buffer, MAX_LENGTH, fd))
    {
        char* splitArr[MAX_LENGTH];
        split(buffer, splitArr, " ");
        
        if(strcmp(splitArr[0], "DELAY") != 0)
        {
            JOB* newJob = createJOB(splitArr[0], atoi(splitArr[1]), atoi(splitArr[2]), atoi(splitArr[3]));
            pthread_mutex_lock(&newJob->objLock);
            TAILQ_INSERT_TAIL(&all_jobs, newJob, pointers);
            TAILQ_INSERT_TAIL(&all_obj, newJob, objPointers);
            pthread_mutex_unlock(&newJob->objLock);
        } else {
            //DELAY
            sleep((atoi(splitArr[1])/NANOS_PER_USEC)); //in milliseconds
        }
    }

    //if the file ends...
    fileEnded = 1;

    fclose(fd);    
    return NULL;
}

void runAjob(JOB* myJob, int countOfSamePriority)
{
    int randomnumber;
    int taskRunningTime = myJob->task_length;

    if(myJob->firstRunTime == 0)
    {
        struct timespec fRun;
        long fRunTime;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fRun);
        myJob->firstRunTime = fRun.tv_nsec;
        fRunTime = (myJob->firstRunTime) / (long) NANOS_PER_USEC;

        avgRT[myJob->task_type] = avgTT[myJob->task_type] + fRunTime;
        avgRT_TypeCount[myJob->task_type]++;
        myJob->isRunning = 1;
    }

    if(myJob->task_length < QUANTUM_LENGTH)
    {
        taskRunningTime = myJob->task_length;
    } else if(countOfSamePriority > 1) {
        //do roundRobin
        taskRunningTime = QUANTUM_LENGTH;
    } else {
        //reduce priority
        // MLFQ RULE 4 : reducing priority if it is going to use entire timeslice.
        taskRunningTime = PRIORITY_ALLOTMENT_TIME;
        if(myJob->priority > BOTTOM_PRIORITY){
            myJob->priority--;
        }
    }

    // odds of io implement
    randomnumber = (rand() % (100 - 0 + 1)) + 0;
    if(randomnumber <= myJob->odds_of_IO)
    {
        int randomnumber2 = (rand() % (taskRunningTime - 0 + 1)) + 0;
        taskRunningTime = randomnumber2;
    }

    myJob->task_length = myJob->task_length-taskRunningTime;

    //execute task.
    microsleep(taskRunningTime);

    
    if(myJob->task_length < 1)
    {
        struct timespec finTime;
        long timeTaken;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &finTime);
        myJob->completionTime = finTime.tv_nsec;
        myJob->isRunning = 0;
        timeTaken = (myJob->completionTime - myJob->arrivalTime) / (long)NANOS_PER_USEC;
        avgTT[myJob->task_type] = avgTT[myJob->task_type] + timeTaken;
        avgTT_TypeCount[myJob->task_type]++; 

        
    } else {
        // reschedule
        pthread_mutex_lock(&myJob->objLock);
        TAILQ_INSERT_HEAD(&all_jobs, myJob, pointers);
        pthread_mutex_unlock(&myJob->objLock);

        if(TAILQ_EMPTY(&all_jobs))
        {
            pthread_cond_signal(&condQueueNotEmpty);
        }

        
    }
   

}


void* jobDispatcher(void* args)
{
    (void) args;
    
    // RULE 3, Job enters at the highest priority.
    int currPriority = TOP_PRIORITY; 
    int needPriorityBoosting = 0;
    // int timeLock = 0;
    long total_boosted = 1;
    
    while (shouldWorkerThreadExit == 0)
    {
        pthread_mutex_lock(&newLock);
        
        long boostPTime = boostTime*NANOS_PER_USEC;
        long tTime = 0;

        for(int i = 0; i < TYPE_COUNT; i++)
        {
            tTime = tTime + avgTT[i];
        }
        
        if(needPriorityBoosting == 0 && ((tTime/total_boosted) >= boostPTime))
        {
            needPriorityBoosting = 1;
            total_boosted++;
        }
        

        // all priority booster
        // MLFQ RULE 5: After time s, setting all job priority to highest
        if(needPriorityBoosting && !TAILQ_EMPTY(&all_jobs))
        {
            JOB* curr;
            TAILQ_FOREACH_REVERSE(curr, &all_jobs, job_list, pointers)
            {
                curr->priority = TOP_PRIORITY;
            }
            needPriorityBoosting = 0;
            // timeLock = 0;
        }
        pthread_mutex_unlock(&newLock);
        
        JOB* j; 
        JOB* execJob = NULL; 
        int found = 0;
        int increasePriority = 0;
        int countOfSamePriority = 0;

        // MLFQ RULE 1: running the bigger priority first.
        // Here, the last variable j will be the nearest from the head of the queue.
        // if the thread searching for currPriority matches with the nearest from 
        // the queue head, then the bigger prioriry and the first come job gets to 
        // be executed by the dispacher to a thread
        TAILQ_FOREACH_REVERSE(j, &all_jobs, job_list, pointers)
        {
            //checking if the priority needs to be changed to assign the jobs to the threads
            if(currPriority < j->priority)
            {
                //we have bigger priority tasks
                increasePriority = 1;
            }
            
            if(j->priority == currPriority)
            {
                found = 1;
                countOfSamePriority++;
                execJob = j;
            }
        }
        
        if(execJob != NULL)
        {
            // MLFQ RULE 2: JOB A is being assigning now, JOB B will be assigned in the next loop of this TAILQ_FOREACH.
            // Then round robin fasion will be implemented on runAjob() as time will be sliced and they will be 
            // rescheduled and will come at this point again.
            pthread_mutex_lock(&mutexCommonlock);
            TAILQ_REMOVE(&all_jobs, execJob, pointers);
            runAjob(execJob, countOfSamePriority);
            
            struct timespec fRunTime;
            clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &fRunTime);
            execJob->firstRunTime = fRunTime.tv_nsec;                
            
            pthread_mutex_unlock(&mutexCommonlock);
        }
        
        // set curr priority
        //if none found of current priority, either increase or decrease thread job picking priority
        
        if(found == 0)
        {
            if(increasePriority == 1 && currPriority < TOP_PRIORITY)
            {
                currPriority++;
            } else if(currPriority > BOTTOM_PRIORITY) {
                currPriority--;
            }
        }
        pthread_mutex_lock(&mutexCommonlock);

        if(fileEnded && TAILQ_EMPTY(&all_jobs))
        {
            shouldWorkerThreadExit = 1;
        }
        pthread_mutex_unlock(&mutexCommonlock);
        
    }//while
    return NULL;
    
}

void* produceReport()
{
    printf("Using mlfq with %d CPUs.\n", threadCount-1);
    printf("Using boost time of %d usec.\n", boostTime);
    printf("Average turnaround time per type:\n");
    for(int i = 0; i < TYPE_COUNT; i++)
    {
        double average = 0.0;
        if(avgTT_TypeCount[i] >= 1.0)
        {
            average = avgTT[i] / avgTT_TypeCount[i];
        }
        printf ("Type %d: %lf usec\n", i, average);
    }

    printf("Average response time per type:\n");
    for(int i = 0; i < TYPE_COUNT; i++)
    {
        double average = 0.0;
        if(avgRT_TypeCount[i] >= 1.0)
        {
            average = avgRT[i] / avgRT_TypeCount[i];
        }
        printf ("Type %d: %lf usec\n", i, average);
    }

    return NULL;
}

void threadJoin(pthread_t threadArray[])
{
    if(threadCount > 0){
        for (int i = 0; i < threadCount; i++)
        {
            if(pthread_join( threadArray[i], NULL ) != 0)
            {
                fprintf(stderr, "Failed to join thread %d\n", i);
            }
        }
    }
    
}

void freeAllObjMemory(struct job_obj_list *all_obj) {
    if (all_obj == NULL) {
        printf("all_obj is NULL\n");
    } else {
        JOB *curr;
        while (!TAILQ_EMPTY(all_obj)) {
            curr = TAILQ_FIRST(all_obj);
            TAILQ_REMOVE(all_obj, curr, objPointers);

            // Destroying the mutex lock inside the JOB object, if it exists
            pthread_mutex_destroy(&curr->objLock);

            free(curr);
        }
    }
}

int main( int argc, char* argv[] )
{
    printf("Starting Program\n");
    if (argc < 4)
    {
        printf("Not enough argument.\nCheck readme.md for CLI instructions.");
        exit(EXIT_FAILURE);
    }


    threadCount = atoi(argv[1]);
    boostTime = atoi(argv[2]);
    fileName = argv[3];
    
    //init the list
    pthread_cond_init(&condQueueNotEmpty, NULL);
    pthread_mutex_init(&newLock, NULL);
    pthread_mutex_init(&mutexQueuelock, NULL);
    pthread_mutex_init(&mutexCommonlock, NULL);
    TAILQ_INIT (&all_jobs);
    TAILQ_INIT (&all_obj);
    
    //-------------------------------------------------//

    // create threads

    // reading thread
    if(pthread_create( &threadArray[0], NULL, readingTasks, NULL ) == 0)
    {
        threadCount++;
    } else {
        fprintf(stderr, "can't create Reading CPU %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }


    // random seed
    srand(time(NULL));

    // worker threads
    for (int i = 1; i < threadCount; i++)
    {
        if(pthread_create( &threadArray[i], NULL, jobDispatcher, NULL ) == 0)
        {
            printf("CPU %d Started\n", i);
        } else {
            fprintf(stderr, "can't create CPU %d: %s\n", i, strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    
    
    threadJoin(threadArray);

    pthread_cond_destroy(&condQueueNotEmpty);
    pthread_mutex_destroy(&mutexQueuelock);
    pthread_mutex_destroy(&mutexCommonlock);
    pthread_mutex_destroy(&newLock);

    
    

    produceReport();
    freeAllObjMemory(&all_obj);
    printf("End of Program\n\n");

    return EXIT_SUCCESS;
}
