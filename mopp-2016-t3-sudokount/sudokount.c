#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include "thpool.h"

#define INT_TYPE unsigned long long 
#define INT_TYPE_SIZE (sizeof(INT_TYPE) * 8)
#define CELL_VAL_SIZE 1
//MAX_BDIM = floor(sqrt(CELL_VAL_SIZE * INT_TYPE_SIZE)). Current value set for 64-bit INT_TYPE, adjust if needed
#define MAX_BDIM 8

enum SOLVE_STRATEGY {SUDOKU_SOLVE, SUDOKU_COUNT_SOLS};
#define SUDOKU_SOLVE_STRATEGY SUDOKU_COUNT_SOLS

#define BUILD_ERROR_IF(condition) ((void)sizeof(char[1 - 2*!!(condition)]))
void BUILD_TIME_CHECKS() {
    BUILD_ERROR_IF(INT_TYPE_SIZE * CELL_VAL_SIZE < MAX_BDIM * MAX_BDIM);
}

typedef struct cellval {
    INT_TYPE v[CELL_VAL_SIZE];
} cell_v;

typedef struct cell_coord {
    int r,c;
} cell_coord;

typedef struct sudoku {
    int bdim;
    int dim;
    int peers_size;
    int* grid;
    cell_coord ****unit_list; //[r][c][0 - row, 1 - column, 2 - box],
    cell_coord ***peers;
    cell_v **values;
    
    unsigned long long sol_count;
} sudoku;

typedef struct thread_work_t{
    sudoku *s;
    cell_v **values;
    int depth_t;
    int status;
} thread_work_t;

threadpool thpool;
int cpus; 
int thread_no  = 0;
int tasks_ip = 0;
int tasks_d = 0;

static int assign (sudoku *s, cell_v **values, int i, int j, int d);
void freemem(sudoku *s, cell_v **values);
void doWork(void *arg);
void wait();

static inline int cell_v_get(cell_v *v, int p) {
    //printf("%lld\n", (*v).v[p]);
    return !!((*v).v[(p - 1) / INT_TYPE_SIZE] & (((INT_TYPE)1) << ((p - 1) % INT_TYPE_SIZE)));
}

static inline void cell_v_unset(cell_v *v, int p) {
    //printf("%lld\n", (*v).v[p]);
    (*v).v[(p - 1) / INT_TYPE_SIZE] &= ~(((INT_TYPE)1) << ((p - 1) % INT_TYPE_SIZE));
}

static inline void cell_v_set(cell_v *v, int p) {
    //printf("%lld\n", (*v).v[p]);
    (*v).v[(p - 1) / INT_TYPE_SIZE] |= ((INT_TYPE)1) << ((p -1) % INT_TYPE_SIZE);
}

static inline int cell_v_count(cell_v *v) {
    int acc = 0;
    for (int i = 0; i < CELL_VAL_SIZE; i++) {
       // printf("%lld\n", (*v).v[i]);
        acc += __builtin_popcountll((*v).v[i]);
    }
    return acc;
}

static inline int digit_get (cell_v *v) {
    int count = cell_v_count(v);
    if (count != 1) return -1;
    for (int i = 0; i < CELL_VAL_SIZE; i++) 
        if ((*v).v[i]) return 1 + INT_TYPE_SIZE * i + __builtin_ctzll((*v).v[i]);
    return -1;
}

static void destroy_sudoku(sudoku *s) {
    for (int i = 0; i < s->dim; i++) {
        for (int j = 0; j < s->dim; j++) {
            for (int k = 0; k < 3; k++)
                free(s->unit_list[i][j][k]);
            free(s->unit_list[i][j]);
        }
        free(s->unit_list[i]);
    }
    free(s->unit_list);
    
    for (int i = 0; i < s->dim; i++) {
        for (int j = 0; j < s->dim; j++)
            free(s->peers[i][j]);
        free(s->peers[i]);
    }
    free(s->peers);
    
    for (int i = 0; i < s->dim; i++) 
        free(s->values[i]);
    free(s->values);
    
    free(s);
}

static void init(sudoku *s) {
    int i, j, k, l, pos;
    
    //unit list 
    for (i = 0; i < s->dim; i++) {
        int ibase = i / s->bdim * s->bdim;
        for (j = 0; j < s->dim; j++) {
            for (pos = 0; pos < s->dim; pos++) {
                s->unit_list[i][j][0][pos].r = i;
                s->unit_list[i][j][0][pos].c = pos;
                s->unit_list[i][j][1][pos].r = pos;
                s->unit_list[i][j][1][pos].c = j;
            }
            int jbase = j / s->bdim * s->bdim;
            for (pos = 0, k = 0; k < s->bdim; k++)
                for (l = 0; l < s->bdim; l++, pos++) {
                    s->unit_list[i][j][2][pos].r = ibase + k;
                    s->unit_list[i][j][2][pos].c = jbase + l;
                }
        }
    }
    
    //peers
    for (i = 0; i < s->dim; i++)
        for (j = 0; j < s->dim; j++) {
            pos = 0;
            for (k = 0; k < s->dim; k++) {
                if (s->unit_list[i][j][0][k].c != j)
                    s->peers[i][j][pos++] = s->unit_list[i][j][0][k]; 
            }
            for (k = 0; k < s->dim; k++) { 
                cell_coord sq = s->unit_list[i][j][1][k];
                if (sq.r != i)
                    s->peers[i][j][pos++] = sq; 
                sq = s->unit_list[i][j][2][k];
                if (sq.r != i && sq.c != j)
                    s->peers[i][j][pos++] = sq; 
            }
        }
    assert(pos == s->peers_size);
}

static int parse_grid(sudoku *s) {
    int i, j, k;
    int ld_vals[s->dim][s->dim];
    for (k = 0, i = 0; i < s->dim; i++)
        for (j = 0; j < s->dim; j++, k++) {
            ld_vals[i][j] = s->grid[k];
        }
    
    for (i = 0; i < s->dim; i++)
        for (j = 0; j < s->dim; j++)
            for (k = 1; k <= s->dim; k++)
                cell_v_set(&s->values[i][j], k);
    
    for (i = 0; i < s->dim; i++)
        for (j = 0; j < s->dim; j++)
            if (ld_vals[i][j] > 0 && !assign(s, s->values, i, j, ld_vals[i][j]))
                return 0;

    return 1;
}

static sudoku *create_sudoku(int bdim, int *grid) {
    assert(bdim <= MAX_BDIM);
    
    sudoku *r = malloc(sizeof(sudoku));
    r->bdim = bdim;
    int dim = bdim * bdim;
    r->dim = dim;
    r->peers_size = 3 * dim - 2 * bdim - 1;
    r->grid = grid;
    r->sol_count = 0;

    //[r][c][0 - row, 1 - column, 2 - box]//[r][c][0 - row, 1 - column, 2 - box][ix]
    r->unit_list = malloc(sizeof(cell_coord***) * dim);
    assert(r->unit_list);
    for (int i = 0; i < dim; i++) {
        r->unit_list[i] = malloc(sizeof(cell_coord**) * dim);
        assert (r->unit_list[i]);
        for (int j = 0; j < dim; j++) {
            r->unit_list[i][j] = malloc(sizeof(cell_coord*) * 3);
            assert(r->unit_list[i][j]);
            for (int k = 0; k < 3; k++) {
                r->unit_list[i][j][k] = calloc(dim, sizeof(cell_coord));
                assert(r->unit_list[i][j][k]);
            }
        }
    }
    
    r->peers = malloc(sizeof(cell_coord**) * dim);
    assert(r->peers);
    for (int i = 0; i < dim; i++) {
        r->peers[i] = malloc(sizeof(cell_coord*) * dim);
        assert(r->peers[i]);
        for (int j = 0; j < dim; j++) {
            r->peers[i][j] = calloc(r->peers_size, sizeof(cell_coord));
            assert(r->peers[i][j]);
        }
    }
    
    r->values = malloc (sizeof(cell_v*) * dim);
    assert(r->values);
    for (int i = 0; i < dim; i++) {
        r->values[i] = calloc(dim, sizeof(cell_v));
        assert(r->values[i]);
    }
    init(r);
    if (!parse_grid(r)) {
        printf("Error parsing grid\n");
        destroy_sudoku(r);
        return 0;
    }
    
    return r;
}

static int eliminate (sudoku *s, cell_v **values, int i, int j, int d) {
    int k, ii, cont, pos;
    if (!cell_v_get(&values[i][j], d)) 
        return 1;

    cell_v_unset(&values[i][j], d);

    int count = cell_v_count(&values[i][j]);
    if (count == 0) {
        return 0;
    } else if (count == 1) {
        for (k = 0; k < s->peers_size; k++) 
            if (!eliminate(s, values, s->peers[i][j][k].r, s->peers[i][j][k].c, digit_get(&values[i][j])))
                return 0;
    }

    for (k = 0; k < 3; k++) {//row, column, box 
        cont = 0;
        pos = 0;
        cell_coord* u = s->unit_list[i][j][k];
        for (ii = 0; ii < s->dim; ii++) {
            if (cell_v_get(&values[u[ii].r][u[ii].c], d)) {
                cont++;
                pos = ii;
            }
        }
        if (cont == 0)
            return 0;
        else if (cont == 1) {
            if (!assign(s, values, u[pos].r, u[pos].c, d))
                return 0;
        }
    }
    return 1;
}

static int assign (sudoku *s, cell_v **values, int i, int j, int d) {
    for (int d2 = 1; d2 <= s->dim; d2++)
        if (d2 != d) 
            if (!eliminate(s, values, i, j, d2))
               return 0;
    return 1;
}

static void search (sudoku *s, cell_v **values, int depth, int status) {
    int i, j, k;
    depth++;

    if (!status) {
        return;
    }

    int solved = 1;
    for (i = 0; solved && i < s->dim; i++) 
        for (j = 0; j < s->dim; j++) 
            if (cell_v_count(&values[i][j]) != 1) {
                solved = 0;
                break;
            }
    if (solved) {
        __sync_fetch_and_add(&s->sol_count, 1);
        return;
    }

    //ok, there is still some work to be done
    int min = INT_MAX;
    int minI = -1;
    int minJ = -1;

    for (i = 0; i < s->dim; i++) 
        for (j = 0; j < s->dim; j++) {
            int used = cell_v_count(&values[i][j]);
            if (used > 1 && used < min) {
                min = used;
                minI = i;
                minJ = j;
            }
        }

    for (k = 1; k <= s->dim; k++) {
        if (cell_v_get(&values[minI][minJ], k))  {
            cell_v **values_bkp = malloc (sizeof (cell_v *) * s->dim);
            for (i = 0; i < s->dim; i++)
                values_bkp[i] = malloc (sizeof (cell_v) * s->dim);
            for (i = 0; i < s->dim; i++)
                for (j = 0; j < s->dim; j++)
                    values_bkp[i][j] = values[i][j];
            int status_now = assign(s, values_bkp, minI, minJ, k);
            if (depth > 15) {
                thread_work_t* tw = (thread_work_t*) malloc(sizeof(thread_work_t));
                tw->s = s;      
                tw->values = values_bkp;
                tw->depth_t = 0;
                tw->status = status_now;
                __sync_fetch_and_add(&tasks_ip, 1);
                if (thpool_num_threads_working(thpool) < cpus) {
                    thread_no ++;
                    thpool_add_work(thpool, &doWork, tw);
                }
                else {
                    depth = 0;
                    __sync_fetch_and_sub(&tasks_ip, 1);
                    search(s, values_bkp, depth, status_now);
                    freemem(s, values_bkp);
                    
                }
            } else {
                search(s, values_bkp, depth, status_now);
                freemem(s, values_bkp);
            }
        }
    }
}

void freemem(sudoku *s, cell_v **values) {
    for (int i = 0; i < s->dim; i++)
        free(values[i]);
    free (values);
}

void doWork(void *thread_work_uncasted) {
    struct thread_work_t *thread_work = (struct thread_work_t*)thread_work_uncasted;
    sudoku *s = thread_work->s;
    cell_v **values =  thread_work->values;
    int depth_t = thread_work->depth_t;
    int status = thread_work->status;
    search(s, values, depth_t, status);
    freemem(s, values);
    free(thread_work);
    __sync_fetch_and_add(&tasks_d, 1);
}

int solve(sudoku *s) {
    search(s, s->values, 15, 1);
    wait();
    return 0;
}

void wait() {
    while (tasks_ip != tasks_d) {
        sleep(1);
    }
return;
}

static void display(sudoku *s) {
    printf("%d\n", s->bdim);
    for (int i = 0; i < s->dim; i++)
        for (int j = 0; j < s->dim; j++)
            printf("%d ",  digit_get(&s->values[i][j]));
}

int main (int argc, char **argv) {

    int size;
    assert(scanf("%d", &size) == 1);
    assert (size <= MAX_BDIM);
    int buf_size = size * size * size * size;
    int buf[buf_size];

    for (int i = 0; i < buf_size; i++) {
        if (scanf("%d", &buf[i]) != 1) {
            printf("error reading file (%d)\n", i);
            exit(1);
        }
    }

    cpus = get_nprocs();
        // npros() might return wrong amount inside of a container.
        // Use MAX_CPUS instead, if available.
        if (getenv("MAX_CPUS")) {
        cpus = atoi(getenv("MAX_CPUS"));
        }
    // Sanity-check
        assert(cpus > 0 && cpus <= 64);
    //cpus = 1;
    thpool = thpool_init(cpus);

    sudoku *s = create_sudoku(size, buf);
    if (s) {
        solve(s);
        if (s->sol_count) {
            switch (SUDOKU_SOLVE_STRATEGY) {
                case SUDOKU_SOLVE:
                    display(s);
                    break;
                case SUDOKU_COUNT_SOLS: 
                    printf("%lld\n", s->sol_count);
                    break;
                default:
                    assert(0);
            }
        } else {
            printf("Could not solve puzzle.\n");
        }
        destroy_sudoku(s);
    } else {
        printf("Could not load puzzle.\n");
    }

    thpool_destroy(thpool);
    return 0;
}

