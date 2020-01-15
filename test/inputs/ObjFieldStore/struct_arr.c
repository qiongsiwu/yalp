#include <stdio.h>
#include <stdlib.h>

struct foo{
    struct foo *next;
};

int main() {
    struct foo f;
    f.next = NULL;
    struct foo **fptrs;
    fptrs = malloc(3 * sizeof(struct foo *));
    fptrs[2] = NULL;
    struct foo *fptrs_arr[3];
    fptrs_arr[2] = NULL;
    printf("hello foo\n");
}
