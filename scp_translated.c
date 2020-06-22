#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PATHLEN 512
int main()
{
	//for file in translated.txt:
	//	scp -P 2222 file owen@127.0.0.1:file
	
	// TODO: automatically use the password with this trick and sshpass: 
	//https://stackoverflow.com/questions/50096/how-to-pass-password-to-scp
	char line[PATHLEN];	
	char command[1256];
	FILE *translated = fopen("translated.txt","r");
	while(fgets(line,PATHLEN,translated)!=NULL)
	{
		line[strlen(line)-1] = '\0';
		sprintf(command,"echo owen10298 |sudo -S sshpass -p %cowen10298%c scp -P 2222 %s owen@127.0.0.1:%s",(char)34,(char)34,line,line);
		printf("about to execute: %s\n",command);
		int r = system(command); 
		
		if(r != 0)
		{
			printf("failed to run scp on %s\n",line);	
		}
		memset(command,0,1256);
		memset(line,0,PATHLEN);
	}
	
	return 0;
}
