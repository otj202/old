//we need to do two phases! why? because we dont know the user's ssh info(nor should we), so we can only scp from client to the server 
//while logged in as client on client machine
//follow whiteboard
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#define PATHLEN 512
typedef enum bool{false,true} bool;
void parse_output(char *outputfile, char *store_parsed);
void translate_parsed(char *parsed,char *translated);
char *translate_path(char *rel_path, char *cwd);
void untangle_dotdots(int rootlen,char *stored_root,char *retbuf);
int main(int argc, char **argv)
{
	//this should work the same as it did before. 
	/*TODO: after error checking server-side code$DONE$:
	 *make sure this works like it should$DONE$
	 *write a few lines of shell code that scp each file in translated.txt onto the server, @ identical pathandname $DONE$
 	 *update runme.sh to do the following instead of compiling and running runme.c:
	 *compile and run this(runme_client.c)
	 *ssh into the server
	 *compile and run runme_server.c while sshed into the server
	 *close the ssh connection, returning to the client
	 *do shellcode to scp translated pathandnames onto the server(mentioned already above)
	 not bad -- pretty close! basically works except the occasional failure of scp (7/277 times) on random files that are different each run. we will fix whatever causes this weird mistake, trying the failed files several times if necessary.*/ 
	//and finally (with shellcode if possible or with some c if necessary) scp the target program onto the server 

	//once this TODO is done, you'll be a lot closer!	

	//on the client
	parse_output("./open_files.txt","./parsed.txt");
	translate_parsed("./parsed.txt","./translated.txt");
	printf("translate_parsed successful with no seg faults!!!!!\n");
	return 0;
}
/* read the output from running dtruss on the specified program. each line in the file will say "open("file/path/and/name"). so
 * open the file in read mode
 * open store_parsed in write mode
 * read outputfile in a loop, line by line
 * subscript the line from the first " to the next ", 
 * write this substring as a new line in store_parsed
*/   	
void parse_output(char *outputfile, char *store_parsed)
{
	FILE *outp = fopen(outputfile,"r");
	FILE *parsed = fopen(store_parsed,"w");
	char line[PATHLEN];
	while(fgets(line,PATHLEN, outp) !=NULL)
	{
		char c, file[PATHLEN];
		bool isOpen = false;
		int start = -1;
		int end = -1;
		int i;
		for(i=1;&line[i]!= NULL && line[i] != '\0' ;i++)
		{
			c=line[i];
			if(c=='^')
			{
				break;
			}
			if( c == 92)
			{
				end= i;
				isOpen = false;
				break;
			}
			
			if(c == '"' && isOpen == false)
			{
					isOpen = true;
					start = i;
			}
		}
		if(start < 0)
		{
			continue;
		}	
	// copy line[start+1:endi]
		strncpy(file,&line[start+1],end-start-1);
		//not sure if this is write
		fputs(file,parsed);
		fputs("\n",parsed);
		memset(line,0,PATHLEN);	
		memset(file,0,PATHLEN);
	} 
	fclose(outp);
	fclose(parsed);
}


void translate_parsed(char *parsed,char *translated)
{	
	FILE *parse = fopen(parsed,"r");
	FILE *translate = fopen(translated,"w");
	char rel_path[PATHLEN];
	char cwd[PATHLEN];
	getcwd(cwd,PATHLEN);
	while(fgets(rel_path,PATHLEN,parse)!= NULL)
	{
		char *abspath;
		if(rel_path[0] == '/')
		{	
 			fputs(rel_path,translate);
		}	
		else
		{
			abspath = translate_path(rel_path,cwd);	
 			fputs(abspath,translate);
			free(abspath);
		}
		memset(rel_path,0,PATHLEN);
	}
	fclose(parse);
	fclose(translate);
}

//ok, change of plans
//for each path in parsed.txt,
// if it's a relative path, translate it to a direct one, by:
//pasting it next to getcwd()
//"untangling dotdots", inline.
//(much easier). plus, ironically, we can keep "untangle_dotdots" subroutine
//remove the dotdot hack from add_file
// then add_file should stil work on all of these, but newRoot will be the root
//(on the webserver we will have to set this as the root directory --> Virtual machine???.....but that's for later
char *translate_path(char *rel_path, char *cwd)
{			
	int rootlen = 1 + strlen(cwd) + strlen(rel_path);
	char *bufp =(char *) malloc(rootlen * sizeof(char));
	char *retbuf =(char *) malloc(rootlen * sizeof(char));
	
	strncpy(bufp,cwd,strlen(cwd));	
	strncpy(bufp + strlen(cwd),"/",1);	
	strncpy(bufp+strlen(cwd)+1,rel_path,strlen(rel_path)-1);
	strncpy(bufp +strlen(cwd)+strlen(rel_path),"\0",1);
	
	untangle_dotdots(rootlen-1,bufp,retbuf);
	free(bufp);
	return retbuf;
}

//loop through the path name, per folder name. 
//if the folder name is "..", delete it and the one before it, unless the one before it is the first "/"
void untangle_dotdots(int rootlen,char *stored_root,char *retbuf)
{	
	int i;
	int rbi=0;
	int p_ind=0;
	int c_ind=0;
	char current[PATHLEN];
	int *slashes = (int *) malloc(sizeof(int) * rootlen);
	int *incSlashes = (int *) malloc(sizeof(int) * rootlen);	
	char d;
	int si=0;
	//compute slashes
	for(i=0;(d=stored_root[i])!='\0';i++)
	{
		if(d == '/')
		{
			slashes[si] = i;
			incSlashes[si++]=i;
		}
	}
	char c;
	//compute incSlashes
	while(c_ind < si)
	{
		//put the name of the current folder into current
		if(c_ind < si-1)
		{
			strncpy(current,stored_root + slashes[c_ind] + 1,slashes[c_ind+1]-slashes[c_ind]-1);
			current[slashes[c_ind+1]-slashes[c_ind]-1] = '\0';
		}
		else
		{
			strncpy(current,stored_root + slashes[c_ind]+1,rootlen - slashes[c_ind]-1);
			current[rootlen - slashes[c_ind]-1] = '\0';
		}
		if(strcmp(current,"..") == 0)
		{
			incSlashes[c_ind] =-1;
			incSlashes[p_ind] =-1;
			p_ind = p_ind == 0 ? 0 : p_ind - 1;
			c_ind++;
		}
		else
		{
			p_ind = c_ind++;
		}
	}
	
	//copy path into retbuf appropriately 
	for(i=0;i<si;i++)
	{
		if(incSlashes[i]!=-1)
		{
			if(i < si-1)
			{
				strncpy(retbuf+rbi,stored_root + slashes[i], slashes[i+1]-slashes[i]);
				rbi+= slashes[i+1]-slashes[i];
			}
			else
			{
				strncpy(retbuf+rbi,stored_root + slashes[i], rootlen-slashes[i]);
				rbi+= rootlen-slashes[i];
			}
		}
	}	
	//et voila
	retbuf[rbi]='\0';
	free(slashes);
	free(incSlashes);	
}
