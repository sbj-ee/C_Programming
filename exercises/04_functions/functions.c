#include <stdio.h>

/* --- Forward declarations (prototypes) ---
   The compiler reads top-to-bottom. Declaring a function's signature here
   lets you call it before its definition appears in the file. */
int    add(int a, int b);
double average(int arr[], int len);
int    factorial(int n);
int    fibonacci(int n);
void   swap(int *a, int *b);
int    max3(int a, int b, int c);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- Basic call and return value --- */
    printf("=== Basic functions ===\n");
    printf("add(3, 4)     = %d\n", add(3, 4));
    printf("max3(7, 2, 9) = %d\n", max3(7, 2, 9));

    /* --- Passing an array --- */
    printf("\n=== Passing arrays ===\n");
    int scores[] = {88, 72, 95, 60, 83};
    int len = sizeof(scores) / sizeof(scores[0]);   /* element count */
    printf("average       = %.1f\n", average(scores, len));

    /* --- Scope ---
       Variables declared inside a function are local to that function.
       'result' here is a separate variable from any 'result' inside add(). */
    printf("\n=== Scope ===\n");
    int result = 100;
    printf("result before call: %d\n", result);
    add(3, 4);                  /* the 'result' inside add() doesn't affect ours */
    printf("result after call:  %d\n", result);

    /* --- Passing by value vs passing by pointer ---
       C passes arguments by value: the function gets a copy.
       To let a function modify the caller's variable, pass its address. */
    printf("\n=== Pass by value vs pointer ===\n");
    int x = 5, y = 10;
    printf("before swap: x=%d y=%d\n", x, y);
    swap(&x, &y);
    printf("after swap:  x=%d y=%d\n", x, y);

    /* --- Recursion --- */
    printf("\n=== Recursion: factorial ===\n");
    for (int i = 0; i <= 7; i++) {
        printf("%d! = %d\n", i, factorial(i));
    }

    printf("\n=== Recursion: Fibonacci ===\n");
    for (int i = 0; i < 10; i++) {
        printf("fib(%d) = %d\n", i, fibonacci(i));
    }

    return 0;
}

/* ------------------------------------------------------------------ */

int add(int a, int b) {
    int result = a + b;   /* local to add(); vanishes when the function returns */
    return result;
}

/* max3 uses nested calls to the standard library's own pattern:
   pick the larger of two, then compare with the third. */
int max3(int a, int b, int c) {
    int m = (a > b) ? a : b;
    return (m > c) ? m : c;
}

/* Arrays decay to a pointer when passed to a function, so 'len' must be
   passed separately — the function cannot compute it from 'arr' alone. */
double average(int arr[], int len) {
    int sum = 0;
    for (int i = 0; i < len; i++) {
        sum += arr[i];
    }
    return (double)sum / len;
}

/* swap receives addresses, not values, so it can modify the caller's variables */
void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

/* Recursive factorial: n! = n * (n-1)!   Base case: 0! = 1 */
int factorial(int n) {
    if (n == 0) return 1;
    return n * factorial(n - 1);
}

/* Recursive Fibonacci: fib(n) = fib(n-1) + fib(n-2)   Base cases: fib(0)=0, fib(1)=1
   Simple but exponentially slow — each call branches into two more. */
int fibonacci(int n) {
    if (n == 0) return 0;
    if (n == 1) return 1;
    return fibonacci(n - 1) + fibonacci(n - 2);
}
