#include<stdio.h>
#include<stdlib.h>
typedef struct block{
	int bytes;
	int cache_tag;
	short valid;
	short dirty;
	int ci,co;   //to reach back to the main memory
}cache;

typedef struct b{
	int ct;
	int ci;
	int co;
	int data;
	short reset;
}main_mem;

void do_all_zero(cache *cp, int len){
	while(len != 0){
		cp->cache_tag = 0;
		cp->valid = 0;
		cp->dirty = 0;
		cp->ci=0; cp->co=0;
		cp++;
		len--;
	}
}
	
int add(main_mem *p, main_mem *q, cache *cp, int r, int c, int lines, int sets, int *addr_p1, int *addr_p2);

int find_in_cache(int addr, cache *cp, cache **p, int lines, int sets);

void replace_data(int addr1, int addr2, main_mem *p, main_mem *q, cache *cp, int r, int c, int line, int sets, int stress);

void insert_into_mem(main_mem *p, cache *cp, int i, int j, int r, int c);

void insert_into_cache(main_mem *p, cache **cp, int r, int c, int i, int j, int stress);

void main_mem_add_write(main_mem *p, int r, int c, int *addr_p)  //Function name self descriptive
{
	int ct=random()/10000000;
	int co=random()/10000000,ci=random()/10000000,i;
	for(i=0;i<r*c;i++,p++)
	{
		p->data=random()/100000;
		p->co=co++;
		if(!i%3)
			co=random()/10000000;
		p->ci=ci++;
		if(!i%3)
			ci=random()/10000000;
		p->ct=ct++;
		if(!i%3)
			ct=random()/10000000;
		p->reset=0;
		addr_p[i]= p->ct;
	}
}

void print_the_hex(int a,int b,int c){    //for printing the hex codes of the locations
	char ch[20];
	int x,i,j;
	x=sprintf(ch,"%x%x%x",a,b,c);
	printf("0x");
	for(i=0;i<x;i++){
		if(ch[i]!='0' || ch[i]!='x')
			printf("%c",ch[i]);
	}
}			

void pack_up(cache *cp, main_mem *p, int r, int c, int l, int s)
{
	int i,j;
	cache *t;
	for(i=0;i<r*c;i++,p++){
		t=cp;
		for(j=0;j<l*s;j++,t++){
			if(p->ci == t->ci && p->co == t->co && p->reset != 1)
				p->data=t->bytes;
			}
		}
}

void print_the_cache(cache *cp, int lines, int sets){
	int i,j=0;
	for(i=0;i<lines*sets;i++,cp++){	
		printf("%p  %d 0x%x\t",cp,cp->bytes,cp->cache_tag);
		j++;
		if(j%sets == 0)
			printf("\n");
	}
}
	
int main(){
	int i, r, c, j, loc=0, lines, sets, *addr_dir1, *addr_dir2, cm;
	main_mem *main_mem_p1, *main_mem_p2, *t,*t1;
	cache *cache_mem_p;
	/*Input rows and columns*/
	printf("Enter number of rows and columns :-\n");
	printf("Rows :");
	scanf("%d",&r);
	printf("Columns :");
	scanf("%d",&c);
	printf("\nEnter the data for cache :-\n");
	printf("Enter the number of lines :");
	scanf("%d",&lines);
	printf("Enter the number of sets :");
	scanf("%d",&sets);
	printf("\nGenerating random data for the two matrices........");
	/*End*/
	main_mem_p1=(main_mem*)malloc(sizeof(main_mem)*r*c);
	main_mem_p2=(main_mem*)malloc(sizeof(main_mem)*r*c);
	cache_mem_p=(cache*)malloc(sizeof(cache)*lines*sets);
	addr_dir1=(int*)malloc(sizeof(int)*r*c);
	addr_dir2=(int*)malloc(sizeof(int)*r*c);
	j=lines*sets;
	do_all_zero(cache_mem_p,j);
	/*Space allocaed for the cache and the main memories*/
	main_mem_add_write(main_mem_p1,r,c,addr_dir1);    //data written to the main memory containg the first array
	main_mem_add_write(main_mem_p2,r,c,addr_dir2);    //data written to the main memory containg the second array
	printf("\nThe first array with corresponding memory locations.....\n");
	j=0;
	t=main_mem_p1;
	for(i = 0 ; i < r*c ; i++,main_mem_p1++){
		print_the_hex(main_mem_p1->ct, main_mem_p1->ci, main_mem_p1->co);
		printf(" %d\t",main_mem_p1->data);
		j++;
		if(j == c){
			printf("\n");
			j=0;
		}
	}
	main_mem_p1=t;
	t=main_mem_p2;
	printf("\nThe second array with corresponding memory locations.....\n");
	j=0;
	for(i = 0 ; i < r*c ; i++,main_mem_p2++){
		print_the_hex(main_mem_p2->ct, main_mem_p2->ci, main_mem_p2->co);
		printf(" %d\t",main_mem_p2->data);
		j++;
		if(j == c){
			printf("\n");
			j=0;
		}
	}
	main_mem_p2=t;
	cm=add(main_mem_p1, t, cache_mem_p, r, c, lines, sets, addr_dir1, addr_dir2);
	printf("\n\nThe value after addition is ....\n");
	t=main_mem_p1;
	pack_up(cache_mem_p,t,r,c,lines,sets);
	for(i = 0 ; i < r*c ; i++,main_mem_p1++){
		printf(" %d\t",main_mem_p1->data);
		j++;
		if(j == c){
			printf("\n");
			j=0;
		}
	}
	printf("\nTotal number of cache misses : %d\n",cm);
	main_mem_p1=t;
	free(main_mem_p1);
	free(main_mem_p2);
	free(cache_mem_p);
	return 0;
}

int add(main_mem *p, main_mem *q, cache *cp, int r, int c, int lines, int sets, int *addr_p1, int *addr_p2){
	int i, cache_miss=0,stress=(r*c)/2;
	cache *p1=NULL, *p2=NULL;
	for(i=0;i<r*c;i++){
		printf("\n");
		print_the_cache(cp,lines,sets);
		if(find_in_cache(addr_p1[i], cp, &p1, lines, sets) && find_in_cache(addr_p2[i], cp, &p2, lines, sets)){
				p1->bytes = p1->bytes + p2-> bytes;
				p1->dirty=1;
				p2->dirty=1;
				printf("\nCache Hit");
		}
		else
		{
			replace_data(addr_p1[i], addr_p2[i], p, q, cp, r, c, lines, sets, stress);
			stress--;
			cache_miss++;
			printf("\nCache Miss");
			continue;
		}
	}
	return cache_miss;
}

int find_in_cache(int addr, cache *cp, cache **p, int lines, int sets){
	int i;
	for(i=0 ; i<lines*sets ; i++,cp++){
		if(cp->cache_tag == addr){
			(*p)=cp;
			return 1;
		}
	}
	return 0;
}
void replace_data(int addr1, int addr2, main_mem *p, main_mem *q, cache *cp, int r, int c, int lines, int sets, int stress){
	int i,j,k;
	cache *t=cp;
	i=lines*sets/2;
	j=lines*sets;
	insert_into_mem(p, cp, 0, i, r, c);
	for(k=0;k<i;k++,cp++);
	insert_into_mem(q, cp, i, j, r, c);
	for(i=0;i<r*c;i++,p++)
		if(addr1 == p->ct)
			break;
	for(i=0;i<r*c;i++,q++)
		if(addr2 == q->ct)
			break;
	i=lines*sets/2;
	j=lines*sets;
	insert_into_cache(p, &t, r, c, 0, i, stress);
	printf("I am q :");print_the_hex(q->ct,q->ci,q->co);
	insert_into_cache(q, &t, r, c, i, j, stress);
	stress--;
}
void insert_into_mem(main_mem *p, cache *cp, int i, int j, int r, int c){
	int k;
	main_mem *t=p;
	for(;i<j;i++,cp++){
		p=t;
		if(cp->dirty == 1){
			for(k=0 ; k < r*c ; k++,p++){
				if(cp->ci == p->ci && cp->cache_tag == p->ct){
					p->data = cp->bytes;
					p->reset = 1;
				}
			}
		}
	}
}
void insert_into_cache(main_mem *p, cache **cp, int r, int c, int i, int j, int stress){
	for(i;i<j;i++,p++,(*cp)++){
		if(stress == i){
			return;
			
		}
		(*cp)->bytes = p->data;
		(*cp)->cache_tag = p->ct;
		(*cp)->ci = p->ci;
		(*cp)->co = p->co;
		(*cp)->dirty=1;
		
	}
}
