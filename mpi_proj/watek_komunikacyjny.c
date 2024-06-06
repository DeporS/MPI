#include "main.h"
#include "watek_komunikacyjny.h"

pthread_mutex_t student_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void insert_student(packet_t pakiet)
{
    // pthread_mutex_lock(&student_list_mutex);

    students_list[count] = pakiet;
    count++;

    // sortowanie listy studentow po dodaniu nowego studenta
    for (int i = 0; i < count - 1; i++)
    {
        for (int j = 0; j < count - i - 1; j++)
        {
            if (students_list[j].ts > students_list[j + 1].ts ||
                (students_list[j].ts == students_list[j + 1].ts && students_list[j].src > students_list[j + 1].src))
            {
                packet_t temp = students_list[j];
                students_list[j] = students_list[j + 1];
                students_list[j + 1] = temp;
            }
        }
    }

    // pthread_mutex_unlock(&student_list_mutex);
}

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
        // case REQUEST:
        //     debug("Ktoś coś prosi. A niech ma!");
        //     sendPacket(0, status.MPI_SOURCE, ACK);
        //     break;
        // case ACK:
        //     debug("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
        //     printf("Dostałem ACK od %d, mam już %d", status.MPI_SOURCE, ackCount);
        //     ackCount++; /* czy potrzeba tutaj muteksa? Będzie wyścig, czy nie będzie? Zastanówcie się. */
        //     break;
        case MSG_KILL:
            // MSG_KILL-specific logic here
            break;
        case MSG_ROLE:
            debug("Otrzymalem rolę %s od %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src);
            // printf("Otrzymalem rolę %s od %d z zegarem %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src, pakiet.ts);
            if (pakiet.data == VICTIM)
            {
                victim_count++;
            }
            else
            {
                killer_count++;
            }

            // Dodanie procesu do listy studentów
            insert_student(pakiet);

            sendPacket(0, pakiet.src, ACK_ROLE); // Wysylanie potwierdzenia ACK_ROLE
            break;
        case ACK_ROLE:
            debug("Otrzymalem ACK_ROLE od %d\n", pakiet.src);
            // printf("Otrzymalem ACK_ROLE od %d\n", pakiet.src);
            ackCount++;
            break;
        case MSG_VIC:
            // MSG_VIC-specific logic here
            break;
        case REQ_KILL:
            debug("Otrzymalem REQ_KILL od %d\n", pakiet.src);
            sendPacket(0, pakiet.src, ACK_KILL);
            break;
        case ACK_KILL:
            debug("Otrzymalem ACK_KILL od %d\n", pakiet.src);
            ack_kill_count++;
            if ((ack_kill_count == size - 1 - victim_count) && min(victim_count, killer_count) != 0)
            {
                changeState(KILLING);
                debug("Przechodzę w stan KILLING\n");
                printf("Przechodze w stan KILLING\n");
            }
            break;
        case THE_END:
            debug("Otrzymalem THE_END od %d\n", pakiet.src);
            victim_count--;
            killer_count--;
            for (int i = 0; i < count - 1; i++)
            {
                // wziecie pierwszej ofiary z listy
                if (students_list[i].data == VICTIM)
                {
                    // usuniecie tej ofiary z listy
                    for (int j = i; j < count; j++)
                    {
                        students_list[j] = students_list[j + 1];
                    }
                    count--;
                    break;
                }
            }
            break;
        case BEER_TIME:
            // BEER_TIME-specific logic here
            break;
        default:
            break;
        }
    }
}
