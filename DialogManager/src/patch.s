/*
    DialogManager plugin for FAR Manager
    Copyright (C) 2003 Vadim Yegorov
    Copyright (C) 2004-2005 Vadim Yegorov and Alex Yaroslavsky

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

.data
	.long
.text
	.align 4
.globl _Invoke_Patch
_Invoke_Patch:
	pusha
	pushl	$-1
	pushl	_DialogMutex
	call	_WaitForSingleObject@8
	popa
	push %eax
	push %edx
	/*save old ip*/
	mov 8(%esp),%eax
	mov %eax,_old_ip
	/*copy params*/
	mov 12(%esp),%eax
	mov %eax,_Param1
	mov 16(%esp),%eax
	mov %eax,_Param2
	mov 20(%esp),%eax
	mov %eax,_Param3
	mov 24(%esp),%eax
	mov %eax,_Param4
	/*unpatch*/
	mov (_code_ptr),%eax
	mov (_first_data),%edx
	mov %edx,(%eax)
	mov (_second_data),%edx
	mov %edx,4(%eax)
	pop %edx
	pop %eax
	add $4,%esp
	call *(_code_ptr)
	push %eax
	push %edx
	mov (_code_ptr),%eax
	mov (_first_new_data),%edx
	mov %edx,(%eax)
	mov (_second_new_data),%edx
	mov %edx,4(%eax)
	pop %edx
	pop %eax
	push %eax
	push %eax
	mov _old_ip,%eax
	mov %eax,4(%esp)
	mov _Param4,%eax
	push %eax
	mov _Param3,%eax
	push %eax
	mov _Param2,%eax
	push %eax
	mov _Param1,%eax
	push %eax
	call _DialogHook@20
	pusha
	pushl	_DialogMutex
	call	_ReleaseMutex@4
	popa
	ret
