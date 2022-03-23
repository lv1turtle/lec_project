/*
 * Copyright 2021. Heekuck Oh, all rights reserved
 */
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include "threadpool.h"

/*
 * 스레드 풀의 FIFO 대기열 길이와 일꾼 스레드의 갯수를 지정한다.
 */
#define QUEUE_SIZE 10
#define NUMBER_OF_BEES 3

/*
 * 스레드를 통해 실행할 작업 함수와 함수의 인자정보 구조체 타입
 */
typedef struct {
    void (*function)(void *p);
    void *data;
} task_t;

/*
 * 스레드 풀의 FIFO 대기열인 worktodo 배열로 원형 버퍼의 역할을 한다.
 */
static task_t worktodo[QUEUE_SIZE];
static int head = 0;
static int tail = 0;

/*
 * mutex는 대기열을 조회하거나 변경하기 위해 사용하는 상호배타 락이다.
 */
static pthread_mutex_t mutex;
static sem_t *sem;

/*
 * 대기열에 새 작업을 넣는다.
 * enqueue()는 성공하면 0, 꽉 차서 넣을 수 없으면 1을 리턴한다.
 */
static int enqueue(task_t t)
{
    // is full
    if((tail+1)%QUEUE_SIZE==head){
        return 1;
    }
    else{
        worktodo[tail].function = t.function;
        worktodo[tail].data = t.data;
        tail = (tail+1)%QUEUE_SIZE;
        sem_post(sem); // 대기열 count++
        return 0;
    }
}

/*
 * 대기열에서 실행을 기다리는 작업을 꺼낸다.
 * dequeue()는 성공하면 0, 대기열에 작업이 없으면 1을 리턴한다.
 */
static int dequeue(task_t *t)
{
    // is empty
    if(head == tail){
        return 1;
    }
    else{
        t->function = worktodo[head].function;
        t->data = worktodo[head].data;
        head = (head+1)%10;
        return 0;
    }
}

/*
 * bee는 작업을 수행하는 일꾼 스레드의 ID를 저장하는 배열이다.
 * 세마포 sem은 카운팅 세마포로 그 값은 대기열에 입력된 작업의 갯수를 나타낸다.
 */
static pthread_t bee[NUMBER_OF_BEES];

/*
 * 풀에 있는 일꾼 스레드로 FIFO 대기열에서 기다리고 있는 작업을 하나씩 꺼내서 실행한다.
 */

static void *worker(void *param)
{
    task_t t;
    while(1){
        sem_wait(sem);
        pthread_mutex_lock(&mutex);
        if(dequeue(&t))
            pthread_mutex_unlock(&mutex);
        else{
            pthread_mutex_unlock(&mutex);
            (*t.function)(t.data);
        }
    }
    pthread_exit(0);
}

/*
 * 스레드 풀에서 실행시킬 함수와 인자의 주소를 넘겨주며 작업을 요청한다.
 * pool_submit()은 작업 요청이 성공하면 0을, 그렇지 않으면 1을 리턴한다.
 */
int pool_submit(void (*f)(void *p), void *p)
{
    pthread_mutex_lock(&mutex);
    task_t t;
    t.function = f;
    t.data = p;
    if(enqueue(t)){
        pthread_mutex_unlock(&mutex);
        return 1;
    }
    else{
        pthread_mutex_unlock(&mutex);
        return 0;
    }
}

/*
 * 각종 변수, 락, 세마포, 일꾼 스레드 생성 등 스레드 풀을 초기화한다.
 */
void pool_init(void)
{
    pthread_mutex_init(&mutex, NULL);
    //create semaphore
    sem = sem_open("sem",O_CREAT,0666,0);
    for(int i=0;i<NUMBER_OF_BEES;i++){
        pthread_create(&bee[i],NULL,worker, NULL);
    }
}

/*
 * 현재 진행 중인 모든 일꾼 스레드를 종료시키고, 락과 세마포를 제거한다.
 */
void pool_shutdown(void)
{
    for(int i=0;i<NUMBER_OF_BEES;i++){
        pthread_cancel(bee[i]);
        pthread_join(bee[i],NULL);
    }
    pthread_mutex_destroy(&mutex);
    sem_destroy(sem);
}
