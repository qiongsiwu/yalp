; RUN: %clang -c -emit-llvm %S/../inputs/ObjFieldStore/small_arrays.c -o - \
; RUN:   | opt -load %shlibdir/libObjFieldStore%shlibext -legacy-obj-field-store -verify -o small_arrays.bin
; RUN: lli ./small_arrays.bin | FileCheck %s

; CHECK: [ValueProf] | i32* | {{0x[0-9a-f]*}} | 3 | 23
; CHECK-NEXT: [ValueProf] | i32* | {{0x[0-9a-f]*}} | 2 | 42
; CHECK-NEXT: [ValueProf] | i32* | {{0x[0-9a-f]*}} | 3 | 59
