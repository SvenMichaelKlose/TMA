move    macro
        db 66h
        mov #1,#2
        #em
movel   macro
        db 66h,67h
        mov #1,#2
        #em
movl    macro
        db 67h
        mov #1,#2
        #em

adde    macro
        db 66h
        add #1,#2
        #em
addel   macro
        db 66h,67h
        add #1,#2
        #em
addl    macro
        db 67h
        add #1,#2
        #em

sube    macro
        db 66h
        sub #1,#2
        #em
subel   macro
        db 66h,67h
        sub #1,#2
        #em
subl    macro
        db 67h
        sub #1,#2
        #em

cmpe    macro
        db 66h,67h
        cmp #1,#2
        #em
cmpel   macro
        db 66h,67h
        cmp #1,#2
        #em
cmpl    macro
        db 67h
        cmp #1,#2
        #em

clre    macro
        db 66h
        xor #1,#1
        #em

pushe   macro
        db 66h
        push #1
        #em

pope    macro
        db 66h
        pop #1
        #em

dece    macro
        db 66h
        dec #1
        #em

ince    macro
        db 66h
        inc #1
        #em

shle    macro
        db 66h
        shl #1,#2
        #em

shre    macro
        db 66h
        shr #1,#2
        #em

ore     macro
        db 66h
        or #1,#2
        #em
orel    macro
        db 66h,67h
        or #1,#2
        #em
orl     macro
        db 67h
        or #1,#2
        #em

ande    macro
        db 66h
        and #1,#2
        #em

mule    macro
        db 66h
        mul #1
        #em

dive    macro
        db 66h
        div #1
        #em

teste   macro
        db 66h
        test #1,#2
        #em

testel  macro
        db 66h,67h
        test #1,#2
        #em

testl   macro
        db 67h
        test #1,#2
        #em

stosd   macro
        db 66h
        stosw
        #em
stosdl  macro
        db 66h,67h
        stosw
        #em

lodsd   macro
        db 66h
        lodsw
        #em
lodsdl  macro
        db 66h,67h
        lodsw
        #em

