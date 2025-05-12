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

void cd(char *input)
{
    if (strncpy(input, "cd", 3) == 0)
    {                           // 입력 cd 확인, strcpy와 달리 3으로 제한해 overflow 방지
        char *path = input + 3; // 경로 추출
        if (chdir(path) != 0) // dir 변경 시도 경로 맞으면 0, 틀리면 -1 : 절대/상대경로 모두 가능
            perror("cd"); // error message
    }
}

void pwd(char *input)
{
    if (strcmp(input, "pwd") == 0){ // strncpy보다 효율적, strcpy+if로 에러 컨트롤 하는 것보다 수월
        char cwd[1024];  
        getcwd(cwd, sizeof(cwd));
        printf("%s\n", cwd);
    }
}

void exec(char *input)
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
    }else if (pid > 0){ // fork return parent process or child's PID
        // 부모process
        waitpid(pid, NULL, 0); // child exit까지 대기
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

            //명령어 파싱
            char *argv[100];
            char *arg = strtok(cmds[i], "\t\n");
            int j=0;
            while(arg != NULL){
                argv[j]=arg;
                j++;
                arg=strtok(NULL, "\t\n");
            }
            argv[j]=NULL;

            execvp(argv[0], argv);
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

int main(void)
{
    char input[maxline];

    while (1)
    {
        interface();

        if (fgets(input, maxline, stdin) == NULL) break;// 한줄 입력
        input[strcspn(input, "\n")] = 0; // strcount stop not : 개행 제거
        if (strcmp(input, "exit") == 0) // exit나오면 탈출
        {
            break;
        }else if(strchr(input, '|') != NULL){//find pipeline(char)
            pipeline(input);
        }else if(strncmp(input, "cd ", 3) == 0){
            cd(input);
        }else if(strcmp(input, "pwd") == 0){
            pwd(input);
        }else exec(input);

    }
    return 0;
}