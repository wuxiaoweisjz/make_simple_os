%include "pm.inc"

org   0x9000

VRAM_ADDRESS  equ  0x000a0000

jmp   LABEL_BEGIN

[SECTION .gdt]
 ;                                  段基址          段界限                属性
LABEL_GDT:          Descriptor        0,            0,                   0  
LABEL_DESC_CODE32:  Descriptor        0,      SegCode32Len - 1,       DA_C + DA_32
LABEL_DESC_VIDEO:   Descriptor        0B8000h,         0ffffh,            DA_DRW
LABEL_DESC_VRAM:    Descriptor        0,         0ffffffffh,            DA_DRW
LABEL_DESC_STACK:   Descriptor        0,             TopOfStack,        DA_DRWA+DA_32

GdtLen     equ    $ - LABEL_GDT
GdtPtr     dw     GdtLen - 1
           dd     0

SelectorCode32    equ   LABEL_DESC_CODE32 -  LABEL_GDT
SelectorVideo     equ   LABEL_DESC_VIDEO  -  LABEL_GDT
SelectorStack     equ   LABEL_DESC_STACK  -  LABEL_GDT
SelectorVram      equ   LABEL_DESC_VRAM   -  LABEL_GDT

LABEL_IDT:
%rep  33
    Gate  SelectorCode32, SpuriousHandler,0, DA_386IGate
%endrep

.021h:
    Gate  SelectorCode32, keyBoardHandler,0, DA_386IGate

%rep  10
    Gate  SelectorCode32, SpuriousHandler,0, DA_386IGate
%endrep

.2ch:
    Gate  SelectorCode32, mouseHandler,0, DA_386IGate
IdtLen  equ $ - LABEL_IDT
IdtPtr  dw  IdtLen - 1
        dd  0


[SECTION  .s16]
[BITS  16]
LABEL_BEGIN:
     mov   ax, cs
     mov   ds, ax
     mov   es, ax
     mov   ss, ax
     mov   sp, 0100h
     ;calculate memeory 
ComputeMemory:
  mov ebx, 0 
  mov di, MemChKBuf
.loop:
  mov eax, 0e820h
  mov ecx, 20
  mov edx, 0534d4150h
  int 15h
  jc LABEL_MEM_CHK_FAIL
  add di, 20
  inc dword [dwMCRNumber]
  cmp ebx, 0
  jne .loop
  jmp LABEL_MEM_CHK_OK
LABEL_MEM_CHK_FAIL:
  mov dword [dwMCRNumber], 0
LABEL_MEM_CHK_OK:
     mov   al, 0x13
     mov   ah, 0
     int   0x10

    ; call init8259A

     xor   eax, eax
     mov   ax,  cs
     shl   eax, 4
     add   eax, LABEL_SEG_CODE32
     mov   word [LABEL_DESC_CODE32 + 2], ax
     shr   eax, 16
     mov   byte [LABEL_DESC_CODE32 + 4], al
     mov   byte [LABEL_DESC_CODE32 + 7], ah

     xor   eax, eax
     mov   ax, ds
     shl   eax, 4
     add   eax,  LABEL_GDT
     mov   dword  [GdtPtr + 2], eax


     lgdt  [GdtPtr]

     cli   ;关中断
     call init8259A
     ;prepare for loading IDT
     xor   eax, eax
     mov   ax,  ds
     shl   eax, 4
     add   eax, LABEL_IDT
     mov   dword [IdtPtr + 2], eax
     lidt  [IdtPtr]

     in    al,  92h
     or    al,  00000010b
     out   92h, al

     mov   eax, cr0
     or    eax , 1
     mov   cr0, eax



     jmp   dword  SelectorCode32: 0

init8259A:
     mov  al, 011h
     out  020h, al
     call io_delay
  
     out 0A0h, al
     call io_delay

     mov al, 020h
     out 021h, al
     call io_delay

     mov  al, 028h
     out  0A1h, al
     call io_delay

     mov  al, 004h
     out  021h, al
     call io_delay

     mov  al, 002h
     out  0A1h, al
     call io_delay

     mov  al, 001h
     out  021h, al
     call io_delay

     out  0A1h, al
     call io_delay

     mov  al, 11111001b ;允许键盘中断
     out  21h, al
     call io_delay

     mov  al, 11101111b;允许鼠标终端
     out  0A1h, al
     call io_delay

     ret

io_delay:
     nop
     nop
     nop
     nop
     ret

     [SECTION .s32]
     [BITS  32]
     LABEL_SEG_CODE32:
     ;initialize stack for c code
     mov  ax, SelectorStack
     mov  ss, ax
     mov  esp, TopOfStack

     mov  ax, SelectorVram
     mov  ds,  ax

     mov  ax, SelectorVideo
     mov  gs, ax

     sti

     %include "write_vga_desktop.func"
    
     jmp  $

_SpuriousHandler:
SpuriousHandler  equ _SpuriousHandler - $$
    
    
     iretd
_keyBoardHandler: 
keyBoardHandler equ _keyBoardHandler - $$
     push es
     push ds
     pushad
     mov  eax, esp
     push eax

     call intHandlerFromC

     pop  eax
     mov  esp, eax
     popad
     pop  ds
     pop  es
    
     iretd
_mouseHandler:
mouseHandler equ _mouseHandler - $$
     push es
     push ds
     pushad
     mov  eax, esp
     push eax

     call intHandlerForMouse

     pop  eax
     mov  esp, eax
     popad
     pop  ds
     pop  es

     iretd


    io_hlt:  ;void io_hlt(void);
      HLT
      RET

    io_cli:
      CLI
      RET
    
    io_sti:
      STI
      RET
    io_stihlt:
      STI
      HLT
      RET

    io_in8:
      mov  edx, [esp + 4]
      mov  eax, 0
      in   al, dx
      ret

    io_in16:
      mov  edx, [esp + 4]
      mov  eax, 0
      in   ax, dx
      ret

    io_in32:
      mov edx, [esp + 4]
      in  eax, dx
      ret

    io_out8:
       mov edx, [esp + 4]
       mov al, [esp + 8]
       out dx, al
       ret

    io_out16:
       mov edx, [esp + 4]
       mov eax, [esp + 8]
       out dx, ax
       ret

    io_out32:
        mov edx, [esp + 4]
        mov eax, [esp + 8]
        out dx, eax
        ret

    io_load_eflags:
        pushfd
        pop  eax
        ret

    io_store_eflags:
        mov eax, [esp + 4]
        push eax
        popfd
        ret

    get_memory_block_count:
        mov eax, [dwMCRNumber];
        ret 
    get_adr_buffer:
        mov eax, MemChKBuf;
        ret

%include "fontData.inc"

SegCode32Len   equ  $ - LABEL_SEG_CODE32
MemChKBuf: times 256 db 0
dwMCRNumber: dd 0
[SECTION .gs]
ALIGN 32
[BITS 32]
LABEL_STACK:
times 512  db 0
TopOfStack  equ  $ - LABEL_STACK

