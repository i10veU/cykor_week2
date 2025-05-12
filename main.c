#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //getCurrentWorkingdirectory
#include <string.h> //strCoMPare if same return 0

#define maxline 1024

int main(void){
    char input[maxline];

    while(1){
        char cwd[1024];//cwd 쉘에 표시
        getcwd(cwd, sizeof(cwd));
        printf("%s", cwd);

        if (fgets(input, maxline, stdin))==NULL break;//한줄 입력
        input[strcspn(input, "\n")] = 0; //strcount stop not : 개행 제거
        if(strcmp(input, "exit")==0) break; //exit나오면 탈출
    }

    return 0;
}