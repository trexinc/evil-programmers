Так как Борландские компиляторы не дают вручную задавать entry point то нужно линковать с программой, объект который будет содержать уже готовую "entry point". Такой объект можно создать из ниже приведенного ASM файла:

```
.386p

_TEXT SEGMENT DWORD USE32 PUBLIC 'CODE'
_TEXT ENDS

_DATA SEGMENT DWORD USE32 PUBLIC 'DATA'
_DATA ENDS

_BSS SEGMENT DWORD USE32 PUBLIC 'BSS'
_BSS ENDS

DGROUP GROUP _DATA, _BSS

ASSUME CS:_TEXT, DS:DGROUP

_TEXT SEGMENT PUBLIC DWORD USE32 'CODE'

PUBLIC EntryPoint
PUBLIC __acrtused

__acrtused label byte

EntryPoint proc near
    mov eax, 1
    ret 12
EntryPoint endp

_TEXT ENDS

END EntryPoint
```

Объект полученный после компиляции этого файла (с помощью Turbo Assembler-а) можно линковать вместе с программой для получения работающего DLL-а без startup code с помощью BCC 5.5.1