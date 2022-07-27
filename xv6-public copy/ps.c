#include"stat.h"
#include"user.h"
#include"types.h"

int main(){
	struct uproc up[10];
	getprocinfo(up);
	printf(1,"Name \t PID \t PPID \t Size \t State\n");
	for(int i=0;i<10;i++){
		if(up[i].size==0) continue;
		printf(1,"%s \t %d \t %d \t %d \t %s\n",up[i].name,up[i].pid,up[i].ppid,up[i].size,up[i].state);
	}

	exit();
}
