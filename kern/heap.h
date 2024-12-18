#include <param.h>
#include <defs.h>

#ifndef __STRIDE_HEAP_H__
#define __STRIDE_HEAP_H__

#define STRIDE_MAX    100

struct heap {
     struct proc* p[NPROC*2];
     int size;
}pheap;

static void pheap_swap(int a, int b)
{
     struct proc* pt = pheap.p[a];
     pheap.p[a] = pheap.p[b];
     pheap.p[b] = pt;
}


static void pheap_up(int index)
{
     int next = index>>1;
     while(index > 0 && pheap.p[index]->pass < pheap.p[next]->pass)
     {
          pheap_swap(index, next);
          index = next;
          next >>= 1;
     }
}

static void pheap_down(int index)
{
     while((index<<1)+1 < pheap.size)
     {
          int next = (index<<1)+1;
          if(next+1 < pheap.size && pheap.p[next+1]->pass < pheap.p[next]->pass)
               next++;
          if(pheap.p[index]->pass > pheap.p[next]->pass)
          {
               pheap_swap(index,next);
               index = next;
          }
          else
               return;
     }
}

void
pheap_init()
{
     memset(pheap.p,0,sizeof(pheap.p));
     pheap.size = 0;
}

int
pheap_push(struct proc* p)
{
     if(pheap.size >= NPROC)
          panic("pheap is full");

     p->pass += STRIDE_MAX / p->priority;
     pheap.p[pheap.size] = p;
     pheap.size++;
     pheap_up(pheap.size-1);
     return 0;
}

struct proc*
pheap_pop()
{
     struct proc* ret = pheap.p[0];
     pheap_swap(0, pheap.size-1);
     pheap.size--;
     pheap_down(0);
     return ret;
}

void
pheap_show()
{
     int i;
     if(pheap.size == 0) return ;
     cprintf("%d process queue: \n", pheap.size);
     for(i=0; i<pheap.size; i++)
     {
          cprintf("%s\n",pheap.p[i]->name);
     }
}

#endif
