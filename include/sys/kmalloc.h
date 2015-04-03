struct m_blk{
    size_t size;//8bytes
    struct m_blk *next;//8bytes
    struct m_blk *prev;//8bytes
    size_t free;//8bytes
    void *ptr;//8bytes
    char start[1];//1byte
};

typedef struct m_blk *p_blk;

p_blk head=NULL;//head points to the head of the memory linked list
p_blk tail=NULL;//tail points to the last node of the linked list, brk is data chunk + meta

#define M_BLK_SIZE 40 //define size as 24 eventhough 25
#define ALIGNMENT 8
#define MIN_SPLIT_SIZE 8
#define ALIGN(x) (((x)+(ALIGNMENT-1)) & ~ (ALIGNMENT-1))

void* kmalloc(size_t size);
p_blk expand_brk(size_t size);
void * sbrk(uint64_t offset);
int can_split(p_blk t,size_t size);
void split(p_blk t,size_t size);
p_blk get_mem_node(size_t size);

void kfree(void* ptr);
p_blk get_meta_ptr(void *ptr);
int remove_last_blk(p_blk t);
p_blk get_meta_ptr(void *ptr);
int valid_ptr(void *ptr);

int first_alloc = 1;
void* brk_ptr = (void*) 0;