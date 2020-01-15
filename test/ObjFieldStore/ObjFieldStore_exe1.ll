; RUN: %clang -c -emit-llvm %S/../inputs/ObjFieldStore/ObjFieldStore_small.c -o - \
; RUN:   | opt -load %shlibdir/libObjFieldStore%shlibext -legacy-obj-field-store -verify -o OFS_exe1.bin
; RUN: lli ./OFS_exe1.bin | FileCheck %s

; CHECK: [ValueProf] | %struct.node = type { %struct.node*, i32 } | %struct.node* | {{0x[0-9a-f]*}} | 0 | 0 | {{0x0|[(]nil[)]}}
; CHECK-NEXT: [ValueProf] | %struct.node = type { %struct.node*, i32 } | i32 | {{0x[0-9a-f]*}} | 0 | 1 | 5
; CHECK-NEXT: Value of N is 5
