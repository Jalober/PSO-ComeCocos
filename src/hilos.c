#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#ifndef PTHREAD
#define PTHREAD
	#include <pthread.h>
#endif
#include "estructuras.h"
#include "fantasma.h"
#include "jugador.h"
#include "hilos.h"

#define TRUE 1
#define FALSE 0
#define SENIAL SIGALRM

/*
*	Funcion de inicio del hilo de jugador
*	Descripcion: llama y gestiona la temporizacion de la logica
*		del jugador
*	Parametros: estructura jugador
*	Return: nada
*/
void * hiloJugador (void * pjugador) {
	JUGADOR jug = 	{	((JUGADOR *) pjugador)->id, 
						{ 	((JUGADOR *) pjugador)->controles.arriba,
						  	((JUGADOR *) pjugador)->controles.abajo,
						  	((JUGADOR *) pjugador)->controles.derecha,
						  	((JUGADOR *) pjugador)->controles.izquierda,
						},  
						((JUGADOR *) pjugador)->mapa };

	char  tablaControles[4] = 	{ 	jug.controles.derecha, 
									jug.controles.izquierda,
									jug.controles.arriba,
									jug.controles.abajo
								} ;

	sigset_t set; 
	int sig;
	manejaSenial(&set); 
	
	while(1) {						
		jugador(jug.mapa, 1, tablaControles);
		sigwait(&set, &sig);
	}
	pthread_exit(NULL);
}

/*
*	Funcion de inicio del hilo de fantasma
*	Descripcion: llama y gestiona la temporizacion de la logica
*		del fantasma
*	Parametros: estructura fantasma
*	Return: nada
*/
void * hiloFantasma (void * pfantasma)  {
	FANTASMA fant = { ((FANTASMA *) pfantasma)->id, ((FANTASMA *) pfantasma)->mapa }; 	
	sigset_t set; 
	int sig;
	manejaSenial(&set); 
	int cla = 0;
	while(1) {		
		sigwait(&set, &sig);
		fantasma(fant.mapa, fant.id, &cla);
	}
	pthread_exit(NULL);
} 

/*
*	Funcion creaHilos
*	Descripcion: crea los hilos de los jugadores y los fantasmas
*	Parametros: puntero a estructura MAPA y puntero a estructura HILOS
*	Return: si ha habido error de memoria devuelve TRUE, en caso contrario
*		devuelve FALSE
*/
int creaHilos(MAPA * mapa, HILOS * hilos) {

	int i;	
	int error = FALSE;
	JUGADOR * pjugador = NULL;
	JUGADOR jugador = {0, {'w', 's', 'd', 'a'}, mapa};
	FANTASMA * pfantasma = NULL;
	FANTASMA fantasma = {0, mapa};

	if (NULL == (pjugador = calloc(mapa->numJugadores, sizeof(JUGADOR)))) {
		error = TRUE;
		printf("error de memoria\n");
	} else if (NULL == (pfantasma = calloc(mapa->numFantasmas, sizeof(FANTASMA)))) {
		error = TRUE;
		printf("error de memoria\n");
	} else if (NULL == (hilos->jugador = calloc(mapa->numJugadores, sizeof(pthread_t)))) {
		error = TRUE;
		printf("error de memoria\n");
	} else if (NULL == (hilos->fantasma = calloc(mapa->numFantasmas, sizeof(pthread_t)))) {
		error = TRUE;
		printf("error de memoria\n");		
	} else {
		for(i = 0; i < mapa->numJugadores; i++) {
			pjugador[i] = jugador;
			pjugador[i].id = i;
			pthread_create(&(hilos->jugador[i]), NULL, hiloJugador,  &pjugador[i]);
		}
		for(i = 0; i < mapa->numFantasmas; i++) {
			pfantasma[i] = fantasma;
			pfantasma[i].id = i;
			pthread_create(&(hilos->fantasma[i]), NULL, hiloFantasma, &pfantasma[i]);
		}
	}
	if(error == TRUE) {
		liberaHilos(hilos);
	}
	if(NULL != pjugador) {
		free(pjugador);
		pjugador = NULL;
	}
	if(NULL != pfantasma) {
		free(pfantasma);
		pfantasma = NULL;
	}
	return error;
}

/*
*	Funcion liberaHilos
*	Descripcion: libera la memoria reservada para almacenar los identificadores
*		de los hilos.
*	Parametros: HILOS (puntero)
*	Return: nada
*/
void liberaHilos(HILOS * hilos) {
	
	if(NULL != hilos->jugador) {
		free(hilos->jugador);
		hilos->jugador = NULL;
	}
	if(NULL != hilos->fantasma) {
		free(hilos->fantasma);
		hilos->fantasma = NULL; 
	}
}

/*
*	Funcion enviaSenial
*	Descripcion: envia una senial a todos los hilos creados por la funcion creaHilos
*	Parametros: estructura MAPA y estructura HILOS, habiendose llamado anteriormente 
*		a la funcion creaHilos.
*	Return: nada
*/
void enviaSenial(MAPA mapa, HILOS hilos) {
	int i;
	for(i = 0; i < mapa.numJugadores; i++) {
		pthread_kill(hilos.jugador[i], SENIAL);
	}
	for(i = 0; i < mapa.numFantasmas; i++) {
		pthread_kill(hilos.fantasma[i], SENIAL);
	}	
}
/*
*	Funcion manejaSenial
*	Descripcion: configura la captura de las seniales en los hilos.
*	Parametros: sigset_t * set
*	Return: nada
*/
void manejaSenial(sigset_t * set) {
	sigemptyset(set);
	sigaddset(set, SENIAL); 
}