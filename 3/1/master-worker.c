#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

int item_to_produce; //현재 만들어야하는 아이템 담는 변수
int curr_buf_size; //현재 버퍼에 있는 크기
int total_items; //생산하고 싶은 총 아이템 수 (0~M-1)
int max_buf_size; //버퍼 최대 크기
int num_workers; //워커 스레드의 수
int num_masters; //마스터 스레드의 수

int *buffer; //버퍼(배열)

void print_produced(int num, int master) { //생산되었다는걸 출력하는 함수
   printf("Produced %d by master %d\n", num, master);
}

void print_consumed(int num, int worker) { //소비되었다는걸 출력하는 함수
   printf("Consumed %d by worker %d\n", num, worker);
  
}


//produce items and place in buffer
//modify code below to synchronize correctly
void *generate_requests_loop(void *data) //마스터 쓰레드가 돌리는 함수
{
   int thread_id = *((int *)data); //인자로 받은 데이터 정수형으로 타입 변환
	
   while(1) //무한 반복
   {

      if(item_to_produce >= total_items) { //현재 만들어야하는 변수가 범위 벗어날 경우
			break; //무한 반복 탈출
      }
 	
   	buffer[curr_buf_size++] = item_to_produce; //버퍼에 아이템 넣어주고 curr_buf_size 증가
      print_produced(item_to_produce, thread_id); //생산한 아이템 인자로 넣어준 쓰레드 id와 함께 출력
      item_to_produce++; //생산해야하는 아이템 재지정
   }
   return 0;
}

//write function to be run by worker threads
//ensure that the workers call the function print_consumed when they consume an item

int main(int argc, char *argv[])
{
   int *master_thread_id; //마스터 쓰레드 아이디 담을 배열
   pthread_t *master_thread; //마스터 쓰레드 담을 배열
   item_to_produce = 0;
   curr_buf_size = 0; //현재 버퍼 차있는 정도
  
   int i;
  
   //입력인자 받는 과정
   if (argc < 5) { 
     printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
     exit(1);
   }
   else {
     num_masters = atoi(argv[4]);
     num_workers = atoi(argv[3]);
     total_items = atoi(argv[1]);
     max_buf_size = atoi(argv[2]);
   }
	//여기까지
  

  buffer = (int *)malloc (sizeof(int) * max_buf_size); //버퍼 메모리에 할당(입력한 버퍼 최대 크기 사이즈만큼)

  //create master producer threads
  master_thread_id = (int *)malloc(sizeof(int) * num_masters);//마스터 쓰레드 아이디 담을 배열 메모리에 할당
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters); //마스터 쓰레드 배열 메모리에 할당
  for (i = 0; i < num_masters; i++) //마스터 쓰레드 아이디 담을 배열 초기화
    master_thread_id[i] = i;

  for (i = 0; i < num_masters; i++) //마스터 쓰레드 생성, 마쓰터 쓰레드가 돌릴 함수에 쓰레드 아이디 인자로 넘겨줌
    pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);
  
  //create worker consumer threads

  
  //wait for all threads to complete
  for (i = 0; i < num_masters; i++) //전체 마스터 쓰레드에 대하여
    {
      pthread_join(master_thread[i], NULL); //마스터 쓰레드 기다린다.
      printf("master %d joined\n", i); //종료된 마스터 쓰레드 출력
    }
  
  /*----Deallocating Buffers---------------------*/
  free(buffer); //버퍼 할당 해제
  free(master_thread_id); //마스터 쓰레드 아이디 담긴 배열 할당 해제
  free(master_thread); //마스터 쓰레드 배열 할당 해제
  
  return 0;
}
