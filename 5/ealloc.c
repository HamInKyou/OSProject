#include "ealloc.h"
#define ALLOCMAPSIZE PAGESIZE/MINALLOC

struct memory {
	char *addr; //시작주소
	int isAlloc; //할당되어 있는지
	int size; //몇칸 할당되어 있는지
};

char *pagePtrArr[4];
struct memory allocMap[4][ALLOCMAPSIZE];

void init_alloc()
{
	int i;
	for(i = 0; i < 4; i++)
		pagePtrArr[i] = NULL;
}

void cleanup()
{
	int i;
	for(i = 0; i < 4; i++)
		pagePtrArr[i] = NULL;
}

char *alloc(int size)
{
	int i, j, k;
	int needPage = 0;
	int canAlloc = 0;
	char *startAddr;

	if(size%MINALLOC != 0 || size > PAGESIZE)
		return NULL;
	//제일 처음 할당하는경우
	if(pagePtrArr[0] == NULL)
		needPage = 1;
	
	//페이지들 다 탐색
	for(i = 0; i < 4; i++){
		//페이지가 필요한데 해당 페이지 할당 안되어있을 경우
		if(needPage && pagePtrArr[i] == NULL){
			//페이지 할당해주기
			pagePtrArr[i] = (char *)mmap(0, PAGESIZE, PROT_READ|PROT_WRITE, MAP_ANONYMOUS|MAP_PRIVATE, -1, 0);
			if(pagePtrArr[i] == MAP_FAILED)
				return NULL;
		}
		//페이지 하나 맵 탐색
		for(j = 0; j < ALLOCMAPSIZE; j++){
			//그 주소에서 할당된게 없을 경우
			if(allocMap[i][j].isAlloc == 0){
				canAlloc = 1;
				//j번째 주소에 할당하려는 사이즈 더했는데 4KB 넘어섰을 경우
				if(j*MINALLOC + size > PAGESIZE){
					canAlloc = 0;
					needPage = 1;
					break;
				}
				//j번째를 시작으로 하여 충분히 할당할 공간 있는지 탐색
				for(k = 0; k < size/MINALLOC; k++){
					if(allocMap[i][j+k].isAlloc){
						canAlloc = 0;
						needPage = 1;
						break;
					}
				}
				if(canAlloc) //할당 가능하면 탐색 그만!
					break;
			}
		}
		if(canAlloc) //할당 가능하면 탐색 그만!
			break;
		else{
			canAlloc = 0;
			needPage = 1;
		}
	}
	if(canAlloc){ //할당해주기
		startAddr = pagePtrArr[i] + (j * MINALLOC);
		allocMap[i][j].addr = startAddr;
		allocMap[i][j].size = size/MINALLOC;
		for(k = 0; k < size/MINALLOC; k++){
			allocMap[i][j+k].isAlloc = 1;
		}
		return startAddr;
	}
	else
		return NULL;
}

void dealloc(char *str)
{
	int i, j, k;
	for(i = 0; i < 4; i++){
		for(j = 0; j < ALLOCMAPSIZE; j++){ //Map 탐색
			if(allocMap[i][j].addr == str){ //시작주소 발견했을 경우
				for(k = 0; k < allocMap[i][j].size; k++) //그 시작주소부터 할당되지 않았다고 표시
					allocMap[i][j+k].isAlloc = 0;
			}
		}
	}
}
