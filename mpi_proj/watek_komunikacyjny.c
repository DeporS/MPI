#include "main.h"
#include "watek_komunikacyjny.h"

/* wątek komunikacyjny; zajmuje się odbiorem i reakcją na komunikaty */
void *startKomWatek(void *ptr)
{
    MPI_Status status;
    int is_message = FALSE;
    packet_t pakiet;
    /* Obrazuje pętlę odbierającą pakiety o różnych typach */
    while (stan != InFinish)
    {
        debug("czekam na recv");
        MPI_Recv(&pakiet, 1, MPI_PAKIET_T, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        pthread_mutex_lock(&lamport_clock_mutex);
        lamport_clock = max(lamport_clock, pakiet.ts) + 1;
        pthread_mutex_unlock(&lamport_clock_mutex);

        switch (status.MPI_TAG)
        {
        case REQUEST:
            debug("Ktoś coś prosi. A niech ma!");
            sendPacket(0, status.MPI_SOURCE, ACK);
            break;
        case ACK:
            debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
            ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
            break;
        case MSG_KILL:
            // MSG_KILL-specific logic here
            break;
        case MSG_ROLE:
            debug("Otrzymałem rolę %s od %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src);
            sendPacket(0, pakiet.src, ACK_ROLE); // Wysyłanie potwierdzenia ACK_ROLE
            break;
        case ACK_ROLE:
            debug("Otrzymałem ACK_ROLE od %d\n", pakiet.src);
            ackCount++;
            break;
        case MSG_VIC:
            // MSG_VIC-specific logic here
            break;
        case REQ_KILL:
            // REQ_KILL-specific logic here
            break;
        case ACK_KILL:
            // ACK_KILL-specific logic here
            break;
        case THE_END:
            // THE_END-specific logic here
            break;
        case BEER_TIME:
            // BEER_TIME-specific logic here
            break;
        default:
            break;
        }
    }
}
