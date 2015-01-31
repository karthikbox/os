#include<stdio.h>
#include<stdlib.h>

int strcmp(const char *s1,const char *s2);

int main(int argc, char *argv[],char *envp[]){
  printf("hello\n");
  //printf("%d\n",strcmp("cd","cd"));
  return 1;
}

int strcmp(const char *s1,const char * s2){
  int i=0;
  for(i=0;s1[i]==s2[i];i++){
    if(s1[i]=='\0')
      return 0;
  }
  return s1[i]-s2[i];
}
