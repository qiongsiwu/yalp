; RUN: %clang -c -emit-llvm %S/inputs/ObjFieldStore_small.c -o - \
; RUN:   | opt -load %shlibdir/libObjFieldStore%shlibext -legacy-obj-field-store -verify -o OFS_exe1.bin
; RUN: lli ./OFS_exe1.bin | FileCheck %s

; CHECK: [ValueProf] %struct.node = type { %struct.node*, i32 }
; CHECK-NEXT: [ValueProf] %struct.node = type { %struct.node*, i32 }
; CHECK-NEXT: Value of N is 1
