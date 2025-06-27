#include "funciones.h"
#include <time.h>

//Variables globales del programa
Cola * cola_proc = NULL;
Bloqueados* colaBloqueados = NULL;
float quantum = 0.0;
float probabilidad = 0.0;
int senal = 1;
Informacion* estadisticas_proc = NULL;
//Fin de variables globales


int main(int argc, char* argv[]) {
    char* archivo = NULL;
    int num_procesadores = 1;
    float quantum_val = 100.0;
    float prob_val = 0.1;
    
    //Parseo de argumentos por consola
    parseo_argumentos(argc, argv, &archivo, &num_procesadores, &quantum_val, &prob_val);
    
    if (archivo == NULL) {
        printf("Error: Debe especificar --data archivo\n");
        printf("Uso: %s --data archivo --numproc N --quantum Q --prob P\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    printf("Iniciando simulación...\n");
    printf("- Archivo: %s\n", archivo);
    printf("- Núcleos: %d\n", num_procesadores);
    printf("- Quantum: %.1f ms\n", quantum_val);
    printf("- Probabilidad: %.2f\n", prob_val);
    
    quantum = quantum_val;
    probabilidad = prob_val;
    srand(time(NULL));
    
    //Inicio de estructuras declaradas en funciones.h
    cola_proc = iniciar_cola();
    colaBloqueados = cola_bloqueados();
    iniciar_estadisticas(num_procesadores);
    
    //Se crean las hebras 
    pthread_t* nucleos = malloc(num_procesadores * sizeof(pthread_t));
    pthread_t hebra_desbloqueo;
    pthread_t hebra_lectura;
    int* ids = malloc(num_procesadores * sizeof(int));
    
    //Se crea la hebra maestra que desbloquea los procesos
    pthread_create(&hebra_desbloqueo, NULL, despertar, colaBloqueados);
    
    //Se crean las hebras para la planificacion
    for (int i = 0; i < num_procesadores; i++) {
        ids[i] = i;
        pthread_create(&nucleos[i], NULL, planificador_proc, &ids[i]);
    }
    pthread_create(&hebra_lectura, NULL, (void*)leer_archivo, archivo);
    printf("Hilos creados. Ejecutando simulación...\n");
    
    //Espera resultado lectura
    pthread_join(hebra_lectura, NULL);
    printf("Lectura de procesos completada.\n");
    sleep(5);  


    //Se liberan las hebras
    printf("Terminando simulación...\n");
    senal = 0;
    pthread_cond_broadcast(&cola_proc->cond);
    usleep(100000);
    

    //Se ejecutan los "cores"
    int i;
    for(int i = 0; i < num_procesadores; i++) {
        pthread_join(nucleos[i], NULL);
    }
    pthread_join(hebra_desbloqueo, NULL);
    printf("\nSimulación completada. Mostrando estadísticas:\n");
    mostrar_estadisticas(num_procesadores, 10000);
    
    
    destruir_cola(cola_proc);
    destruir_colaBloqueados(colaBloqueados);
    free(estadisticas_proc);
    free(nucleos);
    free(ids);
    

    printf("\nPrograma finalizado correctamente.\n");
    return EXIT_SUCCESS;
}
