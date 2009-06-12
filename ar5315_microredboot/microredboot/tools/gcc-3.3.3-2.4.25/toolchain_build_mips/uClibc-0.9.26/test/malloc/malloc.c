
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define N_PTRS 1000
#define N_ALLOCS 10000
#define MAX_SIZE 0x10000

#define random_size()	(random()%MAX_SIZE)
#define random_ptr()	(random()%N_PTRS)

void test1(void);
void test2(void);

int main(int argc,char *argv[])
{
	test1();
	test2();
	return 0;
}

void test1(void)
{
	void **ptrs;
	int i,j;
	int size;

	srandom(0x19730929);

	ptrs = malloc(N_PTRS*sizeof(void *));

	for(i=0;i<N_PTRS;i++){
		ptrs[i]=malloc(random_size());
	}
	for(i=0;i<N_ALLOCS;i++){
		j=random_ptr();
		free(ptrs[j]);

		size=random_size();
		ptrs[j]=malloc(size);
		if(!ptrs[j]){
			printf("malloc failed! %d\n",i);
		}
		memset(ptrs[j],0,size);
	}
	for(i=0;i<N_PTRS;i++){
		free(ptrs[i]);
	}
}

void test2(void)
{
	void *ptr = NULL;

	ptr = realloc(ptr,100);
	if(!ptr){
		printf("couldn't realloc() a NULL pointer\n");
	}else{
		free(ptr);
	}
	
	ptr = malloc(100);
	ptr = realloc(ptr, 0);
	if(ptr){
		printf("realloc(,0) failed\n");
		free(ptr);
	}
}

