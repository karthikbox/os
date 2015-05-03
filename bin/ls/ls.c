#include<stdio.h>
#include<stdlib.h>

void print_dir(char *str);

int main(int argc,char* argv[],char* envp[]){

    //TEST FOR opendir,readdir,closedir
  int i=0;
  for(i=1;i<argc;i++){
    if(argc>2)
      perror("%s:\n",argv[i]);
    print_dir(argv[i]);
    if(argc >2)
      perror("\n");
  }
  if(i==1){
    print_dir(".");
  }
  return 0;
}

void print_dir(char *dir){
  void *dirp=opendir(dir);
  if(!dirp){
    perror("opendir failed\n");
    exit(1);
  }
  if((uint64_t)dirp==1)
	  return;
  struct dirent *ptr;
  while((ptr=readdir(dirp))){
    printf("%s\n",ptr->d_name);
  }
  if(closedir(dirp)==-1){
    perror("closedir failed\n");
    exit(1);
  }
}
