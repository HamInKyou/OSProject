#include  <stdio.h>
#include  <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64 //토큰 하나 최대 사이즈
#define MAX_NUM_TOKENS 64 //최대 토큰 개수
#define MAX_CMD_SIZE 64 //명령 하나 최대 사이즈
#define MAX_NUM_CMDS 64 //최대 명령 개수


//공백 기준으로 토큰을 만들어서 토큰배열을 리턴해주는 함수
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *)); //토큰배열 공간할당
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char)); //토큰하나 공간할당
	int i, tokenIndex = 0, tokenNo = 0;

	//토큰화 할 라인 한글자씩 검사한다.
	for(i =0; i < strlen(line); i++){
		char readChar = line[i]; //현재차례 글자 readChar에 저장

		if (readChar == ' ' || readChar == '\n' || readChar == '\t') //공백, 개행, 탭이 나왔을 때
		{ 
			token[tokenIndex] = '\0'; //현재토큰 문자열 자리에 널값 넣어줌
			if (tokenIndex != 0) //현재 토큰 인덱스가 0번째가 아니라면
			{ 
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char)); //토큰의 새자리 동적할당
				strcpy(tokens[tokenNo++], token);//동적할당한 그 자리에 현재 토큰 넣어준다.
				tokenIndex = 0; //토큰 내 인덱스를 맨 앞으로 바꾼다.
			}
		} 
		else 
		{
			token[tokenIndex++] = readChar; //토큰에 현재 글자 넣어주고 인덱스 다음 가리키게
		}
	}

	free(token); //다 끝나면 현재 토큰 하나 동적할당 했던거 해제
	tokens[tokenNo] = NULL ; //토큰배열 끝내기
	return tokens; //토큰 배열 리턴
}


int main(int argc, char* argv[]) {
	char  line[MAX_INPUT_SIZE];  //입력 한줄 받아 저장할 문자열
	char 	execTo[MAX_TOKEN_SIZE]; //exec해서 넘어갈 실행파일 있는 위치 저장할 문자열
	char  **tokens; //토큰 배열
	char  **cmd;
	char  **cmdTokens;
	int i, status, cmdCnt, cmdTokenCnt;
	int fd[2]; //읽기용 파이프, 쓰기용 파이프를 지정하기 위한 배열 선언
	int fdd;
	pid_t pid;
	FILE* fp;

	if(argc == 2) { //실행파일 뒤에 파일하나 더 붙었을 경우
		fp = fopen(argv[1],"r"); //읽기모드로 그 파일 연다.
		if(fp < 0) { //파일이 존재하지 않을 경우
			printf("File doesn't exists.");
			return -1; //종료
		}
	}

	while(1) {			
		//명령 한줄 입력하기
		bzero(line, sizeof(line)); //line을 다 0으로 채운다.
		if(argc == 2) { // 명령어입력되어 있는 파일을 받았을 경우
			if(fgets(line, sizeof(line), fp) == NULL) { //파일 한줄 읽어 line에 저장
				break;	
			}
			line[strlen(line) - 1] = '\0'; //line 끝에 개행대신 널값 추가. 문자열 끝내기
		} else { // 파일 받지 않았을 경우(ssu_shell.c 내에서 유저한테 입력받아 한줄씩 실행)
			printf("$ "); 
			scanf("%[^\n]", line); //엔터칠 때까지 받는다.
			getchar(); //엔터 버퍼에서 뽑기 위해
		}
		//명령 한줄 입력 완료

		line[strlen(line)] = '\n'; //명령 끝에 널대신에 개행
		tokens = tokenize(line); //명령 토큰으로 찢어주기


		if(tokens[0] == NULL) //엔터만 쳤을 경우 넘어가게
		continue;

		cmdCnt = 0;
		cmdTokenCnt = 0;
		cmd = (char **)malloc(MAX_NUM_CMDS * sizeof(char *)); //명령어 배열 공간할당
		cmd[cmdCnt] = (char*)malloc(MAX_CMD_SIZE * sizeof(char)); //명령어의 새자리 동적할당
		bzero(cmd[0], sizeof(MAX_CMD_SIZE)); //cmd를 다 0으로 채운다.
		for(i=0;tokens[i]!=NULL;i++){
			if(tokens[i][0] == '|'){
				cmdCnt++;
				cmdTokenCnt = 0;
				cmd[cmdCnt] = (char*)malloc(MAX_CMD_SIZE * sizeof(char)); //명령어의 새자리 동적할당
				continue;
			}
			if(cmdTokenCnt == 0)
				sprintf(cmd[cmdCnt], "%s", tokens[i]);
			else
				sprintf(cmd[cmdCnt], "%s %s", cmd[cmdCnt], tokens[i]);
			cmdTokenCnt++;
		}


		fdd = 0;
		while(*cmd != NULL)
		{
			*(*cmd+strlen(*cmd)) = '\n'; //명령 끝에 널대신에 개행
			cmdTokens = tokenize(*cmd); //명령 토큰으로 찢어주기


			if (pipe(fd) < 0){
				fprintf(stderr, "pipe error\n");
				exit(1);
			}
			if((pid = fork()) < 0) {
				fprintf(stderr, "fork error\n");
				exit(1);
			}
			else if(pid == 0) { //자식 프로세스
				dup2(fdd, 0);
				if (*(cmd+1) != NULL)
					dup2(fd[1],1);
				close(fd[0]);


				sprintf(execTo, "/bin/%s", cmdTokens[0]);
				if (execv(execTo, cmdTokens) < 0) {
					fprintf(stderr, "execv error\n");
					exit(1);
				}

			}
			else { //부모 프로세스
				wait(&status);
				close(fd[1]);
				fdd=fd[0];
				cmd++;
			}
		}


		//토큰 배열에 토큰들 할당 해제
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]); //토큰 하나 할당 해제
		}
		free(tokens); //토큰 배열 할당 해제

		//토큰 배열에 토큰들 할당 해제
		for(i=0;cmdTokens[i]!=NULL;i++){
			free(cmdTokens[i]); //토큰 하나 할당 해제
		}
		free(cmdTokens); //토큰 배열 할당 해제

	}
	//명령어 배열에 명령어들 할당 해제
	for(i=0;cmd[i]!=NULL;i++){
		free(cmd[i]); //명령 하나 할당 해제
	}
	free(cmd); //명령 배열 할당 해제

	return 0;
}

