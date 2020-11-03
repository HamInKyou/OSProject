#include "rw_lock.h"

void init_rwlock(struct rw_lock * rw)
{
  //Write the code for initializing your read-write lock.
  rw->num_Reader_Threads = 0; //일하고 있는 읽기 쓰레드 수
  rw->num_Writer_Threads = 0; //일하고 있는 쓰기 쓰레드 수
  rw->num_Waiting_Writer_Threads = 0; //일하기 위해 대기중인 쓰기 쓰레드 수
  pthread_mutex_init(&(rw->mutex), NULL); //mutex 초기화
  pthread_cond_init(&(rw->read_cond), NULL); //읽기 쓰레드를 위한 조건변수 초기화
  pthread_cond_init(&(rw->write_cond), NULL); //쓰기 쓰레드를 위한 조건변수 초기화
}

void r_lock(struct rw_lock * rw)
{
   pthread_mutex_lock(&(rw->mutex)); //락 걸기
	//쓰기 쓰레드가 하나라도 일하고 있거나, 쓰기 쓰레드 대기 중인게 있을 경우
   while(rw->num_Writer_Threads>0 || rw->num_Waiting_Writer_Threads>0)
	   pthread_cond_wait(&(rw->read_cond), &(rw->mutex)); //읽기 시그널 기다림
   rw->num_Reader_Threads += 1; //현재 일하고 있는 읽기 쓰레드 개수 늘림
   pthread_mutex_unlock(&(rw->mutex)); //락 풀기
}

void r_unlock(struct rw_lock * rw)
{
   pthread_mutex_lock(&(rw->mutex)); //락 걸기
   rw->num_Reader_Threads -= 1; //현재 일하고 있는 읽기 쓰레드 개수 하나 줄임
	//현재 일하고 있는 읽기 쓰레드가 없고, 일하기 위해 대기중인 쓰기 쓰레드가 있을 경우
   if(rw->num_Reader_Threads == 0 && rw->num_Waiting_Writer_Threads > 0)
	   pthread_cond_signal(&(rw->write_cond)); //쓰기 시그널 보내기
	pthread_mutex_unlock(&(rw->mutex)); //락 풀기
}

void w_lock(struct rw_lock * rw)
{
   pthread_mutex_lock(&(rw->mutex)); //락 걸기
	rw->num_Waiting_Writer_Threads += 1; //쓰기 대기 중인 쓰레드 개수 하나 늘림
	//일하고 있는 읽기 쓰레드가 존재하거나, 일하고 있는 쓰기 쓰레드가 있는 동안
   while(rw->num_Reader_Threads > 0 || rw->num_Writer_Threads > 0) 
	   pthread_cond_wait(&(rw->write_cond), &(rw->mutex)); //쓰기 시그널 기다림 
   rw->num_Waiting_Writer_Threads -= 1; //쓰기 대기중인 쓰레드 개수 하나 줄이기
	rw->num_Writer_Threads += 1; //쓰기 쓰레드 개수 하나 늘리기
   pthread_mutex_unlock(&(rw->mutex)); //락 풀기
}

void w_unlock(struct rw_lock * rw)
{
   pthread_mutex_lock(&(rw->mutex)); //락 걸기
	rw->num_Writer_Threads -= 1; //쓰기 쓰레드 개수 하나 줄임
   if(rw->num_Waiting_Writer_Threads > 0) //대기 중인 쓰기 쓰레드 있을 경우
   {
	   pthread_cond_signal(&(rw->write_cond)); //쓰기 신호 보내기
   }
	pthread_cond_broadcast(&(rw->read_cond)); //읽기 시그널 기다리는 모든 쓰레드에 시그널 보내기
   pthread_mutex_unlock(&(rw->mutex)); //락 풀기
}
