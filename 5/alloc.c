#include "alloc.h"
#define ALLOCMAPSIZE PAGESIZE/MINALLOC

struct memory {
	char *addr; //시작주소
	int isAlloc; //할당되어 있는지
	int size; //몇칸 할당되어 있는지
};

char *memoryPtr;
struct memory allocMap[ALLOCMAPSIZE];

int init_alloc()
{
	memoryPtr = (char *)mmap(0, PAGESIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0); 
	if(memoryPtr == MAP_FAILED)
		return -1;
	return 0;
}

int cleanup()
{
	return munmap(memoryPtr, PAGESIZE);
}

char *alloc(int size)
{
	int i, j;
	int canAlloc;
	char *startAddr;
	
	if(size%8 != 0) //할당하려는 사이즈 8바이트로 안나뉘어떨어지면 실패!
		return NULL;
	for(i = 0; i < ALLOCMAPSIZE; i++){ //Map에 ALLOC할 수 있는 곳 탐색
		if(allocMap[i].isAlloc == 0){ //ALLOC이 되어있지 않은 곳이라면
			canAlloc = 1;
			//i번째의 주소에서 할당하려는 사이즈 더했는데 4KB 넘어섰을 경우 실패!
			if(i*MINALLOC + size > PAGESIZE)
				return NULL;
			//i번째를 시작으로 하여 충분히 할당할 공간 있는지 탐색
			for(j = 0; j < size/MINALLOC; j++){ 
				if(allocMap[i+j].isAlloc){ //할당할 공간 충분하지 않은 경우
					canAlloc = 0;
					break;
				}
			}
			if(canAlloc) //할당 가능하면 탐색 그만!
				break;
		}
	}
	if(canAlloc){ //할당 가능할 경우
		startAddr = memoryPtr + (i * MINALLOC); //시작 주소 지정
		allocMap[i].addr = startAddr; //시작 주소 저장
		allocMap[i].size = size/MINALLOC; //할당될 칸 수 저장
		for(j = 0; j < size/MINALLOC; j++){ //Map에 사이즈만큼 할당 되었다고 표시
			allocMap[i+j].isAlloc = 1;
		}
		return startAddr;
	}
	else //ALLOC할 수 있는 곳 다 탐색해봤는데 넣을 데 없을 경우
		return NULL;
}

void dealloc(char *str)
{
	int i, j;
	for(i = 0; i < ALLOCMAPSIZE; i++){ //Map 탐색
		if(allocMap[i].addr == str){ //시작주소 발견했을 경우
			for(j = 0; j < allocMap[i].size; j++) //그 시작주소부터 할당되지 않았다고 표시
				allocMap[i+j].isAlloc = 0;
		}
	}
}
