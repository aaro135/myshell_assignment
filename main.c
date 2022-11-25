#include<unistd.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>

#define NUM 1024
#define SUB 128
#define ENV_BUFFER 1024
#define CLOSE printf("\033[0m"); //关闭彩色字体
#define RED printf("\033[31m"); //红色字体 警告或错误
#define GREEN printf("\033[36m"); //深绿色字体 指令
#define GREEN2 printf("\033[32m"); //绿色字体 命令解释程序的提示符
#define YELLOW printf("\033[33m"); //黄色字体
#define BLUE printf("\033[34m"); //蓝色字体
#define LIGHT printf("\033[1m"); //高亮
#define MOVERIGHT(y) printf("\033[%dC",(y)) //右移光标
#define RESET_CURSOR() printf("\033[u") //恢复光标

char cmd_str[1024];//用于用户输入指令
char* sub_str[SUB];//用于分离指令和携带的选项
char env_buffer[ENV_BUFFER];//用于储存要增加的环境变量

void Help(){
    GREEN
    printf("-----------------------\n" );
    printf("可以输入在环境变量中的命令\n");
    printf("命令格式： [CMD] [选项]\n");
    printf("例子：\t\t功能\n" );
    printf("cd [目录]\t更改当前目录到另一个<目录>\n");
    printf("ls [选项]\t输出目录下的文件\n");
    printf("env\t\t列出所有环境变量字符串的设置\n" );
    printf("help\t\t列出shell的使用方法和基本功能\n" );
    printf("echo [内容]\t显示出echo后的内容并换行\n" );
    printf("process\t\t输出 shell 当前的一系列子进程。\n" );
    printf("quit,exit,bye\t退出 shell。\n" );
    printf("-----------------------\n");
    CLOSE
}
//获取进程
void Process(){
//通过创建子进程执行系统命令
    pid_t id =fork();
    if(!id){
        //child
        //运行系统命令，execlp会从系统环境变量里找到文件并且执行
        execlp("pstree","-p",NULL);
        //如果指令有误才会执行下面的语句
        RED
        printf("-myshell: %s: command not found\n",sub_str[0]);
        CLOSE
        exit(0);
    }
    else if(id>0){
        //father
        int status=0;
        int ret=0;
        ret =waitpid(id,&status,0);//阻塞等待子进程
        if(ret){
        }
        else{
            RED
            printf("创建进程失败！\n");
            CLOSE
            exit(2);
        }
    }
}

int main(){
    //获取提示符信息
    char pwd[SUB];//用于获取当前路径
    memset(pwd,0,SUB);
    //显示hostname
    char host_name[NUM];
    memset(host_name,0,NUM);
    while(1){
        //获取当前工作目录
        getcwd(pwd,1000);
        //printf(pwd);
        //获取hostname
        gethostname(host_name, 1024);
        //printf("%s",host_name);
        //输出shell信息，仿造成现在用的zsh的颜色
        LIGHT
        printf("\033[0m\033[34m[\033[0m\033[36m%s@\033[0m\033[33m%s:\033[0m\033[32m%s\033[0m\033[34m]\033[0m\033[31m$\033[0m ",getenv("USER"),host_name,pwd);
        CLOSE
        //刷新输入
        fflush(stdout);
        //用户输入指令
        memset(cmd_str,0,NUM);//每次输入前清空上一条指令
        fgets(cmd_str,NUM,stdin);//可能携带选项 所以需要按行读入 不能用scanf
        cmd_str[strlen(cmd_str)-1]=0;//将回车去掉
        //分离指令和选项，strtok可以分割字符串
        sub_str[0]=strtok(cmd_str," ");//拿到指令
        int i=1;//对ls 等指令增加默认选项
        if(!strcmp(sub_str[0],"ls"))
            sub_str[i++]="--color=auto";
        while(sub_str[i++]=strtok(NULL," ")){;}//拿到选项
        //针对内建指令
        if(!strcmp(sub_str[0],"cd")&&sub_str[1]){ //cd
            chdir(sub_str[1]);
            continue;
        }
        if(!strcmp(sub_str[0],"export")&&sub_str[1]){//增加环境变量
            strcpy(env_buffer,sub_str[1]);//因为每次sub_str会被修改
            putenv(env_buffer);//环境变量需要一块独立的空间
            continue;
        }
        if(!strcmp(sub_str[0],"process")&&!sub_str[1]){
            Process();
            continue;
        }
        if((!strcmp(sub_str[0],"exit")||!strcmp(sub_str[0],"quit")||!strcmp(sub_str[0],"bye"))&&!sub_str[1]){ //退出
            exit(0);
        }
        if(!strcmp(sub_str[0],"help")&&!sub_str[1]){ //显示提示
            Help();
            continue;
        }
        //通过创建子进程执行系统命令
        pid_t id =fork();
        if(!id){
            //child
            //运行系统命令，execvp会从系统环境变量里找到文件并且执行
            execvp(sub_str[0],sub_str);
            //如果指令有误才会执行下面的语句
            RED
            printf("-myshell: %s: command not found\n",sub_str[0]);
            CLOSE
            exit(0);
        }
        else if(id>0){
            //father
            int status=0;
            int ret=0;
            ret =waitpid(id,&status,0);//阻塞等待子进程
            if(ret){
            }
            else{
                RED
                printf("创建进程失败！\n");
                CLOSE
                exit(2);
            }
        }
    }
}
