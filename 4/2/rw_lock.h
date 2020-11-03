#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct rw_lock
{
	int num_Reader_Threads; //읽기 쓰레드 수
	int num_Writer_Threads; //쓰기 쓰레드 수
	int num_Waiting_Writer_Threads; //대기중인 쓰기 쓰레드 수
	pthread_mutex_t mutex; //pthread mutex변수
	pthread_cond_t read_cond; //pthread 조건 변수
	pthread_cond_t write_cond; //pthread 조건 변수

};

void init_rwlock(struct rw_lock * rw);
void r_lock(struct rw_lock * rw);
void r_unlock(struct rw_lock * rw);
void w_lock(struct rw_lock * rw);
void w_unlock(struct rw_lock * rw);
long *max_element(long* start, long* end);
long *min_element(long* start, long* end);
