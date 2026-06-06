/* stats.h — basic descriptive statistics */
#ifndef STATS_H
#define STATS_H

/* Declaring a variable extern means "it exists somewhere else."
   The definition (storage allocation) lives in stats.c. */
extern int stats_calls;   /* counts how many times any stats_ function is called */

double stats_mean(const double *arr, int n);
double stats_variance(const double *arr, int n);
double stats_stddev(const double *arr, int n);
double stats_min(const double *arr, int n);
double stats_max(const double *arr, int n);

#endif /* STATS_H */
