.data
.global _I
_I:
.long 0
.global _J
_J:
.long 0
.comm _C, 129, 1
.comm _Vet, 1161, 1

.text

.global _Vetor
_Vetor:
                                movl  $0, %ebx
                                movl  %ebx, _I
R0:
                                movl  _I, %ebx
                                movl  $3, %ecx
                                cmpl  %ecx, %ebx
                                jl    R1
                                movl  $0, %ebx
                                jmp   R2
R1:
                                movl  $1, %ebx
R2:
                                cmpl  $0, %ebx
                                jne   R3
                                jmp   R4
R3:
                                movl  $0, %ebx
                                movl  %ebx, _J
R5:
                                movl  _J, %ebx
                                movl  $3, %ecx
                                cmpl  %ecx, %ebx
                                jl    R6
                                movl  $0, %ebx
                                jmp   R7
R6:
                                movl  $1, %ebx
R7:
                                cmpl  $0, %ebx
                                jne   R8
                                jmp   R9
R8:
                                jmp   R11
.data
R10:
.byte 'C','a','d','e','i','a','['
.byte 0

.text
R11:
                                leal  R10, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _I, %ebx
                                pushl %ebx
                                call  _impint
                                addl  $4, %esp
                                jmp   R13
.data
R12:
.byte ','
.byte 0

.text
R13:
                                leal  R12, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _J, %ebx
                                pushl %ebx
                                call  _impint
                                addl  $4, %esp
                                jmp   R15
.data
R14:
.byte ']',' ','<','=',' '
.byte 0

.text
R15:
                                leal  R14, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                leal  _C, %ebx
                                pushl %ebx
                                call  _lercad
                                addl  $4, %esp
                                movl  $0, %eax
                                movl  _I, %ebx
                                addl  %ebx, %eax
                                movl  _J, %ebx
                                movl  $3, %ecx
                                mull  %ecx
                                addl  %ebx, %eax
                                movl  $129, %ebx
                                mull  %ebx
                                leal  _Vet, %ebx
                                addl  %ebx, %eax
                                pushl %eax
                                leal  _C, %ebx
                                popl  %eax
                                pushl %eax
                                pushl %ebx
                                call  _copcad
                                addl  $8, %esp
                                movl  _J, %ebx
                                movl  $1, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, _J
                                jmp   R5
R9:
                                movl  _I, %ebx
                                movl  $1, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, _I
                                jmp   R0
R4:
                                movl  $0, %ebx
                                movl  %ebx, _I
R16:
                                movl  _I, %ebx
                                movl  $3, %ecx
                                cmpl  %ecx, %ebx
                                jl    R17
                                movl  $0, %ebx
                                jmp   R18
R17:
                                movl  $1, %ebx
R18:
                                cmpl  $0, %ebx
                                jne   R19
                                jmp   R20
R19:
                                movl  $0, %ebx
                                movl  %ebx, _J
R21:
                                movl  _J, %ebx
                                movl  $3, %ecx
                                cmpl  %ecx, %ebx
                                jl    R22
                                movl  $0, %ebx
                                jmp   R23
R22:
                                movl  $1, %ebx
R23:
                                cmpl  $0, %ebx
                                jne   R24
                                jmp   R25
R24:
                                jmp   R27
.data
R26:
.byte 'V','e','t','['
.byte 0

.text
R27:
                                leal  R26, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _I, %ebx
                                pushl %ebx
                                call  _impint
                                addl  $4, %esp
                                jmp   R29
.data
R28:
.byte ','
.byte 0

.text
R29:
                                leal  R28, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _J, %ebx
                                pushl %ebx
                                call  _impint
                                addl  $4, %esp
                                jmp   R31
.data
R30:
.byte ']',' ','=',' '
.byte 0

.text
R31:
                                leal  R30, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                pushl %ebx
                                movl  $0, %eax
                                movl  _I, %ebx
                                addl  %ebx, %eax
                                movl  _J, %ebx
                                movl  $3, %ecx
                                mull  %ecx
                                addl  %ebx, %eax
                                popl  %ebx
                                movl  $129, %ebx
                                mull  %ebx
                                leal  _Vet, %ebx
                                addl  %eax, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                jmp   R33
.data
R32:
.byte ' '
.byte 0

.text
R33:
                                leal  R32, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _J, %ebx
                                movl  $1, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, _J
                                jmp   R21
R25:
                                jmp   R35
.data
R34:
.byte '\','n'
.byte 0

.text
R35:
                                leal  R34, %ebx
                                pushl %ebx
                                call  _impcad
                                addl  $4, %esp
                                movl  _I, %ebx
                                movl  $1, %ecx
                                addl  %ecx, %ebx
                                movl  %ebx, _I
                                jmp   R16
R20:
                                movl  $0, %ebx
                                movl  $1, %eax
                                int   $0x80
.global _start
_start:
                                jmp   _Vetor
