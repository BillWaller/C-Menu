#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uthash/uthash.h>

struct my_struct {
    int id;            /* key */
    char name[32];     /* data */
    UT_hash_handle hh; /* makes this structure hashable */
};

struct my_struct *users = NULL; /* important: initialize to NULL */

void add_user(int user_id, const char *name) {
    struct my_struct *s;
    HASH_FIND_INT(users, &user_id, s); /* check if already exists */
    if (s == NULL) {
        s = malloc(sizeof(*s));
        s->id = user_id;
        HASH_ADD_INT(users, id, s); /* id is the key field */
    }
    strcpy(s->name, name);
}

struct my_struct *find_user(int user_id) {
    struct my_struct *s;
    HASH_FIND_INT(users, &user_id, s);
    return s;
}

void delete_user(struct my_struct *user) {
    HASH_DEL(users, user);
    free(user);
}

int main(void) {
    add_user(1, "Alice");
    add_user(2, "Bob");

    struct my_struct *s = find_user(1);
    if (s) {
        printf("Found: %s\n", s->name);
    }

    delete_user(s);
    return 0;
}
