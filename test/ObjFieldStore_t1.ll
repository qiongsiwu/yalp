; RUN:  opt -load-pass-plugin  %shlibdir/libObjFieldStore%shlibext -passes="print<obj-field-store>" -disable-output 2>&1 %s\
; RUN:   | FileCheck %s

; A much more complicated example that involves
; - struct and arrays
; - global instances of structs and arrays
; - struct pointers and arrays passed into functions
; Note that with global variables, gep is an operator, not an instruction

; CHECK: Store instructions are
; CHECK-NEXT:  store i32 1, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr, i64 0, i64 0), align 16
; CHECK-NEXT:	destn type: [10 x i32]	field 0 idx i64 0	field 1 idx i64 0
; CHECK-NEXT:  store i32 2, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr, i64 0, i64 5), align 4
; CHECK-NEXT:	destn type: [10 x i32]	field 0 idx i64 0	field 1 idx i64 5
; CHECK-NEXT:  store i32 1, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 0), align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 0
; CHECK-NEXT:  store i32 2, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 1), align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 1
; CHECK-NEXT:  store i32 1, i32* %4, align 4
; CHECK-NEXT:	destn type: i32	field 0 idx i64 0
; CHECK-NEXT:  store i32 2, i32* %6, align 4
; CHECK-NEXT:	destn type: i32	field 0 idx i64 1
; CHECK-NEXT:  store i32 42, i32* %5, align 4
; CHECK-NEXT:	destn type: [10 x i32]	field 0 idx i64 0	field 1 is not constant
; CHECK-NEXT:  store i32 %3, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 0), align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 0
; CHECK-NEXT:  store i32 2, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 1), align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 1
; CHECK-NEXT:  store i32 %5, i32* %7, align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 0
; CHECK-NEXT:  store i32 2, i32* %9, align 4
; CHECK-NEXT:	destn type: %struct.s2 = type { i32, i32, i32 }	field 0 idx i32 0	field 1 idx i32 1

%struct.s2 = type { i32, i32, i32 }

@arr = common global [10 x i32] zeroinitializer, align 16
@S = common global %struct.s2 zeroinitializer, align 4

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @foo() #0 {
  store i32 1, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr, i64 0, i64 0), align 16
  store i32 2, i32* getelementptr inbounds ([10 x i32], [10 x i32]* @arr, i64 0, i64 5), align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @foo2() #0 {
  store i32 1, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 0), align 4
  store i32 2, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 1), align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @foo3(i32*) #0 {
  %2 = alloca i32*, align 8
  store i32* %0, i32** %2, align 8
  %3 = load i32*, i32** %2, align 8
  %4 = getelementptr inbounds i32, i32* %3, i64 0
  store i32 1, i32* %4, align 4
  %5 = load i32*, i32** %2, align 8
  %6 = getelementptr inbounds i32, i32* %5, i64 1
  store i32 2, i32* %6, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @bar(i32) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  %4 = sext i32 %3 to i64
  %5 = getelementptr inbounds [10 x i32], [10 x i32]* @arr, i64 0, i64 %4
  store i32 42, i32* %5, align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @bar2(i32) #0 {
  %2 = alloca i32, align 4
  store i32 %0, i32* %2, align 4
  %3 = load i32, i32* %2, align 4
  store i32 %3, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 0), align 4
  store i32 2, i32* getelementptr inbounds (%struct.s2, %struct.s2* @S, i32 0, i32 1), align 4
  ret void
}

; Function Attrs: noinline nounwind optnone ssp uwtable
define void @bar3(i32, %struct.s2*) #0 {
  %3 = alloca i32, align 4
  %4 = alloca %struct.s2*, align 8
  store i32 %0, i32* %3, align 4
  store %struct.s2* %1, %struct.s2** %4, align 8
  %5 = load i32, i32* %3, align 4
  %6 = load %struct.s2*, %struct.s2** %4, align 8
  %7 = getelementptr inbounds %struct.s2, %struct.s2* %6, i32 0, i32 0
  store i32 %5, i32* %7, align 4
  %8 = load %struct.s2*, %struct.s2** %4, align 8
  %9 = getelementptr inbounds %struct.s2, %struct.s2* %8, i32 0, i32 1
  store i32 2, i32* %9, align 4
  ret void
}
