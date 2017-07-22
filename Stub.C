#include <stdio.h>
#include <elf.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <dlfcn.h>
#include <sys/mman.h>

#define FILESIZE 7348 // it must be changed because it's related to file that will run from memory.
#define PAGESIZE 4096 // it depends on OS.

#define CONVERT(x) x/PAGESIZE * PAGESIZE + PAGESIZE

void getfilename(char *dest ){

char path[PATH_MAX]="/proc/self/exe";
int result=readlink(path,dest,PATH_MAX);

if(result==-1){
	perror("readlink");
}

}

void find_library(Elf32_Phdr *PHT_INTERNALS[],int number,int fd,int size){

int i=0;
int current=0;
Elf32_Word dt_needed_array[1];
Elf32_Addr str_table;
Elf32_Dyn *internal_element=(Elf32_Dyn*)malloc(sizeof(Elf32_Dyn));

while(i<number){

if(PHT_INTERNALS[i]->p_type==PT_DYNAMIC){
		break;
	}

i++;

}

lseek(fd,size-FILESIZE+PHT_INTERNALS[i]->p_offset,SEEK_SET);
read(fd,internal_element,sizeof(Elf32_Dyn));

while(internal_element->d_tag!=DT_NULL){
	if(internal_element->d_tag==DT_STRTAB){
		str_table=internal_element->d_un.d_ptr;
	}

	if(internal_element->d_tag==DT_NEEDED){
     dt_needed_array[current]=internal_element->d_un.d_val;
     current++;

	}
read(fd,internal_element,sizeof(Elf32_Dyn));
}



int stroff=PHT_INTERNALS[2]->p_offset+(str_table-PHT_INTERNALS[2]->p_vaddr);
char *buff=malloc(100);

i=0;
while(i<current){

lseek(fd,size-FILESIZE+stroff+dt_needed_array[i],SEEK_SET);

read(fd,buff,100);

void *ptr=dlopen(buff,RTLD_LAZY);

if(ptr!=NULL){
	printf("it's loaded.\n");
}

i++;
}

}

void *create_area(int pagesize,int fd,int offset,int realsize){

char *addr=mmap(NULL,pagesize, PROT_WRITE | PROT_READ | PROT_EXEC,MAP_ANONYMOUS|MAP_SHARED,-1,0);

char *temp=malloc(realsize);

lseek(fd,offset,SEEK_SET);
read(fd,temp,realsize);
memcpy(addr,temp,realsize);

free(temp);

return addr;

}



void mapfile2memory(int fd,Elf32_Phdr *PHT_INTERNALS[],Elf32_Ehdr *EHT,int size,char *eps){

int i=0;
Elf32_Addr EP;
void *addr;


while(i<EHT->e_phnum){

if(PHT_INTERNALS[i]->p_type==PT_LOAD){

addr=create_area(CONVERT(PHT_INTERNALS[i]->p_filesz),fd,size-FILESIZE+PHT_INTERNALS[i]->p_offset,PHT_INTERNALS[i]->p_filesz);

if(addr==-1){
 	perror("mmap");
 }

 if(PHT_INTERNALS[i]->p_vaddr < EHT->e_entry && PHT_INTERNALS[i]->p_vaddr+PHT_INTERNALS[i]->p_memsz > EHT->e_entry)
 {

 	EP=addr+(EHT->e_entry - PHT_INTERNALS[i]->p_vaddr);

 }

}

i++;
}

eps=EP;

}

int main(int argc,char **argv){

struct stat st;
int fd,size,i;
char PATH[PATH_MAX];
Elf32_Ehdr *EHT;

getfilename(PATH);

stat(PATH,&st);
size=st.st_size;


fd=open(PATH,O_RDONLY);

if(fd==-1){
	perror("open");
}

EHT=malloc(52);

lseek(fd,size-FILESIZE,SEEK_SET);
read(fd,EHT,sizeof(Elf32_Ehdr));

Elf32_Phdr *PHT[EHT->e_phnum];

lseek(fd,size-FILESIZE+EHT->e_phoff,SEEK_SET);

i=0;

while(EHT->e_phnum > i) {

	PHT[i]=(Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
	read(fd,PHT[i],EHT->e_phentsize);

i++;

}

find_library(PHT,EHT->e_phnum,fd,size);

char *entry_point=malloc(10);
mapfile2memory(fd,PHT,EHT,size,entry_point);

__asm__ volatile ("mov %%eax,%0\n\t"

    		   : "=m"(entry_point));

__asm__ ("jmp %eax");

exit(0);

}

