#ifndef MAINH
#define MAINH
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#include "util.h"
/* boolean */
#define TRUE 1
#define FALSE 0
#define SEC_IN_STATE 2
#define STATE_CHANGE_PROB 10

#define NITEMS 3

#define REQUEST 1
#define ACK 2
#define FINISH 3
#define APP_PKT 4
#define RELEASE 5

#define ACK_ROLE 6
#define REQ_KILL 7
#define ACK_KILL 8
#define MSG_KILL 9
#define MSG_VIC 10
#define THE_END 11
#define BEER_TIME 12
#define MSG_ROLE 13

#define ROOT 0

#define REST 0
#define KILLER 1
#define VICTIM 2

/* tutaj TYLKO zapowiedzi - definicje w main.c */
extern int rank;
extern int size;
extern int ackCount;
extern int lamport_clock;
extern int count;
extern packet_t students_list[]; // Lista studentów

extern pthread_t threadKom;

/* macro debug - działa jak printf, kiedy zdefiniowano
   DEBUG, kiedy DEBUG niezdefiniowane działa jak instrukcja pusta

   używa się dokładnie jak printfa, tyle, że dodaje kolorków i automatycznie
   wyświetla rank

   w związku z tym, zmienna "rank" musi istnieć.

   w printfie: definicja znaku specjalnego "%c[%d;%dm [%d]" escape[styl bold/normal;kolor [RANK]
                                           FORMAT:argumenty doklejone z wywołania debug poprzez __VA_ARGS__
                       "%c[%d;%dm"       wyczyszczenie atrybutów    27,0,37
                                            UWAGA:
                                                27 == kod ascii escape.
                                                Pierwsze %c[%d;%dm ( np 27[1;10m ) definiuje styl i kolor literek
                                                Drugie   %c[%d;%dm czyli 27[0;37m przywraca domyślne kolory i brak pogrubienia (bolda)
                                                ...  w definicji makra oznacza, że ma zmienną liczbę parametrów

*/
#ifdef DEBUG
#define debug(FORMAT, ...) printf("%c[%d;%dm [%d:%d]: " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamport_clock, ##__VA_ARGS__, 27, 0, 37);
#else
#define debug(...) ;
#endif

// makro println - to samo co debug, ale wyświetla się zawsze
#define println(FORMAT, ...) printf("%c[%d;%dm [%d:%d]: " FORMAT "%c[%d;%dm\n", 27, (1 + (rank / 7)) % 2, 31 + (6 + rank) % 7, rank, lamport_clock, ##__VA_ARGS__, 27, 0, 37);

#endif
