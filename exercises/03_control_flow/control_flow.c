#include <stdio.h>

int main(void) {

    /* --- if / else if / else --- */
    printf("=== if / else ===\n");
    int temp = 72;
    if (temp >= 90) {
        printf("Hot\n");
    } else if (temp >= 70) {
        printf("Comfortable\n");
    } else if (temp >= 50) {
        printf("Cool\n");
    } else {
        printf("Cold\n");
    }

    /* Multiple conditions with && (and) and || (or) */
    int humidity = 85;
    if (temp >= 70 && humidity >= 80) {
        printf("Warm and humid\n");
    }

    /* --- switch --- */
    printf("\n=== switch ===\n");
    int day = 3;
    switch (day) {
        case 1: printf("Monday\n");    break;
        case 2: printf("Tuesday\n");   break;
        case 3: printf("Wednesday\n"); break;
        case 4: printf("Thursday\n");  break;
        case 5: printf("Friday\n");    break;
        /* Intentional fall-through: Saturday and Sunday share the same label */
        case 6:
        case 7: printf("Weekend\n");   break;
        default: printf("Invalid day\n");
    }

    /* switch works on integers and chars */
    char grade = 'B';
    switch (grade) {
        case 'A': printf("Excellent\n"); break;
        case 'B': printf("Good\n");      break;
        case 'C': printf("Average\n");   break;
        default:  printf("Below average\n");
    }

    /* --- for loop --- */
    printf("\n=== for loop ===\n");
    for (int i = 0; i < 5; i++) {
        printf("%d ", i);
    }
    printf("\n");

    /* Counting down */
    for (int i = 5; i > 0; i--) {
        printf("%d ", i);
    }
    printf("\n");

    /* Nested loops: multiplication table */
    printf("\nMultiplication table (1-4):\n");
    for (int row = 1; row <= 4; row++) {
        for (int col = 1; col <= 4; col++) {
            printf("%3d", row * col);
        }
        printf("\n");
    }

    /* --- while loop --- */
    printf("\n=== while loop ===\n");
    int n = 1;
    while (n <= 16) {
        printf("%d ", n);
        n *= 2;
    }
    printf("\n");

    /* break exits the loop early; continue skips to the next iteration */
    printf("\nSkip multiples of 3, stop at 15:\n");
    for (int i = 1; i <= 20; i++) {
        if (i == 15) break;
        if (i % 3 == 0) continue;
        printf("%d ", i);
    }
    printf("\n");

    /* --- do-while loop --- */
    /* Body always executes at least once, even if the condition is false initially */
    printf("\n=== do-while ===\n");
    int x = 100;
    do {
        printf("x = %d\n", x);
        x++;
    } while (x < 3);   /* condition false from the start, but body runs once */

    return 0;
}
