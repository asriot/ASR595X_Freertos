/******************************************************************************
 * @file     gcc.ld
 * @brief    GNU Linker Script for ASR595X in ilm Mode
 * @version  V1.0.0
 * @date     18. Nov 2021
 ******************************************************************************/
OUTPUT_ARCH( "riscv" )

ENTRY( _start )

/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size  = 0x10000;  /* required amount of heap  */

MEMORY
{
    ilm (rxai!w) :      ORIGIN = 0x80012000, LENGTH = 952K
    ITCM (xrw)   :      ORIGIN = 0x00081000, LENGTH = 124K   /*ITCM for Critical Code, first 4k for ble rom data+bss*/
    ram (wxa!ri) :      ORIGIN = 0x20FC0000, LENGTH = 320K   /*SoC ram config=0x19, wifi shareram is 32K, BT is 32K, left sw available 64K,256+64 = 320K*/
    SHARED_MEMORY(rw) : ORIGIN = 0x60000000, LENGTH = 64K
}


SECTIONS
{
  __stack_size = DEFINED(__stack_size) ? __stack_size : 12K;

  .init           :
  {
    *(.vtable)
    KEEP (*(SORT_NONE(.init)))
  } >ilm AT>ilm

  .ilalign         :
  {
    . = ALIGN(4);
    PROVIDE( _ilm_lma = . );
  } >ilm AT>ilm

  .ialign         :
  {
    PROVIDE( _ilm = . );
  } >ilm AT>ilm

  .text           :
  {
    *(.text.unlikely .text.unlikely.*)
    *(.text.startup .text.startup.*)
    *(.text)
    *(EXCLUDE_FILE (*lib_a-memcpy.o) .text*)
    *(.gnu.linkonce.t.*)
  } >ilm AT>ilm

  .rodata : ALIGN(4)
  {
    . = ALIGN(4);
    *(.rdata)
    *(.rodata .rodata.*)
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

  } >ilm AT>ilm

  .fini           :
  {
    KEEP (*(SORT_NONE(.fini)))
  } >ilm AT>ilm

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
  } >ilm AT>ilm

  .init_array     :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*) SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.init_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .ctors))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >ilm AT>ilm

  .fini_array     :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*) SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.fini_array EXCLUDE_FILE (*crtbegin.o *crtbegin?.o *crtend.o *crtend?.o ) .dtors))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >ilm AT>ilm

  .ctors          :
  {
    /* gcc uses crtbegin.o to find the start of
       the constructors, so we make sure it is
       first.  Because this is a wildcard, it
       doesn't matter if the user does not
       actually link against crtbegin.o; the
       linker won't look for a file to match a
       wildcard.  The wildcard also means that it
       doesn't matter which directory crtbegin.o
       is in.  */
    KEEP (*crtbegin.o(.ctors))
    KEEP (*crtbegin?.o(.ctors))
    /* We don't want to include the .ctor section from
       the crtend.o file until after the sorted ctors.
       The .ctor section from the crtend file contains the
       end of ctors marker and it must be last */
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .ctors))
    KEEP (*(SORT(.ctors.*)))
    KEEP (*(.ctors))
  } >ilm AT>ilm

  .dtors          :
  {
    KEEP (*crtbegin.o(.dtors))
    KEEP (*crtbegin?.o(.dtors))
    KEEP (*(EXCLUDE_FILE (*crtend.o *crtend?.o ) .dtors))
    KEEP (*(SORT(.dtors.*)))
    KEEP (*(.dtors))
  } >ilm AT>ilm


  .lalign         :
  {
    . = ALIGN(4);
    PROVIDE( _data_lma = . );
  } >ilm AT>ilm

  .dalign         :
  {
    . = ALIGN(4);
    PROVIDE( _data = . );
  } >ram AT>ilm

  .data          :
  {
    *(.data .data.*)
    *(.gnu.linkonce.d.*)
    . = ALIGN(8);
    PROVIDE( __global_pointer$ = . + 0x800 );
    *(.sdata .sdata.* .sdata*)
    *(.gnu.linkonce.s.*)
    . = ALIGN(8);
    *(.srodata.cst16)
    *(.srodata.cst8)
    *(.srodata.cst4)
    *(.srodata.cst2)
    *(.srodata .srodata.*)
  } >ram AT>ilm

  . = ALIGN(4);
  PROVIDE( _edata = . );
  PROVIDE( edata = . );

  _itcm_lma = _data_lma + SIZEOF(.data);
  /* Initialized func sections goes into ROM, load LMA copy after code */
  .itcm_section		: AT(_itcm_lma)
  {
    . = ALIGN(4);
    PROVIDE(_itcm_start = .);
    *(seg_critical_function)          /* .func sections */
    *(seg_flash_driver)
    *(seg_wf_flash_driver)
    *lib_a-memcpy.o (.text*)
    . = ALIGN(4);
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
    . = ALIGN(4);
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


  .stack ORIGIN(ram) + LENGTH(ram) - __stack_size :
  {
    PROVIDE( _heap_end = . );
    . = __stack_size;
    PROVIDE( _sp = . );
  } >ram AT>ram
}

  /* left ram for heap */
  PROVIDE (heap_start = _end);
  PROVIDE (heap_end   = _heap_end);
  PROVIDE (heap_len   = heap_end - heap_start);
  ASSERT  ((heap_len > _Min_Heap_Size), "Error: No room left for the heap")


