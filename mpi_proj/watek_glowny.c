#include "main.h"
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
	victim_count_local = 0;
	killer_count_local = 0;
	lamport_clock = 0;
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
				printf("[%d] Jestem zabojca!!\n", rank);
			}
			else
			{
				printf("[%d] Jestem ofiarą.. Trudne sie wylosowalo\n", rank);
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
				printf("[%d] Jestem ofiara i mowie Koniec!\n", rank);

				resetValues();

				changeState(REST);
			}
			break;
		case KILLER:
			if (ackCount == size - 1)
			{
				changeState(WANNAKILL);
				printf("[%d] Wchodze w stan WANNAKILL!\n", rank);
			}
			break;
		case WANNAKILL:
			if (beer_counter == size - 1)
			{
				printf("[%d] Jestem zabojca i mowie Koniec!\n", rank);

				resetValues();

				changeState(REST);

				break;
			}
			for (int i = 0; i < size; i++)
			{
				if (i != rank)
				{
					sendPacket(0, i, REQ_KILL); // Wysyłanie checi bitwy do wszystkich
				}
			}

			break;
		case KILLING:

			// Wchodzenie do sekcji krytycznej
			pthread_mutex_lock(&student_list_mutex); // Blokowanie dostępu do listy studentow

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
				if (victim_found)
				{ // Sprawdzenie, czy ofiara została znaleziona
					debug("Paruję się z ofiarą %d\n", victim.src);
					printf("[%d] Paruję się z ofiarą %d\n", rank, victim.src);

					// Losowanie wyniku starcia
					double result = (double)rand() / RAND_MAX;
					if (result > 0.5)
					{
						debug("Wygrałem starcie z %d\n", victim.src);
						printf("[%d] Wygrałem starcie z %d\n", rank, victim.src);
						// Możesz dodać dodatkową logikę dla wygranej
					}
					else
					{
						debug("Przegrałem starcie z %d\n", victim.src);
						printf("[%d] Przegrałem starcie z %d\n", rank, victim.src);
						// Możesz dodać dodatkową logikę dla przegranej
					}
				}
				else
				{
					debug("Nie znaleziono ofiary\n");
					printf("[%d] Nie znaleziono mi ofiary\n", rank);
				}
			}
			for (int i = 0; i < size; i++)
			{
				// if (i != rank)
				// {
				sendPacket(0, i, THE_END); // wysylanie wiadomosci o koncu walki
										   //}
			}

			changeState(ITS_OVER);

			printf("[%d] Wychodze z sekcji krytycznej\n", rank);

			pthread_mutex_unlock(&student_list_mutex); // Odblokowanie dostępu do listy studentów

			break;

		case ITS_OVER:
			if (beer_counter == size - 1)
			{
				printf("[%d] Jestem zabojca i mowie Koniec!\n", rank);

				resetValues();

				changeState(REST);
			}
			break;
		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
