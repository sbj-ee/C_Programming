#include "stats.h"
#include <math.h>

/* Definition: this is where the storage for stats_calls actually lives.
   Every other file that includes stats.h sees it as extern (no storage). */
int stats_calls = 0;

double stats_mean(const double *arr, int n) {
    stats_calls++;
    double sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    return sum / n;
}

double stats_variance(const double *arr, int n) {
    stats_calls++;
    double m = stats_mean(arr, n);
    stats_calls--;   /* don't double-count the mean call */
    double sum = 0;
    for (int i = 0; i < n; i++) sum += (arr[i] - m) * (arr[i] - m);
    return sum / n;
}

double stats_stddev(const double *arr, int n) {
    stats_calls++;
    return sqrt(stats_variance(arr, n));
}

double stats_min(const double *arr, int n) {
    stats_calls++;
    double m = arr[0];
    for (int i = 1; i < n; i++) if (arr[i] < m) m = arr[i];
    return m;
}

double stats_max(const double *arr, int n) {
    stats_calls++;
    double m = arr[0];
    for (int i = 1; i < n; i++) if (arr[i] > m) m = arr[i];
    return m;
}
