.global __core_id
__core_id:
    mrs x0, mpidr_el1
    and x0, x0, #0xFF	
	ret

.global __set_vector_table
__set_vector_table:
    msr vbar_el1, x0  
    ret
/*
 * void __asm_dcache_level(level)
 *
 * flush or invalidate one level cache.
 *
 * x0: cache level
 * x1: 0 clean & invalidate, 1 invalidate only
 * x2~x9: clobbered
 */
.globl __asm_dcache_level
__asm_dcache_level:
    lsl    x12, x0, #1
    msr    csselr_el1, x12        /* select cache level */
    isb                /* sync change of cssidr_el1 */
    mrs    x6, ccsidr_el1        /* read the new cssidr_el1 */
    and    x2, x6, #7        /* x2 <- log2(cache line size)-4 */
    add    x2, x2, #4        /* x2 <- log2(cache line size) */
    mov    x3, #0x3ff
    and    x3, x3, x6, lsr #3    /* x3 <- max number of #ways */
    clz    w5, w3            /* bit position of #ways */
    mov    x4, #0x7fff
    and    x4, x4, x6, lsr #13    /* x4 <- max number of #sets */
    /* x12 <- cache level << 1 */
    /* x2 <- line length offset */
    /* x3 <- number of cache ways - 1 */
    /* x4 <- number of cache sets - 1 */
    /* x5 <- bit position of #ways */

loop_set:
    mov    x6, x3            /* x6 <- working copy of #ways */
loop_way:
    lsl    x7, x6, x5
    orr    x9, x12, x7        /* map way and level to cisw value */
    lsl    x7, x4, x2
    orr    x9, x9, x7        /* map set number to cisw value */
    tbz    w1, #0, 1f
    dc    isw, x9
    b    2f
1:    dc    cisw, x9        /* clean & invalidate by set/way */
2:    subs    x6, x6, #1        /* decrement the way */
    b.ge    loop_way
    subs    x4, x4, #1        /* decrement the set */
    b.ge    loop_set

    ret

/*
 * void __asm_flush_dcache_all(int invalidate_only)
 *
 * x0: 0 clean & invalidate, 1 invalidate only
 *
 * flush or invalidate all data cache by SET/WAY.
 */
.globl __asm_dcache_all
__asm_dcache_all:
    mov    x1, x0
    dsb    sy
    mrs    x10, clidr_el1        /* read clidr_el1 */
    lsr    x11, x10, #24
    and    x11, x11, #0x7        /* x11 <- loc */
    cbz    x11, finished        /* if loc is 0, exit */
    mov    x15, lr
    mov    x0, #0            /* start flush at cache level 0 */
    /* x0  <- cache level */
    /* x10 <- clidr_el1 */
    /* x11 <- loc */
    /* x15 <- return address */

loop_level:
    lsl    x12, x0, #1
    add    x12, x12, x0        /* x0 <- tripled cache level */
    lsr    x12, x10, x12
    and    x12, x12, #7        /* x12 <- cache type */
    cmp    x12, #2
    b.lt    skip            /* skip if no cache or icache */
    bl    __asm_dcache_level    /* x1 = 0 flush, 1 invalidate */
skip:
    add    x0, x0, #1        /* increment cache level */
    cmp    x11, x0
    b.gt    loop_level

    mov    x0, #0
    msr    csselr_el1, x0        /* restore csselr_el1 */
    dsb    sy
    isb
    mov    lr, x15

finished:
    ret

.global __invalidate_icache_all
__invalidate_icache_all:
    ic      iallu                   // 无效化所有指令缓存
    dsb     sy                     // 非共享域的数据同步屏障
    isb                             // 同步上下文
    ret

.globl __flush_dcache_all
__flush_dcache_all:
    mov    x0, #0
    b    __asm_dcache_all
    ret

.global __flush_tlb
__flush_tlb:
    dsb ishst
    tlbi vmalle1is
	dsb sy
	isb
	ret

.global __set_translation_table_base
__set_translation_table_base:
	msr ttbr0_el1, x0
	msr ttbr1_el1, x0
	isb
	ret

.global __irq_enable
__irq_enable:
	msr daifclr, #0x2
	ret

.global __irq_disable
__irq_disable:
	msr daifset, #0x2
	ret

.global __smp_lock
__smp_lock:
    ldxr    w1, [x0]        
    cbnz    w1, __lock_wait          
    mov     w1, #1         
    stxr    w2, w1, [x0]
    cbnz    w2, __smp_lock
    dmb     ish 
    ret

__lock_wait:  
	wfe                     
    b       __smp_lock

.global __smp_unlock
__smp_unlock:
    dmb     ishst           
    mov     w1, #0          
    str     w1, [x0]        
    dsb     ish             
    sev                     
    ret                     