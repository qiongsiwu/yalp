; RUN: %clang -c -emit-llvm %S/../inputs/ObjFieldStore/struct_arr.c -o - \
; RUN:   | opt -load %shlibdir/libObjFieldStore%shlibext -legacy-obj-field-store -verify -o struct_arr.bin
; RUN: lli ./struct_arr.bin | FileCheck %s

; CHECK: [ValueProf] | %struct.foo = type { %struct.foo* } | %struct.foo* | {{0x[0-9a-f]*}} | 0 | 0 | 0x0
; CHECK-NEXT: [ValueProf] | %struct.foo* | %struct.foo* | {{0x[0-9a-f]*}} | 2 | 0x0
; CHECK-NEXT: ValueProf] | [3 x %struct.foo*] | %struct.foo* | {{0x[0-9a-f]*}} | 0 | 2 | 0x0
; CHECK-NEXT: hello foo
