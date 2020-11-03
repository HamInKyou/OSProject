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

SSU_Sem child;
SSU_Sem parent;

void *justprint(void *data)
{
  int thread_id = *((int *)data);
  SSU_Sem_down(&child); //child 공유자원 사용하기 위해
  printf("This is thread %d\n", thread_id);
  SSU_Sem_up(&parent); //parent 공유자원 만들어주기
  return 0;
}

int main(int argc, char *argv[])
{

  pthread_t t1, t2;
  int t1id = 1, t2id = 2; 

  SSU_Sem_init(&child, 0); //child 세마포어 초기화
  SSU_Sem_init(&parent, 0); //parent 세마포어 초기화
  
  pthread_create(&t1, NULL, justprint, &t1id); //tid 1 쓰레드 생성
  pthread_create(&t2, NULL, justprint, &t2id); //tid 2 쓰레드 생성
  
  sleep(1); //1밀리초 기다렸다가


  //in spite of sleep, this should print first
  printf("This is main thread. This should print first\n");

  SSU_Sem_up(&child); //child 공유자원 만들어주기
  SSU_Sem_down(&parent); //parent 공유자원 사용
  printf("One thread has printed\n");
  
  SSU_Sem_up(&child); //child 공유자원 만들어주기
  SSU_Sem_down(&parent); //parent 공유자원 사용
  printf("Second thread has printed\n");
  
  pthread_join(t1, NULL); //tid1 쓰레드 끝나는거 기다림
  pthread_join(t2, NULL); //tid2 쓰레드 끝나는거 기다림
  
  return 0;
}
