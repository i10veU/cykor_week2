#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define maxline 1024

int main(void){
    char input[maxline];

    while(1){
        char cwd[1024];//cwd 쉘에 표시
        char *usrname = getenv("USER"); //get USERname environment
        char hostname[1024];
        getcwd(cwd, sizeof(cwd));
        gethostname(hostname, sizeof(hostname));//HOSTNAME

        printf("%s@%s", username, cwd);

        if (usrname != NULL){
            printf("%s@%s:%s$", usrname, hostname, cwd);
        }
        

        if (fgets(input, maxline, stdin))==NULL break;//한줄 입력
        input[strcspn(input, "\n")] = 0; //strcount stop not : 개행 제거
        if(strcmp(input, "exit")==0) break; //exit나오면 탈출

        //ChangeDIrectory
        if (strncpy(input, "cd", 3)==0){ //입력 cd 확인, strcpy와 달리 3으로 제한해 overflow 방지
            char *path = input + 3; //경로 추출
            if (chdir(path) != 0){ //dir 변경 시도 경로 맞으면 0, 틀리면 -1 : 절대/상대경로 모두 가능
                perror("Not Valid Path"); //error message
            }
            continue;
        }

    }

    return 0;
}