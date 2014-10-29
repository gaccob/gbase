#include <assert.h>
#include "core/thread.h"
#include "core/atom.h"

atom_t atom_flag;
uint32_t atom_cmp;
#define ATOM_LOOP 1000000

void*
func(void* arg) {
    int index;
    for (index = 0; index < ATOM_LOOP; index ++) {
        //atom_inc(&atom_flag);
        atom_add(&atom_flag, 2);
        atom_cmp += 2;
    }
    return NULL;
}

int
test_atom() {
    pthread_t t1, t2;
    atom_flag = 0;
    atom_cmp = 0;
    atom_set(&atom_flag, 10);
    pthread_create(&t1, NULL, func, NULL);
    pthread_create(&t2, NULL, func, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    printf("\natom_flag=%u atom_cmp=%u...\n", atom_flag, atom_cmp);
    return 0;
}

