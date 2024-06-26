#ifndef UTILH
#define UTILH
#include "main.h"

/* typ pakietu */
typedef struct
{
    int ts; /* timestamp (zegar lamporta */
    int src;

    int data; /* przykładowe pole z danymi; można zmienić nazwę na bardziej pasującą */
} packet_t;
/* packet_t ma trzy pola, więc NITEMS=3. Wykorzystane w inicjuj_typ_pakietu */

extern MPI_Datatype MPI_PAKIET_T;
void inicjuj_typ_pakietu();

/* wysyłanie pakietu, skrót: wskaźnik do pakietu (0 oznacza stwórz pusty pakiet), do kogo, z jakim typem */
void sendPacket(packet_t *pkt, int destination, int tag);

typedef enum
{
    InRun,
    InWant,
    InSection,
    InFinish,
    REST,
    VICTIM,
    KILLER,
    WANNAKILL,
    KILLING,
    ITS_OVER
} state_t;

extern state_t stan;
extern pthread_mutex_t lamport_clock_mutex;
extern pthread_mutex_t stateMut;
extern pthread_mutex_t student_list_mutex;
extern pthread_mutex_t beer_mutex;

/* zmiana stanu, obwarowana muteksem */
void changeState(state_t);

#define max(a, b) \
    ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a > _b ? _a : _b; })

#endif
