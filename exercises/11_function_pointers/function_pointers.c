#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define M_PI 3.14159265358979323846

/* A function pointer type declaration reads right-to-left:
     'operation' is a pointer to a function that takes two ints and returns an int */
typedef int (*BinaryOp)(int, int);

/* Callback type: a function that receives one int and returns void */
typedef void (*IntCallback)(int);

/* Comparator type matching the signature qsort expects */
typedef int (*Comparator)(const void *, const void *);

/* ------------------------------------------------------------------ */
/* Functions we'll point at */

int add(int a, int b)      { return a + b; }
int subtract(int a, int b) { return a - b; }
int multiply(int a, int b) { return a * b; }

void print_value(int x)    { printf("  %d\n", x); }
void print_doubled(int x)  { printf("  %d\n", x * 2); }
void print_square(int x)   { printf("  %d\n", x * x); }

double apply_to_pi(double (*fn)(double)) { return fn(M_PI); }

/* ------------------------------------------------------------------ */

/* Takes a function pointer as a parameter — applies op to every element */
void map(int *arr, int len, IntCallback fn) {
    for (int i = 0; i < len; i++) fn(arr[i]);
}

/* Reduces an array to a single value using op */
int reduce(const int *arr, int len, int initial, BinaryOp op) {
    int acc = initial;
    for (int i = 0; i < len; i++) acc = op(acc, arr[i]);
    return acc;
}

/* Dispatch table: array of function pointers indexed by a key */
int dispatch(int a, int b, char op) {
    BinaryOp table[] = {
        ['+'] = add,
        ['-'] = subtract,
        ['*'] = multiply,
    };
    if (op < 0 || (size_t)op >= sizeof(table)/sizeof(table[0]) || table[(int)op] == NULL) {
        fprintf(stderr, "unknown op '%c'\n", op);
        return 0;
    }
    return table[(int)op](a, b);
}

/* Returns a function pointer — lets the caller choose behavior at runtime */
BinaryOp get_operation(char op) {
    switch (op) {
        case '+': return add;
        case '-': return subtract;
        case '*': return multiply;
        default:  return NULL;
    }
}

/* Comparators for qsort */
int cmp_int_asc (const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}
int cmp_int_desc(const void *a, const void *b) {
    return (*(int *)b - *(int *)a);
}
int cmp_str     (const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- Basic syntax: declare, assign, call --- */
    printf("=== Basic function pointer ===\n");

    int (*op)(int, int);    /* declare a function pointer */
    op = add;               /* assign — no & needed, function name decays to pointer */
    printf("op(3, 4)  = %d\n", op(3, 4));   /* call through the pointer */
    printf("(*op)(3,4)= %d\n", (*op)(3, 4)); /* explicit dereference — identical */

    op = multiply;
    printf("op(3, 4)  = %d  (now multiply)\n", op(3, 4));

    /* typedef makes the syntax much cleaner */
    BinaryOp fn = subtract;
    printf("fn(10, 3) = %d\n", fn(10, 3));

    /* --- Passing a function pointer to a function (callback) --- */
    printf("\n=== map with callbacks ===\n");
    int arr[] = {1, 2, 3, 4, 5};
    int len   = sizeof(arr) / sizeof(arr[0]);

    printf("print_value:\n");
    map(arr, len, print_value);

    printf("print_doubled:\n");
    map(arr, len, print_doubled);

    printf("print_square:\n");
    map(arr, len, print_square);

    /* --- Reduce --- */
    printf("\n=== reduce ===\n");
    int sum     = reduce(arr, len, 0, add);
    int product = reduce(arr, len, 1, multiply);
    printf("sum     = %d\n", sum);
    printf("product = %d\n", product);

    /* --- Dispatch table --- */
    printf("\n=== dispatch table ===\n");
    printf("dispatch(10, 3, '+') = %d\n", dispatch(10, 3, '+'));
    printf("dispatch(10, 3, '-') = %d\n", dispatch(10, 3, '-'));
    printf("dispatch(10, 3, '*') = %d\n", dispatch(10, 3, '*'));

    /* --- Returning a function pointer --- */
    printf("\n=== returning a function pointer ===\n");
    char ops[] = {'+', '-', '*'};
    for (int i = 0; i < 3; i++) {
        BinaryOp f = get_operation(ops[i]);
        if (f) printf("get_operation('%c')(6, 2) = %d\n", ops[i], f(6, 2));
    }

    /* --- Passing to a standard library function --- */
    printf("\n=== apply to math functions ===\n");
    printf("sin(pi)  = %.4f\n", apply_to_pi(sin));
    printf("cos(pi)  = %.4f\n", apply_to_pi(cos));
    printf("sqrt(pi) = %.4f\n", apply_to_pi(sqrt));

    /* --- qsort: the classic stdlib callback --- */
    printf("\n=== qsort ===\n");
    int nums[] = {42, 7, 19, 3, 55, 1, 28};
    int nlen   = sizeof(nums) / sizeof(nums[0]);

    qsort(nums, nlen, sizeof(int), cmp_int_asc);
    printf("ascending:  ");
    for (int i = 0; i < nlen; i++) printf("%d ", nums[i]);
    printf("\n");

    qsort(nums, nlen, sizeof(int), cmp_int_desc);
    printf("descending: ");
    for (int i = 0; i < nlen; i++) printf("%d ", nums[i]);
    printf("\n");

    const char *words[] = {"banana", "apple", "cherry", "date"};
    int wlen = sizeof(words) / sizeof(words[0]);
    qsort(words, wlen, sizeof(char *), cmp_str);
    printf("sorted strings: ");
    for (int i = 0; i < wlen; i++) printf("%s ", words[i]);
    printf("\n");

    /* --- Array of function pointers --- */
    printf("\n=== array of function pointers ===\n");
    BinaryOp ops_arr[] = {add, subtract, multiply};
    const char *names[] = {"add", "subtract", "multiply"};
    for (int i = 0; i < 3; i++) {
        printf("%-10s(8, 3) = %d\n", names[i], ops_arr[i](8, 3));
    }

    return 0;
}
