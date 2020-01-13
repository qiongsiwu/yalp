; RUN:  opt -load-pass-plugin  %shlibdir/libObjFieldStore%shlibext -verify -passes="obj-field-store" -S %s\
; RUN:   | FileCheck %s

; Check to see if the transformation gets simple case correct

; String for the struct
; CHECK: @struct_1 = global [36 x i8] c"%struct.s1 = type { i32, i16, i64 }\00"

; The format string
; CHECK-NEXT: @PrintFormatStr_2 = global [39 x i8] c"[ValueProf] | %s | %p | %d | %d | %d \0A\00"

%struct.s1 = type { i32, i16, i64 }

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @foo(%struct.s1*) #0 {
  %2 = alloca %struct.s1*, align 8
  store %struct.s1* %0, %struct.s1** %2, align 8
  %3 = load %struct.s1*, %struct.s1** %2, align 8
  %4 = getelementptr inbounds %struct.s1, %struct.s1* %3, i32 0, i32 0
; CHECK: %5 = call i32 (i8*, ...) @printf
  store i32 3, i32* %4, align 8
  %5 = load %struct.s1*, %struct.s1** %2, align 8
  %6 = getelementptr inbounds %struct.s1, %struct.s1* %5, i32 0, i32 1
; CHECK: %8 = call i32 (i8*, ...) @printf
  store i16 5, i16* %6, align 4
  %7 = load %struct.s1*, %struct.s1** %2, align 8
  %8 = getelementptr inbounds %struct.s1, %struct.s1* %7, i32 0, i32 2
; CHECK: %11 = call i32 (i8*, ...) @printf
  store i64 1024, i64* %8, align 8
  ret void
}

; CHECK: declare i32 @printf(i8* nocapture readonly, ...) #0
; CHECK: attributes #0 = { nounwind }
