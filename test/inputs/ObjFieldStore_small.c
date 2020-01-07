#include <stdio.h>

struct node {
    struct node *next;
    int value;
};

int main() {
    struct node N;
    N.next = NULL;
    N.value = 1;
    printf("Value of N is %d\n", N.value);
}
