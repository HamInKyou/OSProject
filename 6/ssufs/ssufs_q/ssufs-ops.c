#include "ssufs-ops.h"

extern struct filehandle_t file_handle_array[MAX_OPEN_FILES];

int ssufs_allocFileHandle() {
	for(int i = 0; i < MAX_OPEN_FILES; i++) {
		if (file_handle_array[i].inode_number == -1) {
			return i;
		}
	}
	return -1;
}

int ssufs_create(char *filename){
	int inode_number; //inodenum 담을 임시변수
	
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t)); //inode담을 임시 구조체 초기화
	if(open_namei(filename) != -1){ //동일한 이름의 파일이 파일 시스템에 존재할 경우
		free(inode); //임시구조체 할당 해제
		return -1;
	}
	inode_number = ssufs_allocInode(); //inode_freelist에서 비어있는 첫 노드의 인덱스 찾기
	if(inode_number == -1){ //inode_freelist가 없다면(추가로 파일을 생성할 공간이 없을 경우)
		free(inode); //임시구조체 할당 해제
		return -1;
	}
	
	inode->status = INODE_IN_USE; //inode 상태 사용으로 바꾸기
	strcpy(inode->name, filename); //inode에 filename 넣어주기
	inode->file_size = 0; //이제 파일 생성한거라 파일 사이즈 0

	ssufs_writeInode(inode_number,inode); //inode block의 inodenum에 생성한 파일 정보 추가
	free(inode); //ssufs에 썼으니까 inode 할당 해제
	return inode_number; //새로 생성 된 파일의 inode 번호 리턴
}

void ssufs_delete(char *filename){
	int inode_number;
	int i;
	inode_number = open_namei(filename); //해당 파일 아이노드 번호 찾기
	if(inode_number != -1){ //해당 파일 찾았을 경우
		struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct inode_t)); //임시 구조체 만들기
		ssufs_readInode(inode_number,inode); //해당 아이노드 번호의 아이노드 구조체 읽기
		ssufs_freeInode(inode_number); //아이노드 프리해주기
		free(inode); //임시 구조체 할당 해제
	}
}

int ssufs_open(char *filename){
	int inode_number;
	int file_handle;
	if((inode_number = open_namei(filename)) == -1) //파일의 inodenum 찾기
		return -1;
	if((file_handle = ssufs_allocFileHandle()) == -1) //파일 핸들러의 빈자리 찾기
		return -1;
	file_handle_array[file_handle].inode_number = inode_number; //파일 핸들러의 빈자리에 inodenum 넣기
	return file_handle; //그 파일 핸들 번호 리턴
}

void ssufs_close(int file_handle){
	file_handle_array[file_handle].inode_number = -1;
	file_handle_array[file_handle].offset = 0;
}

int ssufs_read(int file_handle, char *buf, int nbytes){
	int inode_number;
	int i;
	char *tmpBuf = malloc(BLOCKSIZE*MAX_FILE_SIZE);
	char *ptr;
	inode_number = file_handle_array[file_handle].inode_number; //해당 파일 inode 찾아오기
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct superblock_t)); //임시 구조체 만들기
	ssufs_readInode(inode_number,inode); //해당 아이노드 번호의 아이노드 구조체 읽기
	if(file_handle_array[file_handle].offset + nbytes > inode->file_size){ //요청된 바이트 수 읽었는데 파일 크기 초과할 경우 에러!
		free(inode);
		return -1;
	}
	memset(tmpBuf, 0, BLOCKSIZE*MAX_FILE_SIZE);
	ptr = tmpBuf; //tmpBuf 시작점 가리키게
	for(i = 0; i < MAX_FILE_SIZE; i++){ //해당 파일의 데이터들 읽기
		if(inode->direct_blocks[i] == -1){
			continue;
		}
		ssufs_readDataBlock(inode->direct_blocks[i], ptr); //블록 읽어서 tmpBuf에 붙이기
		ptr += BLOCKSIZE;	//포인터 뒤로 밀기
	}
	ptr = tmpBuf + file_handle_array[file_handle].offset; //오프셋으로 이동
	strncpy(buf,ptr,nbytes); //ptr부터 buf에 nbytes만큼 읽어서 저장
	ssufs_lseek(file_handle, nbytes);
	
	free(inode);
	free(tmpBuf);
	return 0;
}

int ssufs_write(int file_handle, char *buf, int nbytes){
	int inode_number;
	int i;
	int tmp_direct_blocks[MAX_FILE_SIZE];
	int cur_directBlock_amount = 0;
	int after_directBlock_amount = 0;
	char *tmpBlock = malloc(BLOCKSIZE);
	char *tmpBuf = malloc(BLOCKSIZE*MAX_FILE_SIZE);
	char *ptr;
	inode_number = file_handle_array[file_handle].inode_number; //해당 파일 inode 찾아오기
	struct inode_t *inode = (struct inode_t *)malloc(sizeof(struct superblock_t)); //임시 구조체 만들기
	ssufs_readInode(inode_number,inode); //해당 아이노드 번호의 아이노드 구조체 읽기
	if(file_handle_array[file_handle].offset + nbytes > BLOCKSIZE*MAX_FILE_SIZE){ //요청된 바이트 수 쓰면 최대 파일 크기 제한 초과할 경우 에러!
		free(inode);
		return -1;
	}
	memset(tmpBuf, 0, BLOCKSIZE*MAX_FILE_SIZE);
	ptr = tmpBuf;
	for(i = 0; i < MAX_FILE_SIZE; i++){ //해당 파일의 데이터들 읽기
		if(inode->direct_blocks[i] == -1){
			tmp_direct_blocks[i] = -1;
			continue;
		}
		tmp_direct_blocks[i] = inode->direct_blocks[i];
		cur_directBlock_amount++; //블록개수 세기
		ssufs_readDataBlock(inode->direct_blocks[i], ptr); //블록 읽어서 tmpBuf에 붙이기
		ptr += BLOCKSIZE;	//포인터 뒤로 밀기
	}
	ptr = tmpBuf;
	ptr += file_handle_array[file_handle].offset;
	strncpy(ptr, buf, nbytes); //ptr부터 buf 붙임
	if(strlen(tmpBuf)%BLOCKSIZE) //딱맞아 떨어지지 않으면
		after_directBlock_amount = strlen(tmpBuf)/BLOCKSIZE+1;
	else //딱맞아 떨어지면
		after_directBlock_amount = strlen(tmpBuf)/BLOCKSIZE;
	
	for(i = cur_directBlock_amount; i < after_directBlock_amount; i++){ //추가적으로 필요한 블록들 임시 저장
		tmp_direct_blocks[i] = ssufs_allocDataBlock();
		if(tmp_direct_blocks[i] == -1) //만약에 추가적으로 블록 할당할 칸이 남아있지 않다면
			return -1;
	}
	
	memset(tmpBlock, 0, BLOCKSIZE); //블록하나 데이터 담을 블록 초기화
	for(i = 0; i < after_directBlock_amount; i++){
		ptr = tmpBuf + (i * BLOCKSIZE); //큰 tmpBuf 블록 단위로 쪼개기 위해
		strncpy(tmpBlock, ptr, BLOCKSIZE); //tmpBlock의 블록단위로 쪼갠 tmpBuf 넣는다.
		ssufs_writeDataBlock(tmp_direct_blocks[i], tmpBlock); //쪼갠걸 써준다.
		inode->direct_blocks[i] = tmp_direct_blocks[i]; //아이노드의 direct_blocks 최신화
	}
	inode->file_size = strlen(tmpBuf); //아이노드의 size 최신화
	ssufs_writeInode(inode_number, inode);
	ssufs_lseek(file_handle, nbytes);
	
	free(inode);
	free(tmpBuf);
	free(tmpBlock);
	return 0;
	
}

int ssufs_lseek(int file_handle, int nseek){
	int offset = file_handle_array[file_handle].offset;

	struct inode_t *tmp = (struct inode_t *) malloc(sizeof(struct inode_t));
	ssufs_readInode(file_handle_array[file_handle].inode_number, tmp);
	
	int fsize = tmp->file_size;
	
	offset += nseek;

	if ((fsize == -1) || (offset < 0) || (offset > fsize)) {
		free(tmp);
		return -1;
	}

	file_handle_array[file_handle].offset = offset;
	free(tmp);

	return 0;
}
