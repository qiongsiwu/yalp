#include <stdio.h>

int arr1[10];

int main() {
    printf("hello world\n");
    int arr2[16];

    arr1[3] = 23;
    arr1[2] = 42;
    arr2[3] = 59;

    printf("%d %d %d\n", arr1[3], arr1[2], arr2[3]);
}
