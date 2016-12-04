#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct{
	int key;
	int number;
	char massage[255];
}nodetype;

typedef struct{
	nodetype heap[BUFSIZ];
	int heap_size;
}HeapType;

void get_file(HeapType *h);
void init(HeapType *h);
void heap_insert(HeapType *h, nodetype item);
nodetype delete_heap(HeapType *h);
void put_file(HeapType *h);


int main(){
	HeapType h;
	nodetype nt[BUFSIZ];
	int k=0;
	int i;
	init(&h);
	get_file(&h);

	for(i=1;i<h.heap_size;i++){
		printf("%d %d %s\n",h.heap[i].key, h.heap[i].number, h.heap[i].massage);
	}
	printf("\n");
	i=0;

	
	put_file(&h);
	return 0;
}

//파일에서 값 얻어오는 함수
void get_file(HeapType *h){
	nodetype item;


	FILE *f = fopen("test.txt", "r");

	if (f == NULL){
		puts("파일이존재하지않습니다");
		exit(1);
	}
	while (!feof(f)){
		fscanf(f,"%d %d %s",&item.key, &item.number, item.massage);

		heap_insert(h, item);

		item.key =0;
		item.number =0;
	}
	fclose(f);

}

//힙 초기화 함수
void init(HeapType *h){
	h->heap_size = 0;
}

void heap_insert(HeapType *h,nodetype item){ 

	int i; 
	i=++(h->heap_size); 

	while((i!=1) && (item.number > h->heap[i/2].number)){ 
		h->heap[i]=h->heap[i/2];       
		i/=2; 
	} 

	h->heap[i]=item;
}

//힙 삭제,정렬 함수
nodetype delete_heap(HeapType *h){
	int parent, child;
	nodetype item, temp;

	item = h->heap[1];
	temp = h->heap[(h->heap_size)--];
	parent = 1;
	child = 2;
	while(child <= h->heap_size){
		if((child < h->heap_size) && (h->heap[child].number) < h->heap[child + 1].number)
			child++;
		if(temp.number >= h->heap[child].number) break;
		
		h->heap[parent] = h->heap[child];
		parent = child;
		child *= 2;
	}
	h->heap[parent] = temp;
	return item;
}

void put_file(HeapType *h){
	nodetype item;
	int i;
	int size = h->heap_size;

	FILE *f = fopen("test1.txt", "w");
	printf("1 %d\n", size);
	for (i = 0; i<size - 1; i++){
		item = delete_heap(h);
		printf("%d %d %s\n", item.key, item.number, item.massage);
		fprintf(f, "%d %d %s\n", item.key, item.number, item.massage);
	}
	fclose(f);
}







