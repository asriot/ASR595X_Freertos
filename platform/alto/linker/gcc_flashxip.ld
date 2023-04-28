/******************************************************************************
 * @file     gcc.ld
 * @brief    GNU Linker Script for ASR595X in flash XIP Mode
 * @version  V1.0.0
 * @date     18. Nov 2021
 ******************************************************************************/
OUTPUT_ARCH( "riscv" )
/* Define entry label of program */
ENTRY( _start )

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x10000;  /* required amount of heap  */

MEMORY
{
    flash (rxai!w) :    ORIGIN = 0x80012000, LENGTH = 1780K
    ITCM (xrw)   :      ORIGIN = 0x00081000, LENGTH = 124K   /*ITCM for Critical Code, first 4k for ble rom data+bss*/
    ram (wxa!ri) :      ORIGIN = 0x20FC0000, LENGTH = 320K   /*SoC ram config=0x19, wifi shareram is 32K, BT is 32K, left sw available 64K,256+64 = 320K*/
    SHARED_MEMORY(rw) : ORIGIN = 0x60000000, LENGTH = 64K
}
/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH,ILM and RAM.
 * It references following symbols, which must be defined in code:
 *   _Start : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   _ilm_lma
 *   _ilm
 *   __etext
 *   _etext
 *   etext
 *   _eilm
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   _data_lma
 *   _edata
 *   edata
 *   __data_end__
 *   __bss_start
 *   __fbss
 *   _end
 *   end
 *   __heap_end
 *   __StackLimit
 *   __StackTop
 *   __STACK_SIZE
 */

SECTIONS
{
  __stack_size = DEFINED(__stack_size) ? __stack_size : 8K;
  __vtable_size = 256;

  .init           :
  {
    /* vector table locate at flash */
    *(.vtable)
    KEEP (*(SORT_NONE(.init)))
  } >flash AT>flash

  .ilalign         :
  {
    . = ALIGN(4);
    /* Create a section label as _ilm_lma which located at flash */
    PROVIDE( _ilm_lma = . );
  } >flash AT>flash

  .ialign         :
  {
    /* Create a section label as _ilm which located at flash */
    PROVIDE( _ilm = . );
  } >flash AT>flash

  /* Code section located at flash */
  .text           :
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(EXCLUDE_FILE (*lib_a-memcpy.o *lib_a-rand.o *_clzsi2.o *portasm.S.o *port.c.o *lega_rtos.c.o).text*)

    *(.gnu.linkonce.t.*)
  } >flash AT>flash

  .rodata : ALIGN(4)
  {
    . = ALIGN(4);
    *(.rdata)
    *(EXCLUDE_FILE (*_clz.o).rodata)
    *(EXCLUDE_FILE (*_clz.o).rodata*)
    /* section information for initial. */
    . = ALIGN(4);
    __rt_init_start = .;
    KEEP(*(SORT(.rti_fn*)))
    __rt_init_end = .;
    /* section information for finsh shell */
    . = ALIGN(4);
    __fsymtab_start = .;
    KEEP(*(FSymTab))
    __fsymtab_end = .;
    . = ALIGN(4);
    __vsymtab_start = .;
    KEEP(*(VSymTab))
    __vsymtab_end = .;
    *(.gnu.linkonce.r.*)
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
  } >flash AT>flash

  .fini           :
  {
    KEEP (*(SORT_NONE(.fini)))
  } >flash AT>flash

  . = ALIGN(4);

  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);
  PROVIDE( _eilm = . );


  .preinit_array  :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >flash AT>flash

  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >flash AT>flash

  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >flash AT>flash

  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
     * the constructors, so we make sure it is
     * first.  Because this is a wildcard, it
     * doesn't matter if the user does not
     * actually link against crtbegin.o; the
     * linker won't look for a file to match a
     * wildcard.  The wildcard also means that it
     * doesn't matter which directory crtbegin.o
     * is in.
     */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
     * the crtend.o file until after the sorted ctors.
     * The .ctor section from the crtend file contains the
     * end of ctors marker and it must be last
     */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } >flash AT>flash

  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } >flash AT>flash


  .stack :
  {
    . = ALIGN(4);
    PROVIDE( _sp_start = . );
    . = . + __stack_size;
    PROVIDE( _sp = . );
  } >ram AT>ram

  .lalign         :
  {
    . = ALIGN(4);
    PROVIDE( _data_lma = . );
  } >flash AT>flash

  .dalign         :
  {
    . = ALIGN(4);
    PROVIDE( _data = . );
  } >ram AT>flash

  /* Define data section virtual address is ram and physical address is flash */
  .data          :
  {
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata .sdata.* .sdata*)
    *(.gnu.linkonce.s.*)
  } >ram AT>flash

  . = ALIGN(4);
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  /* vtable_section */
  .vtable_section	:
  {
    . = ALIGN(4);
    PROVIDE(_vtable_start = .);
    . = . + __vtable_size;
    PROVIDE(_vtable_end = .);
  } >ITCM AT>ITCM

  _itcm_lma = _data_lma + SIZEOF(.data);
  /* Initialized func sections goes into ROM, load LMA copy after code */
  .itcm_section		: AT(_itcm_lma)
  {
    . = ALIGN(4);
    PROVIDE(_itcm_start = .);
    *(seg_critical_function)          /* .func sections */
    *(seg_flash_driver)
    *(seg_wf_flash_driver)
    *lib_a-rand.o(.text*)
    *lib_a-memcpy.o (.text*)
    *_clzsi2.o (.text*)
    *portasm.S.o(.text*)
    *port.c.o (.text*)
    *lega_rtos.c.o (.text*)
    . = ALIGN(8);
    *_clz.o (.rodata .rodata*)
    *(.gcc_except_table*)
    . = ALIGN(8);
    PROVIDE(_itcm_end = .);
  } >ITCM

  __shmem_s = LOADADDR(sharemem);
  /* shared RAM*/
  sharemem(NOLOAD)    :
  {
      PROVIDE( __shmem_s = . );
      . = ALIGN(4);
      _shmem_s = .;
      *(SHAREDRAM)
      . = ALIGN(4);
      _shmem_e = .;
      PROVIDE( _shmem_e = . );
  } >SHARED_MEMORY

  .bss            :
  {
    PROVIDE( _fbss = . );
    PROVIDE( __bss_start = . );
    *(.sbss*)
    *(.gnu.linkonce.sb.*)
    *(.bss .bss.*)
    *(.gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN(8);
    _ebss = .;         /* define a global symbol at bss end */
  } >ram AT>ram

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  . = ALIGN(8);
  PROVIDE( _end = . );
  PROVIDE( end = . );
}

  /* left ram for heap */
  PROVIDE(itcm_heap_start = _itcm_end);
  PROVIDE(itcm_heap_end   = ORIGIN(ITCM) + LENGTH(ITCM));
  PROVIDE(itcm_heap_len   = ORIGIN(ITCM) + LENGTH(ITCM) - itcm_heap_start);

  PROVIDE(sram_heap_start = _ebss);
  PROVIDE(sram_heap_end   = ORIGIN(ram) + LENGTH(ram));
  PROVIDE(sram_heap_len   = ORIGIN(ram) + LENGTH(ram) - sram_heap_start);

  PROVIDE(heap_len   = (itcm_heap_len + sram_heap_len));
  ASSERT ((heap_len > _Min_Heap_Size), "Error: No room left for the heap")
