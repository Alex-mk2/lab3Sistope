/*Estructura código*/
#ifndef FUNCIONES_H
#define FUNCIONES_H
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

//Estructura para crear un proceso
typedef struct{
    int pid;
    int tiempo_servicio;
    int tiempo_llegada;
    int tiempo_restante;
    int esta_bloqueado; 
    double tiempo_en_cola;
    int ejecutado; //Si se ejecuto por primera vez caso o más de una vez
    double primera_ejecucion;
}Proceso;


//Estructura para mover un nodo (Proceso)
typedef struct Nodo {
    Proceso* proceso;
    struct Nodo * siguiente;
} Nodo;


//Estructura para crear cola
typedef struct{
    Nodo * frente;
    Nodo * final;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int tamano;
}Cola;


//Cola para procesar los bloqueados
typedef struct {
    Nodo * frente;
    Nodo * final;
    pthread_mutex_t mutex;
    int tamano;
}Bloqueados;


//Estructura para las estadisticas
typedef struct{
    int num_procesos_ejecutados;
    double tiempo_ocupado; //Quantum utilizado
    double tiempo_espera_acum_proc; //Tiempo que lleva esperando el proceso
    double tiempo_proc_bloqueado; //Tiempo que ha sido bloqueado el proceso
    int num_procesos_bloqueados; //Cantidad de procesos bloqueados
}Informacion;


//Variables globales para su uso correspondiente
extern Cola * cola_proc;
extern float quantum;
extern float probabilidad;
extern Bloqueados * colaBloqueados;
extern int senal;
extern Informacion * estadisticas_proc;
extern int tiempo_global;
extern pthread_mutex_t mutex_tiempo;
/*Llamado de funciones para que sean ejecutadas a traves del funciones.c*/



//Descripcion: Funcion para leer el archivo de entrada
//Dom: archivo
//Rec: void

void leer_archivo(char * archivo);


//Descripcion: Funcion para crear el proceso
//Dom: pid X tiempo_servicio X tiempo_llegada
//Rec: Proceso(estructura)

Proceso * crear_proceso(int pid, int tiempo_servicio, int tiempo_llegada);

//Descripcion: Funcion que permite destruir un proceso que ha sido creado
//Dom: Proceso 
//Rec: void

void destruir_proceso(Proceso * proceso);

//Descripcion: Funcion que permite iniciar mutex para la cola
//Dom: void
//Rec: void

void mutex_cond();

//Descripcion: Funcion que permite despertar a un proceso de la lista de bloqueados
//Dom: arg
//Rec: void

void * despertar(void * arg);


//Descripcion: Funcion que permite iniciar una cola
//Dom: void 
//Rec: puntero cola

Cola * iniciar_cola();


//Descripcion: Funcion que permite encolar un proceso
//Dom: Cola X proceso
//Rec: void

void encolar(Proceso * proceso, Cola * cola);

//Descripcion: Funcion que permite descolar un proceso
//Dom: Cola
//Rec: proceso

Proceso * desencolar(Cola * cola);

//Descripcion: Funcion que permite crear una cola de bloqueados
//Dom: void
//Rec: Cola bloqueados

Bloqueados * cola_bloqueados();

//Descripcion: Funcion que permite agregar procesos bloqueados a la cola de bloqueados
//Dom: Bloqueados X proceso
//Rec: void

void agregar_bloqueados(Proceso * proceso, Bloqueados * bloqueados);


//Descripcion: Funcion que permite quitar un proceso de la cola de bloqueados
//Dom: Bloqueados
//Rec: void

Proceso * quitar_bloqueados(Bloqueados * bloqueados);


//Descripcion: Funcion que actua como planificador de procesos
//Dom: arg
//Rec: void

void * planificador_proc(void * arg);


//Descripcion: Funcion que permite destruir una cola
//Dom: Cola 
//Rec: void

void destruir_cola(Cola * cola);

//Descripcion: Funcion que permite terminar la ejecucion de una hebra
//Dom: hebras X num_hebras
//Rec: void

void finalizar_hebras(pthread_t * hebras, int num_hebras);

//Descripcion: Funcion que permite destruir la cola de bloqueados
//Dom: Bloqueados
//Rec: void

void destruir_colaBloqueados(Bloqueados * bloqueados);

//Descripcion: Funcion para mostrar la estadisticas por proceso
//Dom: num_procesadores X tiempo_ejecucion
//Rec: void

void mostrar_estadisticas(int num_procesadores, int tiempo_ejecucion);


//Descripcion: Funcion que inicia las estadisticas por proceso
//Dom: num_procesadores
//Rec: void

void iniciar_estadisticas(int num_procesadores);


//Descripcion: Funcion que permite el parseo de argumentos (no mencionan si es con geto)
//Dom: argc X argv X archivo X num_proceso X quatum X prob
//Rec: void

void parseo_argumentos(int argc, char * argv[], char ** archivo, int * num_proceso, float * quantum, float * prob);

#endif 