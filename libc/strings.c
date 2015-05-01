#include<string.h>



int strcmp(const char *s1,const char * s2) {
    int i=0;
    for(i=0; s1[i]==s2[i]; i++) {
        if(s1[i]=='\0')
            return 0;
    }
    return s1[i]-s2[i];
}



int strlen(char *s)
{
    int i = 0;
    while(s[i] != '\0')
        i++;
    return i;

}

void strcat(char* s1, char* s2)
{
    int i=0, j=0;
    while (s1[i] != '\0')
    {
        i++;
    }
    while (s2[j] != '\0')
    {
        s1[i] = s2[j];
        i++;
        j++;
    }
    s1[i] = '\0';
}

void strcpy(char* dest, char* src)
{
    int i=0;
    while(src[i] != '\0')
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}
