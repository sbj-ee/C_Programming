#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ================================================================
   SORTING ALGORITHMS

   Algorithm       Best      Average   Worst     Space   Stable
   ─────────────────────────────────────────────────────────────
   Bubble          O(n)      O(n²)     O(n²)     O(1)    yes
   Insertion       O(n)      O(n²)     O(n²)     O(1)    yes
   Selection       O(n²)     O(n²)     O(n²)     O(1)    no
   Merge           O(n log n) O(n log n) O(n log n) O(n) yes
   Quick           O(n log n) O(n log n) O(n²)   O(log n) no
   ================================================================ */

/* Helpers */
static void swap_int(int *a, int *b) { int t = *a; *a = *b; *b = t; }

static void print_arr(const int *arr, int n) {
    printf("[");
    for (int i = 0; i < n; i++) printf(i < n-1 ? "%d, " : "%d", arr[i]);
    printf("]\n");
}

static void copy_arr(const int *src, int *dst, int n) {
    memcpy(dst, src, n * sizeof(int));
}

/* ================================================================
   BUBBLE SORT
   Repeatedly swap adjacent elements that are out of order.
   Early-exit flag makes it O(n) on an already-sorted array.
   ================================================================ */
void bubble_sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                swap_int(&arr[j], &arr[j + 1]);
                swapped = 1;
            }
        }
        if (!swapped) break;   /* already sorted */
    }
}

/* ================================================================
   INSERTION SORT
   Build the sorted portion one element at a time by shifting
   larger elements right to make room for the current element.
   Excellent on small or nearly-sorted arrays.
   ================================================================ */
void insertion_sort(int *arr, int n) {
    for (int i = 1; i < n; i++) {
        int key = arr[i];
        int j   = i - 1;
        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];   /* shift right */
            j--;
        }
        arr[j + 1] = key;          /* insert in correct position */
    }
}

/* ================================================================
   SELECTION SORT
   Find the minimum of the unsorted portion and swap it to the front.
   Always O(n²) — no early exit possible.
   ================================================================ */
void selection_sort(int *arr, int n) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        if (min_idx != i) swap_int(&arr[i], &arr[min_idx]);
    }
}

/* ================================================================
   MERGE SORT
   Divide the array in half, sort each half recursively, then
   merge the two sorted halves.  O(n log n) guaranteed.
   Requires O(n) extra memory for the temporary buffer.
   ================================================================ */
static void merge(int *arr, int lo, int mid, int hi, int *tmp) {
    /* Copy the two halves into tmp */
    for (int k = lo; k <= hi; k++) tmp[k] = arr[k];

    int i = lo, j = mid + 1, k = lo;
    while (i <= mid && j <= hi) {
        if (tmp[i] <= tmp[j]) arr[k++] = tmp[i++];   /* <= keeps it stable */
        else                  arr[k++] = tmp[j++];
    }
    while (i <= mid) arr[k++] = tmp[i++];
    while (j <= hi)  arr[k++] = tmp[j++];
}

static void merge_sort_rec(int *arr, int lo, int hi, int *tmp) {
    if (lo >= hi) return;
    int mid = lo + (hi - lo) / 2;          /* avoids overflow vs (lo+hi)/2 */
    merge_sort_rec(arr, lo,      mid, tmp);
    merge_sort_rec(arr, mid + 1, hi,  tmp);
    merge(arr, lo, mid, hi, tmp);
}

void merge_sort(int *arr, int n) {
    int *tmp = malloc(n * sizeof(int));
    if (!tmp) { fprintf(stderr, "malloc failed\n"); exit(1); }
    merge_sort_rec(arr, 0, n - 1, tmp);
    free(tmp);
}

/* ================================================================
   QUICKSORT
   Partition around a pivot so everything left < pivot < everything
   right, then recurse on each side.  O(n log n) average, O(n²)
   worst case (mitigated here by median-of-three pivot selection).
   ================================================================ */
static int median_of_three(int *arr, int lo, int hi) {
    int mid = lo + (hi - lo) / 2;
    /* Sort lo, mid, hi in place; return the median index */
    if (arr[lo]  > arr[mid]) swap_int(&arr[lo],  &arr[mid]);
    if (arr[lo]  > arr[hi])  swap_int(&arr[lo],  &arr[hi]);
    if (arr[mid] > arr[hi])  swap_int(&arr[mid], &arr[hi]);
    /* arr[lo] ≤ arr[mid] ≤ arr[hi]; place pivot at hi-1 */
    swap_int(&arr[mid], &arr[hi - 1]);
    return hi - 1;
}

static int partition(int *arr, int lo, int hi) {
    if (hi - lo < 2) {   /* 2 or fewer elements: just sort them */
        if (arr[lo] > arr[hi]) swap_int(&arr[lo], &arr[hi]);
        return lo;
    }
    int pivot_idx = median_of_three(arr, lo, hi);
    int pivot     = arr[pivot_idx];
    int i = lo, j = hi - 1;

    while (1) {
        while (arr[++i] < pivot) {}
        while (arr[--j] > pivot) {}
        if (i >= j) break;
        swap_int(&arr[i], &arr[j]);
    }
    swap_int(&arr[i], &arr[pivot_idx]);   /* restore pivot */
    return i;
}

static void quick_sort_rec(int *arr, int lo, int hi) {
    if (lo >= hi) return;
    int p = partition(arr, lo, hi);
    quick_sort_rec(arr, lo,     p - 1);
    quick_sort_rec(arr, p + 1,  hi);
}

void quick_sort(int *arr, int n) {
    if (n < 2) return;
    quick_sort_rec(arr, 0, n - 1);
}

/* ================================================================
   STDLIB QSORT — generic via function pointer comparator
   ================================================================ */
static int cmp_asc (const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    return (ia > ib) - (ia < ib);   /* subtraction overflows for large mixed-sign values */
}
static int cmp_desc(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    return (ib > ia) - (ib < ia);
}

typedef struct { char name[16]; int score; } Player;
static int cmp_player_score(const void *a, const void *b) {
    return ((Player*)b)->score - ((Player*)a)->score;   /* descending */
}
static int cmp_player_name(const void *a, const void *b) {
    return strcmp(((Player*)a)->name, ((Player*)b)->name);
}

/* ================================================================
   MAIN
   ================================================================ */
#define N 12

int main(void) {

    int original[] = {64, 25, 12, 22, 11, 90, 37, 55, 8, 43, 76, 3};
    int arr[N];

    /* --- Demonstrate each sort --- */
    typedef struct { const char *name; void (*fn)(int *, int); } SortEntry;
    SortEntry sorts[] = {
        {"bubble_sort",    bubble_sort},
        {"insertion_sort", insertion_sort},
        {"selection_sort", selection_sort},
        {"merge_sort",     merge_sort},
        {"quick_sort",     quick_sort},
    };
    int nsorts = sizeof(sorts) / sizeof(sorts[0]);

    printf("original: "); print_arr(original, N);
    printf("\n");

    for (int i = 0; i < nsorts; i++) {
        copy_arr(original, arr, N);
        sorts[i].fn(arr, N);
        printf("%-16s ", sorts[i].name); print_arr(arr, N);
    }

    /* --- stdlib qsort --- */
    printf("\n=== stdlib qsort ===\n");
    copy_arr(original, arr, N);
    qsort(arr, N, sizeof(int), cmp_asc);
    printf("ascending:  "); print_arr(arr, N);
    qsort(arr, N, sizeof(int), cmp_desc);
    printf("descending: "); print_arr(arr, N);

    Player players[] = {
        {"Alice", 9200}, {"Bob", 8750}, {"Carol", 9500},
        {"Dave",  8100}, {"Eve", 9500}
    };
    int np = sizeof(players) / sizeof(players[0]);

    qsort(players, np, sizeof(Player), cmp_player_score);
    printf("\nby score (desc):\n");
    for (int i = 0; i < np; i++)
        printf("  %-8s %d\n", players[i].name, players[i].score);

    qsort(players, np, sizeof(Player), cmp_player_name);
    printf("\nby name (asc):\n");
    for (int i = 0; i < np; i++)
        printf("  %-8s %d\n", players[i].name, players[i].score);

    /* --- Performance comparison on a large random array --- */
    printf("\n=== Performance (n=20000) ===\n");
    int big_n = 20000;
    int *src  = malloc(big_n * sizeof(int));
    int *buf  = malloc(big_n * sizeof(int));
    if (!src || !buf) { fprintf(stderr, "malloc failed\n"); return 1; }

    srand(42);
    for (int i = 0; i < big_n; i++) src[i] = rand();

    for (int i = 0; i < nsorts; i++) {
        copy_arr(src, buf, big_n);
        clock_t start = clock();
        sorts[i].fn(buf, big_n);
        clock_t end   = clock();
        printf("%-16s  %.3f ms\n", sorts[i].name,
               1000.0 * (end - start) / CLOCKS_PER_SEC);
    }

    free(src);
    free(buf);
    return 0;
}
