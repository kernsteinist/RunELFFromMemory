#include <stdio.h>
#include <elf.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#define PATH "stub.out"

int main(int argc,char **argv){

  int fd_stub,fd_file,size;
  struct stat st;
  
  fd_file=open(argv[1],O_RDWR);
  fd_stub=open(PATH,O_APPEND | O_RDWR);
  
  if(fd_file==-1 || fd_stub==-1){
    	exit(2);
    }
  
   stat(argv[1],&st);
   size=st.st_size;
   
   char *file=(char *)malloc(size);
   
   read(fd_file,file,size);
   write(fd_stub,file,size);


return 0;

}

