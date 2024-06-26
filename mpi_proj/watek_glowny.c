#include "main.h"
#include "util.h"
#include "watek_glowny.h"
#include <time.h>

pthread_barrier_t barrier;

void resetValues()
{
	// zerowanie wartosci dla nowego cyklu
	ackCount = 0;
	memset(students_list, 0, sizeof(students_list)); // zerowanie wartosci listy
	count = 0;										 // Licznik dodanych studentów
	ack_kill_count = 0;								 // Liczba otrzymanych ACK_KILL
	victim_count = 0;
	killer_count = 0;
	is_killing = FALSE;
	THE_END_counter = 0;
	beer_counter = 0;
	victim_count_const = 0;
	killer_count_const = 0;
	// lamport_clock = 0;
}

void mainLoop()
{
	// Inicjalizacja bariery
	pthread_barrier_init(&barrier, NULL, size);

	srandom(time(NULL) + rank);
	int tag;
	int perc;

	while (stan != InFinish)
	{
		switch (stan)
		{
		case REST:

			perc = random() % 100;
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = (perc < 50) ? KILLER : VICTIM; // Losowanie roli
			if (pkt->data == KILLER)
			{
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Jestem zabojca!!\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);
			}
			else
			{
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Jestem ofiarą.. Trudne sie wylosowalo\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);
			}

			for (int i = 0; i < size; i++)
			{
				sendPacket(pkt, i, MSG_ROLE); // Wysyłanie roli do wszystkich i do siebie
			}
			changeState((pkt->data == KILLER) ? KILLER : VICTIM); // Zmiana stanu na KILLER lub VICTIM
			break;
		case VICTIM:
			if (beer_counter == size - 1)
			{
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Jestem ofiara i mowie Koniec!\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);

				resetValues();

				changeState(REST);
			}
			// else
			//  {
			//  	pthread_mutex_lock(&beer_mutex);
			//  	pthread_mutex_lock(&beer_mutex);
			//  	pthread_mutex_unlock(&beer_mutex);
			//  }
			break;
		case KILLER:
			if (ackCount == size - 1)
			{
				changeState(WANNAKILL);
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Wchodze w stan WANNAKILL!\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);
			}
			break;
		case WANNAKILL:

			for (int i = 0; i < size; i++)
			{
				if (i != rank)
				{
					sendPacket(0, i, REQ_KILL); // Wysyłanie checi bitwy do wszystkich
				}
			}

			if (beer_counter == size - 1)
			{
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Jestem zabojca i mowie Koniec!\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);

				resetValues();

				changeState(REST);

				break;
			}
			// else
			// {
			// 	pthread_mutex_lock(&beer_mutex);
			// 	pthread_mutex_lock(&beer_mutex);
			// 	pthread_mutex_unlock(&beer_mutex);
			// }

			break;
		case KILLING:

			// Wchodzenie do sekcji krytycznej
			// pthread_mutex_lock(&student_list_mutex); // Blokowanie dostępu do listy studentow

			debug("Zawartość students_list przed KILLING:\n");
			// printf("Zawartość students_list przed KILLING:\n");
			for (int i = 0; i < count; i++)
			{
				debug("students_list[%d].src = %d, students_list[%d].data = %d\n", i, students_list[i].src, i, students_list[i].data);
				// printf("students_list[%d].src = %d, students_list[%d].data = %d\n", i, students_list[i].src, i, students_list[i].data);
			}

			if (count > 0)
			{
				int victim_found = 0;
				packet_t victim;
				for (int i = 0; i < count; i++)
				{
					// wziecie pierwszej ofiary z listy
					if (students_list[i].data == VICTIM)
					{
						victim = students_list[i];
						victim_found = 1;
						// usuniecie tej ofiary z listy
						// for (int j = i; j < count; j++)
						// {
						// 	students_list[j] = students_list[j + 1];
						// }
						count--;
						break;
					}
				}
				pthread_mutex_lock(&lamport_clock_mutex);
				if (victim_found)
				{ // Sprawdzenie, czy ofiara została znaleziona
					debug("Paruję się z ofiarą %d\n", victim.src);
					printf("[%d][%d] Paruję się z ofiarą %d\n", rank, lamport_clock, victim.src);

					// Losowanie wyniku starcia
					double result = (double)rand() / RAND_MAX;
					if (result > 0.5)
					{
						debug("Wygrałem starcie z %d\n", victim.src);
						printf("[%d][%d] Wygrałem starcie z %d\n", rank, lamport_clock, victim.src);
						// Możesz dodać dodatkową logikę dla wygranej
					}
					else
					{
						debug("Przegrałem starcie z %d\n", victim.src);
						printf("[%d][%d] Przegrałem starcie z %d\n", rank, lamport_clock, victim.src);
						// Możesz dodać dodatkową logikę dla przegranej
					}
				}
				else
				{
					debug("Nie znaleziono ofiary\n");
					printf("[%d][%d] Nie znaleziono mi ofiary\n", lamport_clock, rank);
				}
				pthread_mutex_unlock(&lamport_clock_mutex);
			}
			for (int i = 0; i < size; i++)
			{
				// if (i != rank)
				// {
				sendPacket(0, i, THE_END); // wysylanie wiadomosci o koncu walki
										   //}
			}

			changeState(ITS_OVER);

			pthread_mutex_lock(&lamport_clock_mutex);
			printf("[%d][%d] Wychodze z sekcji krytycznej\n\n", rank, lamport_clock);
			pthread_mutex_unlock(&lamport_clock_mutex);

			// pthread_mutex_unlock(&student_list_mutex); // Odblokowanie dostępu do listy studentów

			break;

		case ITS_OVER:
			if (beer_counter == size - 1)
			{
				pthread_mutex_lock(&lamport_clock_mutex);
				printf("[%d][%d] Jestem zabojca i mowie Koniec!\n", rank, lamport_clock);
				pthread_mutex_unlock(&lamport_clock_mutex);

				resetValues();

				changeState(REST);
			}
			// else
			// {
			// 	pthread_mutex_lock(&beer_mutex);
			// 	pthread_mutex_lock(&beer_mutex);
			// 	pthread_mutex_unlock(&beer_mutex);
			// }
			break;
		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
