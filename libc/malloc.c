#include<stdio.h>

struct mem_block{
    size_t size;
    struct mem_block * next;
    int free;
};

typedef struct mem_block * m_block;

void main(){
    printf("%ld %ld\n",sizeof(m_block),sizeof(struct mem_block));
}
