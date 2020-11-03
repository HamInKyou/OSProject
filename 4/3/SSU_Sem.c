#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include "SSU_Sem.h"

void SSU_Sem_init(SSU_Sem *s, int value) {
   pthread_mutex_init(&(s->mutex), NULL); //mutex 초기화
   pthread_cond_init(&(s->cond), NULL); //cond 초기화
   s->counter = value; //사용할 수 있는 공유 데이터의 개수
}

void SSU_Sem_down(SSU_Sem *s) {  //공유자원사용하기 위해
	pthread_mutex_lock(&(s->mutex)); //락 걸기
	while(s->counter <= 0)  //카운트 0이하일 경우
		pthread_cond_wait(&(s->cond),&(s->mutex)); //쓰레드 블록
	s->counter -= 1; //카운터 값을 1 감소시킴
	pthread_mutex_unlock(&(s->mutex)); //락 풀기
}

void SSU_Sem_up(SSU_Sem *s) { //공유자원 다 썼음(반환)
	pthread_mutex_lock(&(s->mutex)); //락 걸기
	s->counter += 1; //카운터 값을 1 증가시킴
	pthread_cond_signal(&(s->cond)); //대기 중인 쓰레드 깨우기
	pthread_mutex_unlock(&(s->mutex)); //락 풀기
}
