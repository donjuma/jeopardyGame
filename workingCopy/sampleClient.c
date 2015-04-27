#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
void main()
{
    int sid;
    char s[10]={},s1[10]={};
    struct sockaddr_in ssock,csock;
    sid=socket(AF_INET,SOCK_STREAM,0);
    ssock.sin_family=AF_INET;
    ssock.sin_addr.s_addr=inet_addr("127.0.0.1");
    ssock.sin_port=htons(9734);
    connect(sid,(struct sockaddr *)&ssock,sizeof(ssock));
    while(1)
    {
        printf("\n Enter the string:");
        scanf("%s",s);
        write(sid,(void*)s,strlen(s));
        if(strlen(s)==0)
            break;
        sleep(1);
        read(sid,(void*)s1,sizeof(s1));
        printf("\n The received string is:%s\n",s1);
    }
    close(sid);
}
