#include <stdio.h>

int main() {
    int i;
    int a[8], b[8], c[8];
    int x = 5, y = 10;

    if (0) {
        printf("This code is unreachable\n");
    }

    for (i = 0; i < 8; i++) {
        a[i] = i * 2;
    }

    for (i = 0; i < 8; i++) {
        b[i] = i + 3;
    }

    for (i = 0; i < 8; i++) {
        int invariant = x * y;
        c[i] = invariant + i * 4;
    }


    for (i = 0; i < 8; i++) {
        if (i == 0) {
            printf("First iteration special case\n");
        }
        a[i] = a[i] + 1;
    }

    for (i = 0; i < 8; i++) {
        b[i] = b[i] * 2;
    }

    for (i = 0; i < 8; i++) {
        int t = x * y;
        int z = x * y;
        c[i] = t + z;
    }

    for (i = 0; i < 8; i++) {
        printf("a[%d]=%d, b[%d]=%d, c[%d]=%d\n",
               i, a[i], i, b[i], i, c[i]);
    }

    return 0;
}