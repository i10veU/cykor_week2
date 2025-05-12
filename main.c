#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#define maxline 1024

void interface(void)
{
    char cwd[1024];                 // cwd 쉘에 표시
    char *usrname = getenv("USER"); // get USERname environment
    char hostname[1024];
    getcwd(cwd, sizeof(cwd));
    gethostname(hostname, sizeof(hostname)); // HOSTNAME

    if (usrname != NULL)
    {
        printf("%s@%s:%s$", usrname, hostname, cwd);
    }
}

int check_bkg(char *input){
    int background=0;
    if(input[strlen(input)-1]=='&'){
        background= -1;
        input[strlen(input)-1]='\0';//&제거
        input[strcspn(input, " \t\r\n")]=0; //공백제거(끝 다듬기), strcspn 공백, \t \r \n까지 길이 구해서 해당 문자 위치 찾기
    }
    return background;
}

int cd(char *input)
{
    if (strncpy(input, "cd", 3) == 0)
    {                           // 입력 cd 확인, strcpy와 달리 3으로 제한해 overflow 방지
        char *path = input + 3; // 경로 추출
        if (chdir(path) != 0) // dir 변경 시도 경로 맞으면 0, 틀리면 -1 : 절대/상대경로 모두 가능
            perror("cd"); // error message
        else return 0;
    }
}

int pwd(char *input)
{
    if (strcmp(input, "pwd") == 0){ // strncpy보다 효율적, strcpy+if로 에러 컨트롤 하는 것보다 수월
        char cwd[1024];  
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
        return 0;
    }
}

int exec(char *input, int bkg)
{
    // 외부 명령어.
    pid_t pid = fork(); // ProcessID of fork(child) 호출
    if (pid == 0){ // fork return 0: child process inner memory
        // 자식process
        char *args[1024];
        char *token = strtok(input, " "); // input을 " "(공백) 기준으로 parsing
        int i = 0;
        while (token != NULL)
        {
            args[i] = token;
            i += 1;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        execvp(args[0], args); // execute progrmm with path (e.g., ls, cat, echo)
        exit(1);               // child exit
        return 0;
    }else if (pid > 0){ // fork return parent process or child's PID
        // 부모process
        waitpid(pid, NULL, 0); // child exit까지 대기
        if(!bkg) waitpid(pid, NULL, 0);
        else prinf("백그라운드 실행 : pid=%d\n", pid);
        return 0;
    }else perror("fork"); // fork return -1: fail
}

void pipeline(char *input){
    char *cmds[10]; // 최대 10개 파이프라인
    int cmd_count = 0;

    // 명령어 pipeline 기준 파싱
    char *token = strtok(input, "|");
    while(token != NULL && cmd_count < 10){
        cmds[cmd_count] = token;
        cmd_count += 1;
        token = strtok(NULL, "|");
    }

    int prev_fd = -1;//pipefd[0] save

    for(int i=0; i<cmd_count; i++){
        int pipefd[2];//pipefd[0]: read, pipefd[1]: write
        if(i<cmd_count) pipe(pipefd);//다음 명령과 연결될 새 파이프 생성

        pid_t pid = fork();
        if(pid==0){
            if(prev_fd != -1){
                dup2(prev_fd, 0);//이전 파이프 읽기 > stdin
                close(prev_fd);
            }
            if(i<cmd_count){
                close(pipefd[0]);//다음 파이프 읽기 close
                dup2(pipefd[1], 1);//쓰기 > stdout
                close(pipefd[1]);
            }

            int bkg=check_bkg(cmds[i]);
            run(cmds[i], bkg);

            perror("execvp");
            exit(1);
        }else{
            if(prev_fd!=-1){
                close(prev_fd);//이전 파이프 읽기 close
            }
            if(i<cmd_count){
                close(pipefd[1]);//write close
                prev_fd = pipefd[0];//다음 fd 저장
            }
        }
    }

    for(int i=0; i<cmd_count; i++) wait(NULL);
}

void multi_cmds(char *input, int bkg){
    int last_status = -1;
    while(*input){
        char *next_and = strstr(input, "&&");
        char *next_or = strstr(input, "||");
        char *next_seq = strstr(input, ";");

        char *next = NULL;
        int type = 0; // 0:;, 1:&&, 2:||

        if(next_and && (!next || next_and < next)) { next = next_and; type = 1;}
        if(next_or && (!next || next_or < next)) { next = next_or; type = 2;}
        if(next_seq && (!next || next_seq < next)) { next = next_seq; type = 0;}

        if(next) *next = '\0';//공백 제거
        while(input==' ')input++;
        if(*input){
            int bkg = check_bkg(input);
            if(strchr(input, '|')) pipeline(input);
            else last_status = run(input, bkg);
        }

        if(!next) break;//더 없음

        input = next + (type == 0? 1:2);
        if(type==1 && last_status==-1) break;
        if(type==2 && last_status==0) break;
    }
}

int run(char *input, int bkg){
    if(strncmp(input, "cd ", 3) == 0){
        return cd(input);
    }else if(strcmp(input, "pwd") == 0){
        return pwd(input);
    }else {
        return exec(input, bkg);
    }
};

int main(void)
{
    char input[maxline];

    while (1)
    {
        interface();

        if (fgets(input, maxline, stdin) == NULL) break;// 한줄 입력
        input[strcspn(input, "\n")] = 0; // strcount stop not : 개행 제거

        int bkg=check_bkg(input);

        if (strcmp(input, "exit") == 0) // exit나오면 탈출
        {
            break;
        }else if(strchr(input, ';') || strchr(input, '&&') || strchr(input, '||')){
            multi_cmds(input, bkg);
        }else if(strchr(input, '|') != NULL){//find pipeline(char)
            pipeline(input);
        }else run(input, bkg);
    }
    return 0;
}