; RUN:  opt -load-pass-plugin  %shlibdir/libObjFieldStore%shlibext -passes="print<obj-field-store>" -disable-output 2>&1 %s\
; RUN:   | FileCheck %s

; Sanity check to see if the pass can filter out stores to sources from a GEP

; CHECK:Store instructions are
; CHECK-NEXT:  store i32 3, i32* %4, align 8
; CHECK-NEXT:	destn type: %struct.s1 = type { i32, i16, i64 }	field 0 idx i32 0	field 1 idx i32 0
; CHECK-NEXT:  store i16 5, i16* %6, align 4
; CHECK-NEXT:	destn type: %struct.s1 = type { i32, i16, i64 }	field 0 idx i32 0	field 1 idx i32 1
; CHECK-NEXT:  store i64 1024, i64* %8, align 8
; CHECK-NEXT:	destn type: %struct.s1 = type { i32, i16, i64 }	field 0 idx i32 0	field 1 idx i32 2

%struct.s1 = type { i32, i16, i64 }

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @foo(%struct.s1*) #0 {
  %2 = alloca %struct.s1*, align 8
  store %struct.s1* %0, %struct.s1** %2, align 8
  %3 = load %struct.s1*, %struct.s1** %2, align 8
  %4 = getelementptr inbounds %struct.s1, %struct.s1* %3, i32 0, i32 0
  store i32 3, i32* %4, align 8
  %5 = load %struct.s1*, %struct.s1** %2, align 8
  %6 = getelementptr inbounds %struct.s1, %struct.s1* %5, i32 0, i32 1
  store i16 5, i16* %6, align 4
  %7 = load %struct.s1*, %struct.s1** %2, align 8
  %8 = getelementptr inbounds %struct.s1, %struct.s1* %7, i32 0, i32 2
  store i64 1024, i64* %8, align 8
  ret void
}
