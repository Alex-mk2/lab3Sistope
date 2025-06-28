#include "funciones.h"
#include <bits/pthreadtypes.h>
#include <pthread.h>
#include <stdio.h>
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
    pthread_mutex_lock(&mutex_tiempo);
    proceso->tiempo_en_cola = tiempo_global;
    pthread_mutex_unlock(&mutex_tiempo);

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

    //Procesos disponibles
    pthread_cond_signal(&cola->cond);
    pthread_mutex_unlock(&cola->mutex);

}

//Descripcion: Funcion que permite desencolar un proceso
//Dom: Cola
//Rec: proceso

Proceso * desencolar(Cola * cola){

    //Mutex se bloquea el proceso
    pthread_mutex_lock(&cola->mutex);

    while(cola->tamano == 0 && senal){
        //Se realiza la espera
        pthread_cond_wait(&cola->cond, &cola->mutex);

    }

    if(!senal && cola->tamano == 0) {
        pthread_mutex_unlock(&cola->mutex);
        return NULL;
    }

    Nodo * nuevo = cola->frente;
    Proceso * proceso = nuevo->proceso;
    cola->frente = nuevo->siguiente;

    //Proceso desencolado
    if(cola->frente == NULL){
        cola->final = NULL;
    }

    free(nuevo);
    cola->tamano--;
    //Se libera mutex
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
    while(senal){
        Proceso * proceso = quitar_bloqueados(cola_bloqueados);
        if(proceso != NULL){
            int tiempo_io = 100 + rand() % 401;

            
            //Se duerme el SO por el tiempo de i/o
            usleep(tiempo_io * 1000);
            pthread_mutex_lock(&mutex_tiempo);
            tiempo_global += tiempo_io;
            pthread_mutex_unlock(&mutex_tiempo);


            //Se encola el proceso
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

    //Esto lo controla una variable global declarada en funciones.h
    while(senal){
        Proceso * proceso = desencolar(cola_proc);
        int tiempo_ejecucion;
        
        //Si el proceso esta vacio
        if(proceso == NULL) {
            break; 
        }

        //Si el proceso se ejecuta por primera vez
        if(proceso->ejecutado == 0){
            pthread_mutex_lock(&mutex_tiempo);
            proceso->primera_ejecucion = tiempo_global;
            double tiempo_espera_inicial = (tiempo_global - proceso->tiempo_llegada);
            pthread_mutex_unlock(&mutex_tiempo);
            estadisticas_proc[nucleo].tiempo_espera_acum_proc += tiempo_espera_inicial;
            proceso->ejecutado = 1;
        }

        //Se revisa si el tiempo es menor al quatum
        if(proceso-> tiempo_restante < quantum){
            tiempo_ejecucion = proceso->tiempo_restante;
        }else{
            tiempo_ejecucion = quantum;
        }


        //Se utiliza mutex para proteger el tiempo de ejecucion
        usleep(1000 * tiempo_ejecucion);
        pthread_mutex_lock(&mutex_tiempo);
        tiempo_global += tiempo_ejecucion;
        pthread_mutex_unlock(&mutex_tiempo);
        proceso->tiempo_restante-= tiempo_ejecucion;


        //Para que sea procesado e imprimido por la funcion de estadisticas y manejado por su estructura 
        //Creado en funciones.h
        estadisticas_proc[nucleo].tiempo_ocupado += tiempo_ejecucion; 
        estadisticas_proc[nucleo].num_procesos_ejecutados++;
        

        //Se toma un valor aleatorio, y en base a eso se toma siempre se cumpla que sea mayor a 0 y menor en probabilidad
        double valor_aleatorio = (rand() / (double)RAND_MAX);
        if(proceso->tiempo_restante > 0 && valor_aleatorio < probabilidad){
            int tiempo_io = 100 + rand() % 401;


            //Se registra el tiempo bloqueado y el numero de procesos bloqueados
            estadisticas_proc[nucleo].tiempo_proc_bloqueado += tiempo_io; 
            estadisticas_proc[nucleo].num_procesos_bloqueados++;
            proceso->esta_bloqueado = 1;


            //Agregar a la cola de bloqueados
            agregar_bloqueados(proceso, colaBloqueados);


        }else if(proceso->tiempo_restante > 0){
            //Se encola el proceso
            encolar(proceso, cola_proc);
        
        
        }else{
            //Si termina su tiempo
            destruir_proceso(proceso);
        }

    }
    return NULL;
}


//Descripcion: Funcion que permite destruir una cola
//Dom: Cola 
//Rec: void

void destruir_cola(Cola * cola){
    while(cola->tamano > 0){
        Proceso * proceso = desencolar(cola);
        destruir_proceso(proceso);
    }
    pthread_mutex_destroy(&cola->mutex);
    pthread_cond_destroy(&cola->cond);
    free(cola);
}



//Descripcion: Funcion para crear el proceso
//Dom: pid X tiempo_servicio X tiempo_llegada
//Rec: Proceso(estructura)

Proceso * crear_proceso(int pid, int tiempo_servicio, int tiempo_llegada){
    Proceso * proceso = (Proceso *)malloc(sizeof(Proceso));
    proceso->pid = pid;
    proceso->tiempo_llegada = tiempo_llegada;
    proceso->tiempo_servicio = tiempo_servicio;
    proceso->tiempo_restante = tiempo_servicio;
    proceso->esta_bloqueado = 0;
    proceso->tiempo_en_cola = 0.0;
    proceso->ejecutado = 0;
    proceso->primera_ejecucion = 0.0;
    return proceso;
}


//Descripcion: Funcion que permite terminar la ejecucion de una hebra
//Dom: hebras X num_hebras
//Rec: void

void finalizar_hebras(pthread_t * hebras, int num_hebras){

    //Variable que esta en funciones h
    senal = 0;

    //Se liberan o despiertan las hebras
    pthread_cond_broadcast(&cola_proc->cond);

    //Esperar al termino de todas las hebras
    int i;
    for(i = 0; i < num_hebras; i++){
        pthread_join(hebras[i], NULL);
    }
}

//Descripcion: Funcion para leer el archivo de entrada
//Dom: archivo
//Rec: void

void leer_archivo(char * archivo){

    FILE * file = fopen(archivo, "r");
    if(file == NULL){
        printf("Error: No se pudo abrir el archivo %s\n", archivo);
        exit(EXIT_FAILURE);
    }

    int pid, tiempo_llegada, tiempo_servicio;
    int ultimo_en_llegar = 0;


    while(fscanf(file, "%d,%d,%d", &pid, &tiempo_llegada, &tiempo_servicio) == 3){
        int diferencia = tiempo_llegada - ultimo_en_llegar;
        if(diferencia > 0){
            usleep(diferencia * 1000);
            pthread_mutex_lock(&mutex_tiempo);
            tiempo_global+= diferencia;
            pthread_mutex_unlock(&mutex_tiempo);
        }


        ultimo_en_llegar = tiempo_llegada;
        Proceso * proceso = crear_proceso(pid, tiempo_servicio, tiempo_llegada);
    

        pthread_mutex_lock(&mutex_tiempo);
        proceso->tiempo_en_cola = tiempo_llegada;
        pthread_mutex_unlock(&mutex_tiempo);
        encolar(proceso, cola_proc);
    }
    fclose(file);
}


//Descripcion: Funcion que permite destruir la cola de bloqueados
//Dom: Bloqueados
//Rec: void

void destruir_colaBloqueados(Bloqueados * bloqueados){
    while(bloqueados->tamano > 0){
        Proceso * proceso = quitar_bloqueados(bloqueados);
        destruir_proceso(proceso);
    }
    pthread_mutex_destroy(&bloqueados->mutex);
    free(bloqueados);
}


//Descripcion: Funcion para mostrar la estadisticas por proceso
//Dom: num_procesadores X tiempo_ejecucion
//Rec: void

void mostrar_estadisticas(int num_procesadores, int tiempo_ejecucion){
    int i;
    for(i = 0; i < num_procesadores;i++){
        printf("\nNucleos en uso %d: ", i);
        printf("\nCantidad de procesos ejecutados: %d", estadisticas_proc[i].num_procesos_ejecutados);


        //Formula de la utilizacion
        double utilizacion = (estadisticas_proc[i].tiempo_ocupado / tiempo_ejecucion) * 100.0;
        printf("\nLa utilizacion es: %f\n", utilizacion);


        //Tiempo de espera promedio por proceso
        if(estadisticas_proc[i].num_procesos_ejecutados > 0){
            double tiempo_espera_prom = (estadisticas_proc[i].tiempo_espera_acum_proc / estadisticas_proc[i].num_procesos_ejecutados);
            printf("\nTiempo promedio de espera por proceso: %f\n", tiempo_espera_prom);
        }


        //Tiempo de procesos bloqueados
        if(estadisticas_proc[i].num_procesos_bloqueados > 0) {
            double tiempo_bloqueo_promedio = estadisticas_proc[i].tiempo_proc_bloqueado / estadisticas_proc[i].num_procesos_bloqueados;
            printf("Tiempo promedio de bloqueo: %.2f ms\n", tiempo_bloqueo_promedio);
        } else {
            printf("Tiempo promedio de bloqueo: 0.00 ms\n");
        }
    }
}
    



//Descripcion: Funcion que inicia las estadisticas por proceso
//Dom: num_procesadores
//Rec: void

void iniciar_estadisticas(int num_procesadores){
    int i;
    estadisticas_proc = (Informacion *)malloc(num_procesadores * sizeof(Informacion));
    for(i = 0; i < num_procesadores; i++){
        estadisticas_proc[i].num_procesos_ejecutados = 0;
        estadisticas_proc[i].tiempo_ocupado = 0.0;
        estadisticas_proc[i].tiempo_espera_acum_proc = 0.0;
        estadisticas_proc[i].tiempo_proc_bloqueado = 0.0;
        estadisticas_proc[i].num_procesos_bloqueados = 0;
    }
}

//Descripcion: Funcion que permite el parseo de argumentos (no mencionan si es con geto)
//Dom: argc X argv X archivo X num_proceso X quatum X prob
//Rec: void

void parseo_argumentos(int argc, char * argv[], char ** archivo, int * num_proceso, float * quantum, float * prob){
    int i;
    for(i = 1; i < argc; i++){
         if (strcmp(argv[i], "--data") == 0 && i + 1 < argc) {
            *archivo = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "--numproc") == 0 && i + 1 < argc) {
            *num_proceso = atoi(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--quantum") == 0 && i + 1 < argc) {
            *quantum = atof(argv[i + 1]);
            i++;
        } else if (strcmp(argv[i], "--prob") == 0 && i + 1 < argc) {
            *prob = atof(argv[i + 1]);
            i++;
        }
    }
}

