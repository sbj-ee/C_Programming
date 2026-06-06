#include <stdio.h>
#include <string.h>
#include <math.h>

/* --- struct: groups related variables under one name --- */
struct Point {
    double x;
    double y;
};

/* --- typedef: gives the struct a cleaner alias so you can write
   'Point' instead of 'struct Point' everywhere --- */
typedef struct {
    char   name[32];
    int    age;
    double gpa;
} Student;

/* --- Nested struct --- */
typedef struct {
    char          street[64];
    char          city[32];
    char          state[3];    /* e.g. "CA\0" */
} Address;

typedef struct {
    char    name[32];
    int     age;
    Address address;           /* struct embedded inside a struct */
} Person;

/* --- Struct with a pointer member (forward declaration) --- */
typedef struct Node {
    int          value;
    struct Node *next;         /* self-referential: pointer to same type */
} Node;

/* Function prototypes */
struct Point   make_point(double x, double y);
double         distance(struct Point a, struct Point b);
void           print_student(Student s);
void           birthday(Student *s);
void           print_person(Person *p);

/* ------------------------------------------------------------------ */

int main(void) {

    /* --- Basic struct usage --- */
    printf("=== Basic struct ===\n");

    struct Point p1 = {3.0, 4.0};   /* initialize with brace list */
    struct Point p2;
    p2.x = 0.0;                      /* member access with dot operator */
    p2.y = 0.0;

    printf("p1 = (%.1f, %.1f)\n", p1.x, p1.y);
    printf("p2 = (%.1f, %.1f)\n", p2.x, p2.y);
    printf("distance(p1, p2) = %.2f\n", distance(p1, p2));

    /* Struct returned by value from a function */
    struct Point p3 = make_point(6.0, 8.0);
    printf("p3 = (%.1f, %.1f)\n", p3.x, p3.y);

    /* --- typedef struct --- */
    printf("\n=== typedef struct ===\n");

    Student s1 = {"Alice", 20, 3.8};
    Student s2;
    strcpy(s2.name, "Bob");
    s2.age = 22;
    s2.gpa = 3.1;

    print_student(s1);
    print_student(s2);

    /* Structs are passed by value — function gets a copy */
    birthday(&s1);
    printf("after birthday: ");
    print_student(s1);

    /* --- Array of structs --- */
    printf("\n=== Array of structs ===\n");

    Student roster[] = {
        {"Carol", 19, 3.5},
        {"Dave",  21, 2.9},
        {"Eve",   20, 3.7},
    };
    int count = sizeof(roster) / sizeof(roster[0]);

    for (int i = 0; i < count; i++) {
        print_student(roster[i]);
    }

    /* --- Nested structs --- */
    printf("\n=== Nested structs ===\n");

    Person person = {
        .name = "Frank",
        .age  = 35,
        .address = {
            .street = "123 Main St",
            .city   = "Springfield",
            .state  = "IL"
        }
    };
    print_person(&person);

    /* Modify a nested field */
    strcpy(person.address.city, "Shelbyville");
    printf("moved to: %s, %s\n", person.address.city, person.address.state);

    /* --- Pointer to struct and the arrow operator --- */
    printf("\n=== Pointer to struct (arrow operator) ===\n");

    Student *sp = &s2;
    printf("via pointer: %s, age %d\n", sp->name, sp->age);   /* sp->field == (*sp).field */
    sp->age = 99;
    printf("after sp->age = 99: %d\n", s2.age);               /* modifies the original */

    /* --- Self-referential struct (basis for linked lists) --- */
    printf("\n=== Self-referential struct ===\n");

    Node n1 = {10, NULL};
    Node n2 = {20, NULL};
    Node n3 = {30, NULL};
    n1.next = &n2;
    n2.next = &n3;

    /* Walk the chain */
    Node *cur = &n1;
    while (cur != NULL) {
        printf("node value = %d\n", cur->value);
        cur = cur->next;
    }

    return 0;
}

/* ------------------------------------------------------------------ */

struct Point make_point(double x, double y) {
    struct Point p = {x, y};
    return p;
}

double distance(struct Point a, struct Point b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

void print_student(Student s) {
    printf("%-10s  age=%-3d  gpa=%.1f\n", s.name, s.age, s.gpa);
}

/* Takes a pointer so it can modify the caller's struct */
void birthday(Student *s) {
    s->age++;
}

void print_person(Person *p) {
    printf("%s (age %d)\n", p->name, p->age);
    printf("  %s, %s %s\n", p->address.street, p->address.city, p->address.state);
}
