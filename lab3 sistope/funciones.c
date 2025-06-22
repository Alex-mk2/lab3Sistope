#include "funciones.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>






//Descripcion: Funcion que permite iniciar una cola (se utiliza lista simple)
//Dom: void 
//Rec: puntero cola

Cola * iniciar_cola(){
    Cola * cola = (Cola*)malloc(sizeof(Cola));
    cola-> frente = NULL;
    cola->final = NULL;

    //Se inicia el mutex en la cola
    pthread_mutex_init(&cola->mutex, NULL);

    //Se bloquea a la espera de la condicion
    pthread_cond_init(&cola->cond, NULL);

    //Tamaño de la cola (inicia en 0)
    cola->tamano = 0;

    return cola;

}

//Descripcion: Funcion que permite encolar un proceso
//Dom: Cola X proceso
//Rec: void

void encolar(Proceso * proceso, Cola * cola){

    //Se bloquea una vez que obtenga el mutex
    pthread_mutex_lock(&cola->mutex);

    //Se crea un nuevo proceso 
    Nodo * nuevo_p = (Nodo*)malloc(sizeof(Nodo));
    nuevo_p->proceso = proceso;

    nuevo_p->siguiente = NULL;

    if(cola->final == NULL){
        cola->frente = nuevo_p;

    }else{
        cola->final->siguiente = nuevo_p;
    }
    cola->final = nuevo_p;
    cola->tamano++;

    //Procesos disponibles//
    pthread_cond_signal(&cola->cond);
    pthread_mutex_unlock(&cola->mutex);

}

//Descripcion: Funcion que permite desencolar un proceso
//Dom: Cola
//Rec: proceso

Proceso * desencolar(Cola * cola){

    //Mutex se bloquea el proceso//
    pthread_mutex_lock(&cola->mutex);

    while(cola->tamano == 0){
        //Se realiza la espera//
        pthread_cond_wait(&cola->cond, &cola->mutex);

    }

    Nodo * nuevo = cola->frente;
    Proceso * proceso = nuevo->proceso;
    cola->frente = nuevo->siguiente;

    //Proceso desencolado//
    if(cola->frente == NULL){
        cola->final = NULL;
    }

    free(nuevo);
    cola->tamano--;
    //Se libera mutex//
    pthread_mutex_unlock(&cola->mutex);
    return proceso;
}


//Descripcion: Funcion que permite destruir un proceso que ha sido creado
//Dom: Proceso 
//Rec: void

void destruir_proceso(Proceso * proceso){
    free(proceso);
}


//Descripcion: Funcion que permite crear una cola de bloqueados
//Dom: void
//Rec: Cola bloqueados

Bloqueados * cola_bloqueados(){
    Bloqueados * bloqueados = (Bloqueados *)malloc(sizeof(Bloqueados));
    bloqueados->frente = NULL;
    bloqueados->final = NULL;
    pthread_mutex_init(&bloqueados->mutex, NULL);
    bloqueados->tamano = 0;
    return bloqueados;
}

//Descripcion: Funcion que permite agregar procesos bloqueados a la cola de bloqueados
//Dom: Bloqueados X proceso
//Rec: void

void agregar_bloqueados(Proceso * proceso, Bloqueados * bloqueados){

    //Se bloquea el proceso (se agregará a la cola)
    pthread_mutex_lock(&bloqueados->mutex);
    Nodo * nuevo_p = (Nodo*)malloc(sizeof(Nodo));

    nuevo_p->proceso = proceso;
    nuevo_p->siguiente = NULL;

    if(bloqueados->final == NULL){
        bloqueados->frente = nuevo_p;
    }else{
        bloqueados->final->siguiente = nuevo_p;
    }
    bloqueados->final = nuevo_p;
    bloqueados->tamano++;
    pthread_mutex_unlock(&bloqueados->mutex);
}

//Descripcion: Funcion que permite quitar un proceso de la cola de bloqueados
//Dom: Bloqueados
//Rec: proceso quitado de la cola de bloqueados

Proceso * quitar_bloqueados(Bloqueados * bloqueados){
    pthread_mutex_lock(&bloqueados->mutex);
    if(bloqueados->tamano == 0){
        pthread_mutex_unlock(&bloqueados->mutex);
        return NULL;
    }
    Nodo * nuevo_p = bloqueados->frente;
    Proceso * proceso = nuevo_p->proceso;
    bloqueados->frente = nuevo_p->siguiente;

    if(bloqueados->frente == NULL){
        bloqueados->final = NULL;
    }

    free(nuevo_p);
    bloqueados->tamano--;
    pthread_mutex_unlock(&bloqueados->mutex);
    return proceso;
}

//Descripcion: Funcion que permite despertar a un proceso de la lista de bloqueados
//Dom: arg
//Rec: void

void * despertar(void * arg){
    Bloqueados * cola_bloqueados = (Bloqueados*) arg;
    while(1){
        Proceso * proceso = quitar_bloqueados(cola_bloqueados);
        if(proceso != NULL){
            int tiempo_io = 100 + rand() % 401;

            //Se duerme el SO//
            usleep(tiempo_io * 1000);

            //Se encola el proceso//
            encolar(proceso, cola_proc);
        }else{
            usleep(10000);
        }
    }
    return NULL;
}


//Descripcion: Funcion que actua como planificador de procesos
//Dom: arg
//Rec: void

void * planificador_proc(void * arg){
    int nucleo = *(int*)arg;
    while(1){
        Proceso * proceso = desencolar(cola_proc);

        int tiempo_ejecucion;

        if(proceso-> tiempo_restante < quantum){
            tiempo_ejecucion = proceso->tiempo_restante;
        }else{
            tiempo_ejecucion = quantum;
        }
        usleep(1000 * tiempo_ejecucion);
        proceso->tiempo_restante-= tiempo_ejecucion;

        int valor_aleatorio = (rand() / (double)RAND_MAX);
        if(proceso->tiempo_restante > 0 && valor_aleatorio < probabilidad){
            //Proceso bloqueado//
            proceso->esta_bloqueado = 1;

            //Agregar a la cola de bloqueados//
            agregar_bloqueados(proceso, colaBloqueados);

        }else if(proceso->tiempo_restante > 0){
            //Se encola el proceso//
            encolar(proceso, cola_proc);
        }else{
            //Si termina su tiempo//
            destruir_proceso(proceso);
        }

    }
    return NULL;
}
