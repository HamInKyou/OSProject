#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>
#include "SSU_Sem.h"

#define NUM_THREADS 3
#define NUM_ITER 10

SSU_Sem sem[NUM_THREADS]; //세마포어 배열

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  int nextSem; //다음에 사용할 세마포어 인덱스 저장할 배열
  for(int i=0; i < NUM_ITER; i++)
    {
		SSU_Sem_down(&sem[thread_id]); //이 쓰레드에 대응하는 세마포어 사용
      printf("This is thread %d\n", thread_id);
		nextSem = (thread_id+1)%NUM_THREADS; //카운트 올릴 세마포어 지정
		SSU_Sem_up(&sem[nextSem]); //그 세마포어 카운트 올려준다.
    }
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t mythreads[NUM_THREADS]; //쓰레드들 저장할 배열
  int mythread_id[NUM_THREADS]; //쓰레드들 아이디 저장할 배열

  for(int i = 0; i < NUM_THREADS; i++) //세마포어들 초기화
    {
  		SSU_Sem_init(&sem[i], 0);
    }
  
  for(int i =0; i < NUM_THREADS; i++) //모든 쓰레드 만들어주기
    {
      mythread_id[i] = i;
      pthread_create(&mythreads[i], NULL, justprint, (void *)&mythread_id[i]);
    }
  SSU_Sem_up(&sem[0]); //첫번째 세마포어부터 사용하기 위해 카운트 올려주기

  
  for(int i =0; i < NUM_THREADS; i++) //모든 쓰레드 끝나는거 기다림
    {
      pthread_join(mythreads[i], NULL);
    }
  
  return 0;
}
