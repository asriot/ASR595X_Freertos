RTOS_C_SRCDIRS += Source \
        Source/portable/GCC

# heap management selection, choose 1 from the portable/MemMang/heap_*.c
ifeq ($(MULTI_HEAP_REGIONS), 1)
RTOS_C_SRCS += Source/portable/MemMang/heap_5.c
else
RTOS_C_SRCS += Source/portable/MemMang/heap_4.c
endif

RTOS_C_SRCS += Source/portable/GCC/port.c

RTOS_ASM_SRCDIRS += Source/portable/GCC

RTOS_INCDIRS += . \
        Source/include \
		Source/portable/GCC

