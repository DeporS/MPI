#include "main.h"
#include "watek_glowny.h"

void mainLoop()
{
	srandom(rank);
	int tag;
	int perc;

	while (stan != InFinish)
	{
		switch (stan)
		{
		// case InRun:
		// perc = random()%100;
		// if ( perc < 25 ) {
		//     debug("Perc: %d", perc);
		//     println("Ubiegam się o sekcję krytyczną")
		//     debug("Zmieniam stan na wysyłanie");
		//     packet_t *pkt = malloc(sizeof(packet_t));
		//     pkt->data = perc;
		//     ackCount = 0;
		//     for (int i=0;i<=size-1;i++)
		// 	if (i!=rank)
		// 	    sendPacket( pkt, i, REQUEST);
		//     changeState( InWant ); // w VI naciśnij ctrl-] na nazwie funkcji, ctrl+^ żeby wrócić
		// 			   // :w żeby zapisać, jeżeli narzeka że w pliku są zmiany
		// 			   // ewentualnie wciśnij ctrl+w ] (trzymasz ctrl i potem najpierw w, potem ]
		// 			   // między okienkami skaczesz ctrl+w i strzałki, albo ctrl+ww
		// 			   // okienko zamyka się :q
		// 			   // ZOB. regułę tags: w Makefile (naciśnij gf gdy kursor jest na nazwie pliku)
		//     free(pkt);
		// } // a skoro już jesteśmy przy komendach vi, najedź kursorem na } i wciśnij %  (niestety głupieje przy komentarzach :( )
		// debug("Skończyłem myśleć");
		// break;
		// case InWant:
		// println("Czekam na wejście do sekcji krytycznej")
		// // tutaj zapewne jakiś semafor albo zmienna warunkowa
		// // bo aktywne czekanie jest BUE
		// if ( ackCount == size - 1)
		//     changeState(InSection);
		// break;
		// case InSection:
		// // tutaj zapewne jakiś muteks albo zmienna warunkowa
		// println("Jestem w sekcji krytycznej")
		//     sleep(5);
		// //if ( perc < 25 ) {
		//     debug("Perc: %d", perc);
		//     println("Wychodzę z sekcji krytycznej")
		//     debug("Zmieniam stan na wysyłanie");
		//     packet_t *pkt = malloc(sizeof(packet_t));
		//     pkt->data = perc;
		//     for (int i=0;i<=size-1;i++)
		// 	if (i!=rank)
		// 	    sendPacket( pkt, (rank+1)%size, RELEASE);
		//     changeState( InRun );
		//     free(pkt);
		// //}
		// break;
		// default:
		// break;
		//     }
		// sleep(SEC_IN_STATE);
		case REST:
			perc = random() % 100;
			packet_t *pkt = malloc(sizeof(packet_t));
			pkt->data = (perc < 50) ? KILLER : VICTIM; // Losowanie roli
			if (pkt->data == KILLER)
			{
				printf("Jestem zabojca ŁAAAA\n");
			}
			else
			{
				printf("Jestem ofiarą.. chu.. sie wylosowalo\n");
			}

			for (int i = 0; i < size; i++)
			{
				if (i != rank)
				{
					sendPacket(pkt, i, MSG_ROLE); // Wysyłanie roli do wszystkich
				}
			}
			changeState((pkt->data == KILLER) ? KILLER : VICTIM); // Zmiana stanu na KILLER lub VICTIM
			break;
		case VICTIM:
			// Victim-specific logic here
			break;
		case KILLER:
			if (ackCount == size - 1)
			{
				changeState(WANNAKILL);
				printf("Wchodze w stan WANNAKILL!\n");
			}
			break;
		case WANNAKILL:
			for (int i = 0; i < size; i++)
			{
				if (i != rank)
				{
					sendPacket(0, i, REQ_KILL); // Wysyłanie roli do wszystkich
				}
			}

			break;
		case KILLING:
			// Wchodzenie do sekcji krytycznej
			pthread_mutex_lock(&student_list_mutex); // Blokowanie dostępu do listy studentow

			if (count > 0)
			{
				packet_t victim;
				for (int i = 0; i < count - 1; i++)
				{
					// wziecie pierwszej ofiary z listy
					if (students_list[i].data == VICTIM)
					{
						victim = students_list[i];
						// usuniecie tej ofiary z listy
						for (int j = i; j < count; j++)
						{
							students_list[j] = students_list[j + 1];
						}
						count--;
						break;
					}
				}
				debug("Paruję się z ofiarą %d\n", victim.src);
				printf("Paruję się z ofiarą %d\n", victim.src);

				// losowanie wyniku starcia
				double result = (double)rand() / RAND_MAX;
				if (result > 0.5)
				{
					debug("Wygrałem starcie z %d\n", victim.src);
					printf("Wygrałem starcie z %d\n", victim.src);
					// Możesz dodać dodatkową logikę dla wygranej
				}
				else
				{
					debug("Przegrałem starcie z %d\n", victim.src);
					printf("Przegrałem starcie z %d\n", victim.src);
					// Możesz dodać dodatkową logikę dla przegranej
				}
			}

			pthread_mutex_unlock(&student_list_mutex); // Odblokowanie dostępu do listy studentów

			break;

		case ITS_OVER:
			// ITS_OVER-specific logic here
			break;
		default:
			break;
		}
		sleep(SEC_IN_STATE);
	}
}
