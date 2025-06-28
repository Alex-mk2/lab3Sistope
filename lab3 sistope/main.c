#include "funciones.h"
#include <time.h>
#include <sys/time.h>  

//Variables globales del programa
Cola * cola_proc = NULL;
Bloqueados* colaBloqueados = NULL;
float quantum = 0.0;
float probabilidad = 0.0;
int senal = 1;
Informacion* estadisticas_proc = NULL;
int tiempo_global = 1;
pthread_mutex_t mutex_tiempo = PTHREAD_MUTEX_INITIALIZER;  
//Fin de variables globales


int main(int argc, char* argv[]) {
    char* archivo = NULL;
    int num_procesadores = 1;
    float quantum_val = 100.0;
    float prob_val = 0.1;
    int i;
    double tiempo_total_simulacion; //Tiempo total de simulacion del programa
    
    //Argumentos por consola, se procesan a traves de parseo
    parseo_argumentos(argc, argv, &archivo, &num_procesadores, &quantum_val, &prob_val);
    
    if(archivo == NULL) {
        printf("Error: Debe especificar --data archivo\n");
        printf("Uso: %s --data archivo --numproc N --quantum Q --prob P\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    printf("Iniciando simulación de round robin\n");
    printf("-Archivo: %s\n", archivo);
    printf("-Núcleos: %d\n", num_procesadores);
    printf("-Quantum: %.1f ms\n", quantum_val);
    printf("-Probabilidad: %.2f\n", prob_val);
    quantum = quantum_val;
    probabilidad = prob_val;
    srand(time(NULL));
    

    //Se inician las estructuras generadas en funciones.h
    cola_proc = iniciar_cola();
    colaBloqueados = cola_bloqueados();
    iniciar_estadisticas(num_procesadores);
    
    //Creacion de hebras de acuerdo al parametro de entrada
    pthread_t* nucleos = malloc(num_procesadores * sizeof(pthread_t));
    pthread_t hebra_desbloqueo;
    pthread_t hebra_lectura;
    int* ids = malloc(num_procesadores * sizeof(int));
    
    //Hebra que desbloquea a las demás
    pthread_create(&hebra_desbloqueo, NULL, despertar, colaBloqueados);
    

    //Hebras que planifican y utilizan la funcion planificador_proc
    for(i = 0; i < num_procesadores; i++) {
        ids[i] = i;
        pthread_create(&nucleos[i], NULL, planificador_proc, &ids[i]);
    }
    
    //Hebra lectura de archivo
    pthread_create(&hebra_lectura, NULL, (void*)leer_archivo, archivo);
    printf("Se han creado las hebras.\n");
    

    //Finalizacion de la hebra de lectura
    pthread_join(hebra_lectura, NULL);
    printf("Se ha leido el archivo de entrada.\n");
    sleep(3);  


    senal = 0;
    pthread_cond_broadcast(&cola_proc->cond);
    usleep(100000);  

    //Esperar a que las hebras terminen la ejecucion y obtener sus resultados
    for(i = 0; i < num_procesadores; i++) {
        pthread_join(nucleos[i], NULL);
    }
    pthread_join(hebra_desbloqueo, NULL);
    
    


    if(tiempo_global > 0){
        tiempo_total_simulacion = tiempo_global;
    }else{
        tiempo_total_simulacion = 10000.0;
    }


    //Se muestran las estadisticas
    mostrar_estadisticas(num_procesadores, tiempo_total_simulacion);
    

    //Se destruyen y liberan los recursos de la cola, de bloqueados, de los mutex
    destruir_cola(cola_proc);
    destruir_colaBloqueados(colaBloqueados);
    free(estadisticas_proc);
    free(nucleos);
    free(ids);
    pthread_mutex_destroy(&mutex_tiempo);
    

    return EXIT_SUCCESS;
}
