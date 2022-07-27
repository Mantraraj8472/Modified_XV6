#include "stat.h"
#include "user.h"
#include "fcntl.h"

int main(){
	int ctime,retime,rutime,stime,ttime;
	int pid=fork();
	if(pid == -1){
		printf(2,"child not created\n");
		exit();
	}
	if(pid){
		wait2(&ctime,&retime,&rutime,&stime,&ttime);
		printf(1,"Creation Time: %d\n",ctime);
		printf(1,"Runnable Time: %d\n",retime);
		printf(1,"Running Time: %d\n",rutime);
		printf(1,"Sleeping Time: %d\n",stime);
		printf(1,"Termination Time: %d\n",ttime);
	}
	else{
		// sleep(100);
		
		// for(int i=0;i<100000000;i++) j++;
		int fd;
		fd=open("Mantra.txt",O_CREATE|O_RDWR);
		char buf[2000]="Testing wait2 system call\n";
		for(int i=0;i<10;i++)
		write(fd,buf,sizeof(buf));
		close(fd);
	}
	exit();
}