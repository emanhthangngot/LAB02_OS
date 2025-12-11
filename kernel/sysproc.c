#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;


  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  // lab pgtbl: your code here.
  uint64 base;
  int npages;
  uint64 mask_addr;

  // Parse arguments
  argaddr(0, &base);
  argint(1, &npages);
  argaddr(2, &mask_addr);

  // Limit max pages to scan
  if(npages > 64) {
    return -1;
  }

  struct proc *p = myproc();
  unsigned int abits = 0;

  // Check each page
  for(int i = 0; i < npages; i++) {
    uint64 va = base + i * PGSIZE;
    pte_t *pte = walk(p->pagetable, va, 0);

    if(pte == 0) {
      continue;  // Page not mapped
    }

    // Check if accessed bit is set
    if(*pte & PTE_A) {
      abits |= (1 << i);
      // Clear the access bit
      *pte &= ~PTE_A;
    }
  }

  // Copy result to user space
  if(copyout(p->pagetable, mask_addr, (char *)&abits, sizeof(abits)) < 0) {
    return -1;
  }

  return 0;
}
#endif

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
