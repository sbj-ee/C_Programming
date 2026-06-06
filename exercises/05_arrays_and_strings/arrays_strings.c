#include <stdio.h>
#include <string.h>

int main(void) {

    /* --- Fixed arrays --- */
    printf("=== Fixed arrays ===\n");

    int nums[5] = {10, 20, 30, 40, 50};
    int len = sizeof(nums) / sizeof(nums[0]);

    for (int i = 0; i < len; i++) {
        printf("nums[%d] = %d\n", i, nums[i]);
    }

    /* Partial initialization: unspecified elements are zeroed */
    int partial[5] = {1, 2};
    printf("\npartial[]: ");
    for (int i = 0; i < 5; i++) printf("%d ", partial[i]);
    printf("\n");

    /* 2D array */
    printf("\n=== 2D array ===\n");
    int grid[3][4] = {
        {1,  2,  3,  4},
        {5,  6,  7,  8},
        {9, 10, 11, 12}
    };
    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 4; c++) {
            printf("%3d", grid[r][c]);
        }
        printf("\n");
    }

    /* --- Strings ---
       A C string is a char array terminated by a null byte '\0'.
       The null terminator is what makes it a string, not just bytes. */
    printf("\n=== Strings ===\n");

    char greeting[20] = "Hello";   /* 5 chars + '\0', rest zeroed */
    printf("greeting      = \"%s\"\n", greeting);
    printf("strlen        = %zu\n",   strlen(greeting));   /* excludes '\0' */
    printf("sizeof        = %zu\n",   sizeof(greeting));   /* includes '\0' and padding */

    /* String literal vs char array */
    char word[] = "World";         /* compiler sizes the array: 6 bytes */
    printf("word          = \"%s\"  (sizeof=%zu)\n", word, sizeof(word));

    /* --- Common string functions --- */
    printf("\n=== String functions ===\n");

    /* strcpy: copy src into dst — dst must be large enough */
    char dst[20];
    strcpy(dst, "Hello");
    printf("strcpy        = \"%s\"\n", dst);

    /* strcat: append src onto the end of dst */
    strcat(dst, ", World");
    printf("strcat        = \"%s\"\n", dst);

    /* strcmp: 0 if equal, <0 if s1 < s2, >0 if s1 > s2 */
    printf("strcmp equal  = %d\n", strcmp("abc", "abc"));
    printf("strcmp less   = %d\n", strcmp("abc", "abd"));
    printf("strcmp great  = %d\n", strcmp("abd", "abc"));

    /* strncpy / strncat: safer bounded versions */
    char safe[10];
    strncpy(safe, "TooLongString", sizeof(safe) - 1);
    safe[sizeof(safe) - 1] = '\0';   /* strncpy does not guarantee termination */
    printf("strncpy safe  = \"%s\"\n", safe);

    /* strchr: find first occurrence of a character */
    char sentence[] = "find the dot.";
    char *dot = strchr(sentence, '.');
    if (dot) printf("strchr '.'    = index %td\n", dot - sentence);

    /* strstr: find first occurrence of a substring */
    char *sub = strstr(sentence, "the");
    if (sub) printf("strstr \"the\"  = index %td\n", sub - sentence);

    /* --- Iterating over a string --- */
    printf("\n=== Iterating characters ===\n");
    char msg[] = "Hello";
    /* The null terminator '\0' is falsy, so this loop stops naturally */
    for (int i = 0; msg[i] != '\0'; i++) {
        printf("msg[%d] = '%c'  (%d)\n", i, msg[i], msg[i]);
    }

    /* --- Modifying characters in place --- */
    printf("\n=== Modifying in place ===\n");
    char name[] = "alice";
    name[0] = 'A';   /* capitalize first letter */
    printf("capitalized   = \"%s\"\n", name);

    return 0;
}
