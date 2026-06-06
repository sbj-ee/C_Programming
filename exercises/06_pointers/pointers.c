#include <stdio.h>
#include <string.h>

int double_it(int *n);
void reverse(char *s, int len);

int main(void) {

    /* --- Address-of and dereference --- */
    printf("=== Address-of (&) and dereference (*) ===\n");

    int x = 42;
    int *p = &x;       /* p holds the memory address of x */

    printf("x        = %d\n",   x);
    printf("&x       = %p\n",   (void *)&x);   /* address of x */
    printf("p        = %p\n",   (void *)p);     /* same address, stored in p */
    printf("*p       = %d\n",   *p);            /* dereference: value at that address */

    /* Writing through a pointer modifies the original variable */
    *p = 99;
    printf("after *p = 99, x = %d\n", x);

    /* --- Pointer types --- */
    printf("\n=== Pointer types ===\n");

    double d  = 3.14;
    double *pd = &d;
    printf("double  *pd: sizeof(d)=%zu, sizeof(pd)=%zu\n", sizeof(d), sizeof(pd));
    printf("*pd      = %.2f\n", *pd);

    char c  = 'Z';
    char *pc = &c;
    printf("*pc      = '%c'\n", *pc);

    /* All pointer variables are the same size regardless of what they point to */
    printf("sizeof(int*)    = %zu\n", sizeof(int *));
    printf("sizeof(double*) = %zu\n", sizeof(double *));
    printf("sizeof(char*)   = %zu\n", sizeof(char *));

    /* --- Pointer arithmetic --- */
    printf("\n=== Pointer arithmetic ===\n");

    int arr[] = {10, 20, 30, 40, 50};

    int *pa = arr;     /* array name decays to pointer to first element */
    printf("pa   points to arr[0] = %d\n", *pa);

    pa++;              /* advances by sizeof(int) bytes, not by 1 byte */
    printf("pa++ points to arr[1] = %d\n", *pa);

    pa += 2;
    printf("pa+2 points to arr[3] = %d\n", *pa);

    /* Subscript notation and pointer arithmetic are interchangeable */
    printf("\nTraversing via pointer arithmetic:\n");
    int len = sizeof(arr) / sizeof(arr[0]);
    for (int i = 0; i < len; i++) {
        printf("*(arr+%d) = %d\n", i, *(arr + i));
    }

    /* Pointer difference gives element count between two pointers */
    int *first = &arr[0];
    int *last  = &arr[4];
    printf("\nelements between first and last: %td\n", last - first);

    /* --- Pointers and strings --- */
    printf("\n=== Pointers and strings ===\n");

    char word[] = "Hello";
    char *ps = word;

    /* Walk the string with a pointer until the null terminator */
    printf("chars: ");
    while (*ps != '\0') {
        printf("%c ", *ps);
        ps++;
    }
    printf("\n");

    /* String functions return pointers — useful for chaining or offset math */
    char sentence[] = "find the needle here";
    char *found = strstr(sentence, "needle");
    if (found) {
        printf("found \"%s\" at index %td\n", found, found - sentence);
    }

    /* --- Pointer to pointer --- */
    printf("\n=== Pointer to pointer ===\n");

    int  val = 7;
    int  *ptr  = &val;
    int **pptr = &ptr;   /* pointer to a pointer to int */

    printf("val    = %d\n",  val);
    printf("*ptr   = %d\n",  *ptr);
    printf("**pptr = %d\n",  **pptr);

    /* --- NULL pointer --- */
    printf("\n=== NULL pointer ===\n");

    int *null_ptr = NULL;   /* explicitly points nowhere */
    if (null_ptr == NULL) {
        printf("null_ptr is NULL — safe to check before dereferencing\n");
    }
    /* Dereferencing NULL is undefined behavior (usually a segfault).
       Always check a pointer before using it if it could be NULL. */

    /* --- Passing pointers to functions --- */
    printf("\n=== Passing pointers to functions ===\n");

    int n = 5;
    printf("before double_it: n = %d\n", n);
    double_it(&n);
    printf("after  double_it: n = %d\n", n);

    char s[] = "abcde";
    printf("before reverse: %s\n", s);
    reverse(s, strlen(s));
    printf("after  reverse: %s\n", s);

    return 0;
}

/* Modifies the caller's variable through a pointer */
int double_it(int *n) {
    *n = *n * 2;
    return *n;
}

/* Reverses a string in place using two pointers walking toward each other */
void reverse(char *s, int len) {
    char *left  = s;
    char *right = s + len - 1;
    while (left < right) {
        char tmp = *left;
        *left    = *right;
        *right   = tmp;
        left++;
        right--;
    }
}
