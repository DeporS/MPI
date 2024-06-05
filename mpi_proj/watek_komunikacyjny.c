#include "main.h"
#include "watek_komunikacyjny.h"

pthread_mutex_t student_list_mutex = PTHREAD_MUTEX_INITIALIZER;

void insert_student(packet_t pakiet)
{
    student_t new_student = {pakiet.ts, pakiet.src, pakiet.data};

    pthread_mutex_lock(&student_list_mutex);

    students_list[student_count] = new_student;
    student_count++;

    // sortowanie listy studentow po dodaniu nowego studenta
    for (int i = 0; i < student_count - 1; i++)
    {
        for (int j = 0; j < student_count - i - 1; j++)
        {
            if (students_list[j].ts > students_list[j + 1].ts ||
                (students_list[j].ts == students_list[j + 1].ts && students_list[j].src > students_list[j + 1].src))
            {
                student_t temp = students_list[j];
                students_list[j] = students_list[j + 1];
                students_list[j + 1] = temp;
            }
        }
    }

    pthread_mutex_unlock(&student_list_mutex);
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
            debug("Otrzymałem rolę %s od %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src);
            printf("Otrzymałem rolę %s od %d z zegarem %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src, pakiet.ts);

            // Dodanie procesu do listy studentów
            insert_student(pakiet);

            sendPacket(0, pakiet.src, ACK_ROLE); // Wysyłanie potwierdzenia ACK_ROLE
            break;
        case ACK_ROLE:
            debug("Otrzymałem ACK_ROLE od %d\n", pakiet.src);
            printf("Otrzymałem ACK_ROLE od %d\n", pakiet.src);
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
