#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEXT_FILE   "output.txt"
#define CSV_FILE    "scores.csv"
#define BIN_FILE    "data.bin"

typedef struct {
    char  name[32];
    int   score;
    float ratio;
} Record;

void write_text(void);
void read_text(void);
void write_csv(void);
void read_csv(void);
void write_binary(void);
void read_binary(void);
void append_text(void);
long file_size(const char *path);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- fprintf / fscanf: formatted text I/O --- */
    printf("=== Text file (fprintf / fgets) ===\n");
    write_text();
    read_text();

    /* --- CSV: structured text with multiple fields per line --- */
    printf("\n=== CSV file ===\n");
    write_csv();
    read_csv();

    /* --- fwrite / fread: raw binary I/O --- */
    printf("\n=== Binary file (fwrite / fread) ===\n");
    write_binary();
    read_binary();

    /* --- Appending to an existing file --- */
    printf("\n=== Append mode ===\n");
    append_text();
    read_text();

    /* --- File size --- */
    printf("\n=== File sizes ===\n");
    printf("%-15s %ld bytes\n", TEXT_FILE, file_size(TEXT_FILE));
    printf("%-15s %ld bytes\n", CSV_FILE,  file_size(CSV_FILE));
    printf("%-15s %ld bytes\n", BIN_FILE,  file_size(BIN_FILE));

    /* Clean up */
    remove(TEXT_FILE);
    remove(CSV_FILE);
    remove(BIN_FILE);

    return 0;
}

/* ------------------------------------------------------------------ */

void write_text(void) {
    /* fopen modes:
         "r"  read (file must exist)
         "w"  write (creates or truncates)
         "a"  append (creates or appends)
         "rb" / "wb" — binary equivalents */
    FILE *f = fopen(TEXT_FILE, "w");
    if (f == NULL) {
        perror("fopen");   /* prints the OS error message */
        exit(1);
    }

    fprintf(f, "Line 1: the quick brown fox\n");
    fprintf(f, "Line 2: jumped over %d lazy dogs\n", 3);
    fprintf(f, "Line 3: pi is approximately %.4f\n", 3.14159);

    fclose(f);
    printf("wrote %s\n", TEXT_FILE);
}

void read_text(void) {
    FILE *f = fopen(TEXT_FILE, "r");
    if (f == NULL) { perror("fopen"); exit(1); }

    char line[128];
    /* fgets reads one line at a time (up to size-1 chars), preserving '\n' */
    while (fgets(line, sizeof(line), f) != NULL) {
        /* strip the trailing newline for clean printing */
        line[strcspn(line, "\n")] = '\0';
        printf("  > %s\n", line);
    }

    /* feof / ferror distinguish end-of-file from a read error */
    if (ferror(f)) perror("read error");

    fclose(f);
}

void write_csv(void) {
    FILE *f = fopen(CSV_FILE, "w");
    if (f == NULL) { perror("fopen"); exit(1); }

    fprintf(f, "name,score,ratio\n");   /* header row */

    const char *names[] = {"Alice", "Bob", "Carol"};
    int    scores[] = {95, 82, 78};
    float  ratios[] = {0.95f, 0.82f, 0.78f};

    for (int i = 0; i < 3; i++) {
        fprintf(f, "%s,%d,%.2f\n", names[i], scores[i], ratios[i]);
    }

    fclose(f);
    printf("wrote %s\n", CSV_FILE);
}

void read_csv(void) {
    FILE *f = fopen(CSV_FILE, "r");
    if (f == NULL) { perror("fopen"); exit(1); }

    char line[128];
    fgets(line, sizeof(line), f);   /* skip header */

    char  name[32];
    int   score;
    float ratio;

    /* sscanf parses a string the same way scanf parses stdin */
    while (fgets(line, sizeof(line), f) != NULL) {
        if (sscanf(line, "%31[^,],%d,%f", name, &score, &ratio) == 3) {
            printf("  %-8s score=%-4d ratio=%.2f\n", name, score, ratio);
        }
    }

    fclose(f);
}

void write_binary(void) {
    FILE *f = fopen(BIN_FILE, "wb");
    if (f == NULL) { perror("fopen"); exit(1); }

    Record records[] = {
        {"Alpha", 100, 1.00f},
        {"Beta",   88, 0.88f},
        {"Gamma",  74, 0.74f},
    };
    int count = sizeof(records) / sizeof(records[0]);

    /* Write the count first so the reader knows how many records follow */
    fwrite(&count, sizeof(int), 1, f);

    /* fwrite(ptr, element_size, element_count, file) */
    size_t written = fwrite(records, sizeof(Record), count, f);
    printf("wrote %zu record(s) to %s\n", written, BIN_FILE);

    fclose(f);
}

void read_binary(void) {
    FILE *f = fopen(BIN_FILE, "rb");
    if (f == NULL) { perror("fopen"); exit(1); }

    int count = 0;
    fread(&count, sizeof(int), 1, f);

    Record *records = malloc(count * sizeof(Record));
    if (records == NULL) { fclose(f); exit(1); }

    /* fread(ptr, element_size, element_count, file) — returns elements read */
    size_t got = fread(records, sizeof(Record), count, f);
    printf("read %zu record(s) from %s\n", got, BIN_FILE);

    for (int i = 0; i < (int)got; i++) {
        printf("  %-8s score=%-4d ratio=%.2f\n",
               records[i].name, records[i].score, records[i].ratio);
    }

    free(records);
    fclose(f);
}

void append_text(void) {
    FILE *f = fopen(TEXT_FILE, "a");   /* "a" — file pointer starts at end */
    if (f == NULL) { perror("fopen"); exit(1); }

    fprintf(f, "Line 4: appended later\n");

    fclose(f);
    printf("appended a line to %s — full contents now:\n", TEXT_FILE);
}

long file_size(const char *path) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) return -1;
    fseek(f, 0, SEEK_END);   /* seek to end */
    long size = ftell(f);    /* position == size in bytes */
    fclose(f);
    return size;
}
