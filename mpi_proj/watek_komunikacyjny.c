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
        case MSG_ROLE:
            debug("Otrzymalem rolę %s od %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src);
            // printf("Otrzymalem rolę %s od %d z zegarem %d\n", (pakiet.data == KILLER) ? "KILLER" : "VICTIM", pakiet.src, pakiet.ts);
            if (pakiet.data == VICTIM)
            {
                victim_count++;
                victim_count_const++;
            }
            else
            {
                killer_count++;
                killer_count_const++;
            }

            if (victim_count == size || killer_count == size)
            {
                for (int i = 0; i < size; i++)
                {
                    if (rank != i)
                        sendPacket(0, i, BEER_TIME); // wysylanie beer time w razie gdy wylosuja sie role tylko jednego typu
                }
            }

            // Dodanie procesu do listy studentów
            insert_student(pakiet);
            if (rank != pakiet.src)
                sendPacket(0, pakiet.src, ACK_ROLE); // Wysylanie potwierdzenia ACK_ROLE
            break;
        case ACK_ROLE:
            debug("Otrzymalem ACK_ROLE od %d\n", pakiet.src);
            // printf("Otrzymalem ACK_ROLE od %d\n", pakiet.src);
            ackCount++;
            break;
        case REQ_KILL:
            debug("Otrzymalem REQ_KILL od %d\n", pakiet.src);
            sendPacket(0, pakiet.src, ACK_KILL);
            break;
        case ACK_KILL:
            debug("Otrzymalem ACK_KILL od %d\n", pakiet.src);
            ack_kill_count++;

            // odczytanie pierwszego zadania od killera w liscie
            int first_killer = 100;
            for (int i = 0; i < victim_count + killer_count; i++)
            {
                if (students_list[i].data == KILLER)
                {
                    first_killer = students_list[i].src;
                    break;
                }
            }

            if ((ack_kill_count >= size - 1 - victim_count_const) && (min(victim_count, killer_count) != 0) && (first_killer == rank) && (!is_killing))
            {
                is_killing = TRUE;
                changeState(KILLING);
                victim_count--;
                killer_count--;
                debug("Przechodzę w stan KILLING\n");
                printf("\n[%d][%d] Przechodze w stan KILLING (wchodze do sekcji krytycznej)\n", rank, pakiet.ts);
            }
            break;
        case THE_END:
            debug("Otrzymalem THE_END od %d\n", pakiet.src);
            // printf("[%d] Otrzymalem THE_END od %d\n", rank, pakiet.src);

            for (int i = 0; i < count; i++)
            {
                // wziecie pierwszej ofiary z listy
                if (students_list[i].data == VICTIM)
                {
                    // usuniecie tej ofiary z listy
                    for (int j = i; j < count; j++)
                    {
                        students_list[j] = students_list[j + 1];
                    }
                    victim_count--;
                    count--;
                    break;
                }
            }

            for (int i = 0; i < count; i++)
            {
                // wziecie pierwszego zabojcy z listy
                if (students_list[i].data == KILLER)
                {
                    // usuniecie tego zabojcy z listy
                    for (int j = i; j < count; j++)
                    {
                        students_list[j] = students_list[j + 1];
                    }
                    killer_count--;
                    count--;
                    break;
                }
            }

            // wszystkie procesy ktore nie byly w sekcji spelnia ten warunek, dlatego beer_counter na koncu bedzie rowny size - min(killer_count_local, victim_count_local)
            THE_END_counter++;
            if (THE_END_counter == min(killer_count_const, victim_count_const))
            {
                for (int i = 0; i < size; i++)
                {
                    if (rank != i)
                        sendPacket(0, i, BEER_TIME); // beer time
                }
            }

            break;
        case BEER_TIME:
            beer_counter++;
            // pthread_mutex_trylock(&beer_mutex);
            // pthread_mutex_unlock(&beer_mutex);

            break;
        default:
            break;
        }
    }
}
