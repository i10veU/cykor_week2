#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define maxline 1024

int main(void){
    char input[maxline];

    while(1){
        char cwd[1024];//cwd 쉘에 표시
        getcwd(cwd, sizeof(cwd));

        char *usrname = getenv("USER"); //get USERname environment

        printf("%s@%s", username, cwd);
        

        if (fgets(input, maxline, stdin))==NULL break;//한줄 입력
        input[strcspn(input, "\n")] = 0; //strcount stop not : 개행 제거
        if(strcmp(input, "exit")==0) break; //exit나오면 탈출
    }

    return 0;
}