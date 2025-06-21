/*Estructura código*/
#include <stdio.h>
#include <pthread.h>

//Estructura para crear un proceso//
typedef struct{
    int pid;
    int tiempo_servicio;
    int tiempo_llegada;
    int tiempo_restante;
    int esta_bloqueado; 
}Proceso;


//Estructura para crear una cola//
typedef struct Nodo {
    Proceso* proceso;
    struct Nodo * siguiente;
} Nodo;

//Estructura para crear cola//
typedef struct{
    Nodo * frente;
    Nodo * final;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int tamano;

}Cola;



/*Llamado de funciones para que sean ejecutadas a traves del funciones.c*/



//Descripcion: Funcion para leer el archivo de entrada
//Dom: archivo
//Rec: void

void leer_archivo(char * archivo);


//Descripcion: Funcion para crear el proceso
//Dom: pid X tiempo_servicio X tiempo_llegada
//Rec: Proceso(estructura)

int crear_proceso(int pid, int tiempo_servicio, int tiempo_llegada);

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

void despertar(void * arg);

//Descripcion: Funcion para inicializar las hebras
//Dom: arg
//Rec: void

void iniciar_hebras(void * arg);

//Descripcion: Funcion que permite almacenar procesos en cola
//Dom: void 
//Rec: puntero cola

void iniciar_cola();



