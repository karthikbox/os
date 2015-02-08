#include<stdio.h>

struct mem_block{
    size_t size;
    m_block next;
    int free;
};

typedef struct mem_block * m_block;

void main(){
    printf("%d %d\n",sizeof(int),sizeof(struct mem_block));
}
