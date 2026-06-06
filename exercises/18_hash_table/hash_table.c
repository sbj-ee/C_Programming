#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ================================================================
   HASH TABLE — separate chaining

   An array of buckets, each a linked list of key/value pairs.
   Average O(1) insert, lookup, and delete when the load factor
   (entries / buckets) stays low.

   Load factor = entries / capacity
   Rehash when load factor exceeds LOAD_MAX: allocate a new bucket
   array twice the size and re-insert every entry.
   ================================================================ */

#define INITIAL_CAPACITY  8
#define LOAD_MAX          0.75

typedef struct Entry {
    char        *key;
    int          value;
    struct Entry *next;   /* chaining within a bucket */
} Entry;

typedef struct {
    Entry  **buckets;
    int      capacity;
    int      count;
} HashMap;

/* API */
HashMap *hmap_create(void);
void     hmap_free(HashMap *map);
void     hmap_put(HashMap *map, const char *key, int value);
int      hmap_get(HashMap *map, const char *key, int *out);
int      hmap_delete(HashMap *map, const char *key);
int      hmap_contains(HashMap *map, const char *key);
void     hmap_print(const HashMap *map);

/* ------------------------------------------------------------------ */

/* djb2 hash — simple, fast, good distribution for string keys */
static unsigned long hash_djb2(const char *key) {
    unsigned long h = 5381;
    int c;
    while ((c = (unsigned char)*key++)) h = h * 33 ^ c;
    return h;
}

static int bucket_index(const char *key, int capacity) {
    return (int)(hash_djb2(key) % (unsigned long)capacity);
}

static char *dup_str(const char *s) {
    size_t len = strlen(s) + 1;
    char  *copy = malloc(len);
    if (!copy) { fprintf(stderr, "malloc failed\n"); exit(1); }
    memcpy(copy, s, len);
    return copy;
}

static Entry *entry_create(const char *key, int value) {
    Entry *e = malloc(sizeof(Entry));
    if (!e) { fprintf(stderr, "malloc failed\n"); exit(1); }
    e->key   = dup_str(key);   /* own a heap copy of the key */
    e->value = value;
    e->next  = NULL;
    return e;
}

/* Rehash: allocate a new bucket array and re-insert all entries. */
static void rehash(HashMap *map) {
    int       old_cap  = map->capacity;
    Entry   **old_bkts = map->buckets;

    map->capacity = old_cap * 2;
    map->buckets  = calloc(map->capacity, sizeof(Entry *));
    if (!map->buckets) { fprintf(stderr, "calloc failed\n"); exit(1); }
    map->count = 0;

    for (int i = 0; i < old_cap; i++) {
        Entry *cur = old_bkts[i];
        while (cur) {
            Entry *next = cur->next;
            /* Re-insert into the new bucket array */
            int idx    = bucket_index(cur->key, map->capacity);
            cur->next  = map->buckets[idx];
            map->buckets[idx] = cur;
            map->count++;
            cur = next;
        }
    }
    free(old_bkts);
}

/* ================================================================
   PUBLIC API
   ================================================================ */

HashMap *hmap_create(void) {
    HashMap *map  = malloc(sizeof(HashMap));
    if (!map) { fprintf(stderr, "malloc failed\n"); exit(1); }
    map->capacity = INITIAL_CAPACITY;
    map->count    = 0;
    map->buckets  = calloc(map->capacity, sizeof(Entry *));
    if (!map->buckets) { fprintf(stderr, "calloc failed\n"); exit(1); }
    return map;
}

void hmap_free(HashMap *map) {
    for (int i = 0; i < map->capacity; i++) {
        Entry *cur = map->buckets[i];
        while (cur) {
            Entry *next = cur->next;
            free(cur->key);
            free(cur);
            cur = next;
        }
    }
    free(map->buckets);
    free(map);
}

/* Insert or update.  Rehashes if load factor exceeds LOAD_MAX. */
void hmap_put(HashMap *map, const char *key, int value) {
    /* Update existing key if found */
    int    idx = bucket_index(key, map->capacity);
    Entry *cur = map->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) { cur->value = value; return; }
        cur = cur->next;
    }

    /* Rehash before inserting if needed */
    if ((double)(map->count + 1) / map->capacity > LOAD_MAX) {
        rehash(map);
        idx = bucket_index(key, map->capacity);   /* recompute after rehash */
    }

    /* Prepend new entry to the bucket chain */
    Entry *e        = entry_create(key, value);
    e->next         = map->buckets[idx];
    map->buckets[idx] = e;
    map->count++;
}

/* Returns 1 and writes *out on hit; returns 0 on miss. */
int hmap_get(HashMap *map, const char *key, int *out) {
    int    idx = bucket_index(key, map->capacity);
    Entry *cur = map->buckets[idx];
    while (cur) {
        if (strcmp(cur->key, key) == 0) { *out = cur->value; return 1; }
        cur = cur->next;
    }
    return 0;
}

/* Returns 1 if deleted, 0 if not found. */
int hmap_delete(HashMap *map, const char *key) {
    int    idx  = bucket_index(key, map->capacity);
    Entry *cur  = map->buckets[idx];
    Entry *prev = NULL;

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next        = cur->next;
            else      map->buckets[idx] = cur->next;
            free(cur->key);
            free(cur);
            map->count--;
            return 1;
        }
        prev = cur;
        cur  = cur->next;
    }
    return 0;
}

int hmap_contains(HashMap *map, const char *key) {
    int ignored;
    return hmap_get(map, key, &ignored);
}

void hmap_print(const HashMap *map) {
    printf("HashMap { count=%d  capacity=%d  load=%.2f }\n",
           map->count, map->capacity,
           (double)map->count / map->capacity);
    for (int i = 0; i < map->capacity; i++) {
        if (!map->buckets[i]) continue;
        printf("  [%2d] ", i);
        for (Entry *e = map->buckets[i]; e; e = e->next) {
            printf("\"%s\":%d", e->key, e->value);
            if (e->next) printf(" → ");
        }
        printf("\n");
    }
}

/* ================================================================
   MAIN
   ================================================================ */
int main(void) {

    /* --- Basic put / get / contains --- */
    printf("=== put / get / contains ===\n");
    HashMap *map = hmap_create();

    hmap_put(map, "alpha",   1);
    hmap_put(map, "beta",    2);
    hmap_put(map, "gamma",   3);
    hmap_put(map, "delta",   4);
    hmap_put(map, "epsilon", 5);
    hmap_print(map);

    int val;
    const char *keys[] = {"alpha", "gamma", "epsilon", "zeta"};
    for (int i = 0; i < 4; i++) {
        if (hmap_get(map, keys[i], &val))
            printf("get(\"%s\") = %d\n", keys[i], val);
        else
            printf("get(\"%s\") = (not found)\n", keys[i]);
    }

    /* --- Update existing key --- */
    printf("\n=== update ===\n");
    hmap_put(map, "beta", 99);
    hmap_get(map, "beta", &val);
    printf("after update, get(\"beta\") = %d\n", val);

    /* --- Delete --- */
    printf("\n=== delete ===\n");
    printf("delete(\"gamma\")   = %d\n", hmap_delete(map, "gamma"));
    printf("delete(\"missing\") = %d\n", hmap_delete(map, "missing"));
    printf("contains(\"gamma\") = %d\n", hmap_contains(map, "gamma"));
    hmap_print(map);

    /* --- Rehashing under load --- */
    printf("=== rehash under load ===\n");
    HashMap *big = hmap_create();
    char key[16];
    for (int i = 0; i < 40; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        hmap_put(big, key, i * i);
        if (i == 5 || i == 11 || i == 23 || i == 39) {
            printf("after %2d inserts: capacity=%-4d  load=%.2f\n",
                   i + 1, big->capacity,
                   (double)big->count / big->capacity);
        }
    }

    /* Verify all values survived the rehash(es) */
    int errors = 0;
    for (int i = 0; i < 40; i++) {
        snprintf(key, sizeof(key), "key%d", i);
        if (!hmap_get(big, key, &val) || val != i * i) errors++;
    }
    printf("post-rehash verification: %s\n", errors ? "FAILED" : "all 40 entries correct");
    hmap_free(big);

    /* --- Word-frequency counter --- */
    printf("\n=== word frequency counter ===\n");
    const char *words[] = {
        "the", "quick", "brown", "fox", "jumps", "over",
        "the", "lazy", "dog", "the", "fox", "the"
    };
    int nwords = sizeof(words) / sizeof(words[0]);

    HashMap *freq = hmap_create();
    for (int i = 0; i < nwords; i++) {
        int count = 0;
        hmap_get(freq, words[i], &count);
        hmap_put(freq, words[i], count + 1);
    }
    hmap_print(freq);
    hmap_free(freq);

    hmap_free(map);
    return 0;
}
