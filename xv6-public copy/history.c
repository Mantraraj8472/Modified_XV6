#include"types.h"
#include"stat.h"
#include"user.h"
#define MAX_HISTORY 16

int main(){
	char hist[MAX_HISTORY*128];

	int histct = gethistory((char*)hist);
	histct=histct;
	
	// printf(1,"Hello\n %p\n",hist);

	for(int i=0;i<=histct;i++)
		printf(1,"%d->%s\n",i+1,&hist[i*128]);

	exit();
	

}