//Nombre: Diana Ramírez Neira
//Proyecto Yuyai-Artificial
//Compilar: gcc -o proyecto proyecto.c -lpthread -lm

#define _GNU_SOURCE

#include<stdio.h>
#include<string.h>
#include<pthread.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<math.h> 
#include<sys/time.h>
#include<semaphore.h>
#include<sched.h>

#define DATA_SIZE 505
#define TASK_QUEUE_SIZE 1024
#define LINE_SIZE 1024
#define EPSILON_SIZE 10
#define N_SIZE 1024

//time
struct timeval current_time;
struct timeval current_time_final;
long int total_time;


//Estructura con los valores de la recta AX+BY+C para calcular la distancia
typedef struct result_recta {
    double* a;
    double* b;
    double* c;
}result_recta;

//Estructura para el par (cuartos,precio)
typedef struct pair_cuartos_precio {
    double* cuartos;
    double* precio;
} pair_cuartos_precio;

//Estructura que define la tarea a almacenar en la recta
typedef struct Task{ 
    void (*taskFunction)(int i); 
    int i;
} Task;

//Estructura que almacena la linea del modelo, los puntos con los que se la obtuvo y la tasa.
typedef struct rectas_puntos_tasas{
    result_recta* recta;
    pair_cuartos_precio* cuartos_precio_1;
    pair_cuartos_precio* cuartos_precio_2;
    double* tasa;
}rectas_puntos_tasas;

//Semáforos para el control del ingreso o salida de tareas en la lista de tareas
sem_t empty_task_queue;
sem_t full_task_queue;
sem_t mtx;

//Declaración de variables de estructuras
pair_cuartos_precio* arreglo_pares;
rectas_puntos_tasas* recta_puntos_tasa;

//Se crea la cola de tareas
Task taskQueue[TASK_QUEUE_SIZE];
int in_taskCount = 0;
int out_taskCount = 0;

//Control del número de repeticiones de los hilos de acuerdo con el número de tareas
int num_task = 0;

//Puntero al Valor Epsilon
double* p_epsilon;

//Puntero al valor n
int* p_n;

//Declaración de funciones a utilizarse
const char* getfield(char* line, int num);
pair_cuartos_precio* random_generator(pair_cuartos_precio *pairs_random);
void submitTask(Task task);
void *runner(void *param);
void executeTask(Task* task);
int cmpfunc (const void * a, const void * b);
int nprocs();
void realizarCalculos( int i);


int main(int argc, char* argv[]){

    char comandoIngresado[N_SIZE];
    char comandoEpsilon[EPSILON_SIZE];
    int i=0;
    int len_n;
    int len_e;
    char line[LINE_SIZE];
    double e;
    int n;

    //Se asigna el número de hilos
    int thread_num=nprocs();

    //Se reserva memoria para epsilon y n
    p_epsilon = (double*) calloc(1, sizeof(double));
    p_n = (int*) calloc(1, sizeof(int));

    // Hilos
    pthread_t tida[thread_num];
    pthread_attr_t attr;

    pthread_attr_init(&attr);

    //Se lee el archivo y se almacenan los elementos en un arreglo de pares

    FILE* stream = fopen("assets/boston.csv", "r");
    
    arreglo_pares =malloc(DATA_SIZE*sizeof(pair_cuartos_precio)); //ATENCION BORRADO *
    for(int j=0;j<DATA_SIZE;j++)
    {
      arreglo_pares[j].cuartos = (double*) calloc(1, sizeof(double));
      arreglo_pares[j].precio = (double*) calloc(1, sizeof(double));
      
    }


    fgets(line, LINE_SIZE, stream);
    while (fgets(line, LINE_SIZE, stream))
    {
        
        char *tmp1 = strdup(line);
        char *tmp2 = strdup(line);
        pair_cuartos_precio *par=(pair_cuartos_precio*)malloc(sizeof(pair_cuartos_precio));

        double f1=atof(getfield(tmp1, 1));
        double f2=atof(getfield(tmp2, 2));

        par->cuartos = (double*) calloc(1, sizeof(double));
        par->precio = (double*) calloc(1, sizeof(double));
        
        *par->cuartos=f1;
        *par->precio=f2;
        
        arreglo_pares[i].cuartos=par->cuartos;
        arreglo_pares[i].precio=par->precio;

        free(tmp1);
        free(tmp2);
        i=i+1;
    }

    //Ingreso y validación de datos
    printf("Ingrese el valor de valor de n: ");
    fgets(comandoIngresado,N_SIZE,stdin);
    len_n = strlen(comandoIngresado);
    if (len_n > 0 && comandoIngresado[len_n-1] == '\n') {
        comandoIngresado[len_n-1] = '\0';
    }
    n=atoi(comandoIngresado);
            
    while(n<=0 || fmod(atof(comandoIngresado),(n/1.0))>0 || strstr(comandoIngresado,",") ){
        printf("Ingrese un número válido, un entero mayor a 0: \n");
        printf("Ingrese n: ");
        fgets(comandoIngresado,N_SIZE,stdin);
        n=atoi(comandoIngresado);
    }
    *p_n=n;


    printf("Ingrese valor de e: ");
    fgets(comandoEpsilon,EPSILON_SIZE,stdin);
    len_e = strlen(comandoEpsilon);
    if (len_e > 0 && comandoEpsilon[len_e-1] == '\n') {
        comandoEpsilon[len_e-1] = '\0';
    }
    e=atof(comandoEpsilon);

    while((e<=0.000000) || strstr(comandoEpsilon,",")){
        printf("Ingrese un número mayor a 0: \n");
        printf("Ingrese el valor de e: ");
        fgets(comandoEpsilon,EPSILON_SIZE,stdin);
        e=atof(comandoEpsilon);
    }
    *p_epsilon=e;

    //Inicialización de semáforos
    sem_init(&mtx, 0, 1);
    sem_init(&empty_task_queue, 0, n);
    sem_init(&full_task_queue, 0, 0);

    //Reserva de memoria para almacenar una lista con las lineas del modelo, los puntos con los que se las obtuvo y las tasas por cada iteración.
    recta_puntos_tasa = malloc(n* sizeof(rectas_puntos_tasas));    

    for(i=0;i<n;i++)
    {
      recta_puntos_tasa[i].recta=(result_recta*)calloc(1,sizeof(result_recta));
      recta_puntos_tasa[i].cuartos_precio_1=(pair_cuartos_precio*)calloc(1,sizeof(pair_cuartos_precio));
      recta_puntos_tasa[i].cuartos_precio_2=(pair_cuartos_precio*)calloc(1,sizeof(pair_cuartos_precio));
      recta_puntos_tasa[i].tasa=(double*) calloc(1, sizeof(double));
    }


    for(i=0; i<thread_num; i++){

        if(pthread_create(&tida[i],&attr,runner,NULL) !=0){//comandoIngresado
            perror("Fallo al crear el hilo");
        }
    }

    srand(time(0));

    //          #######INICIO#######


    for(i=0; i<n; i++){
        //Se crean las n tareas con la función a ejecutar y el número de tarea que es el índice en los arreglos
        Task t = {
            .taskFunction = &realizarCalculos,
            .i = i
        };
        submitTask(t);
    }
    
    //Esperar a que el hilo termine
    for(i=0; i<thread_num; i++){
        if(pthread_join(tida[i],NULL) != 0){
            perror("Failed to join thread\n");
        }
    }

    //Se ordenan las tasas obtenidas
    qsort(recta_puntos_tasa, n, sizeof(rectas_puntos_tasas), cmpfunc);//creada con los puntos x=%f y y=%f


    printf("El valor de la tasa mayor es %f, perteneciente a la recta Y = %fX + %f creada con los puntos X1 = %f y Y1 = %f y X2 = %f y Y2 = %f \n",*recta_puntos_tasa[n-1].tasa
    , -(*recta_puntos_tasa[n-1].recta->a), (-*recta_puntos_tasa[n-1].recta->c),
    *recta_puntos_tasa[n-1].cuartos_precio_1->cuartos, *recta_puntos_tasa[n-1].cuartos_precio_1->precio,
        *recta_puntos_tasa[n-1].cuartos_precio_2->cuartos, *recta_puntos_tasa[n-1].cuartos_precio_2->precio);

    //          #######FIN######

    free(recta_puntos_tasa);
    free(p_epsilon);
    free(p_n);
    return 0;

}


// FUNCIONES

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
            tok && *tok;
            tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}

/**
 * Función que obtiene el número de procesadores para asignar la cantidad de hilos.
 */
int nprocs()
{
  cpu_set_t cpu;
  CPU_ZERO(&cpu);
  sched_getaffinity(0, sizeof(cpu), &cpu);
  return CPU_COUNT(&cpu);
}


/**
 * Función que genera un par (cuartos,precio) de forma aleatoria.
 */
pair_cuartos_precio* random_generator(pair_cuartos_precio *pairs_random)
{
    int upper=DATA_SIZE, lower=0, random_n1,random_n2;
    random_n1= (int) (rand() % (upper-lower+1))+lower;

    pairs_random->cuartos = (double*) calloc(1, sizeof(double));
    pairs_random->precio = (double*) calloc(1, sizeof(double));

    *pairs_random->cuartos=*arreglo_pares[random_n1].cuartos;
    *pairs_random->precio=*arreglo_pares[random_n1].precio;
    return pairs_random;
}

/**
 * Función que agrega una tarea al arreglo de tareas para que sea ejecutada.
 */
void submitTask(Task task)
{
    sem_wait(&empty_task_queue);
    sem_wait(&mtx);

    taskQueue[in_taskCount] = task;
    in_taskCount=(in_taskCount+1)%TASK_QUEUE_SIZE;
    
    sem_post(&mtx);
    sem_post(&full_task_queue);
}


/**
 * Función que espera a que hayan tareas en el arreglo de tareas, luego las obtiene y llama a la función de ejecución
 */
void *runner(void *param)
{

    while(num_task<(*p_n)){
        Task task;
        num_task=num_task+1;
        sem_wait(&full_task_queue);
        sem_wait(&mtx);

        task = taskQueue[out_taskCount];
        out_taskCount=(out_taskCount+1)%TASK_QUEUE_SIZE;
       
        sem_post(&mtx);
        sem_post(&empty_task_queue);
        
        executeTask(&task);
    }

}


/**
 * Función que inicia la ejecución de una tarea llamando a taskFunction.
 */
void executeTask(Task* task)
{ 
    task -> taskFunction(task -> i); 
} 

/**
 * Función de comparación para ordenar las tasas.
 */
int cmpfunc (const void * a, const void * b)
{
    const rectas_puntos_tasas *rA = a;
    const rectas_puntos_tasas *rB = b;
    double tasaA = *rA->tasa;
    double tasaB = *rB->tasa;
  if (tasaA > tasaB)
    return 1;
  else if (tasaA < tasaB)
    return -1;
  else
    return 0;  
}

/**
 * Función que de acuerdo a un par de puntos obtiene la linea del modelo y=mx+b, calcula la distancia de los puntos a dicha línea y obtiene la tasa.
 */
void realizarCalculos( int i)
{ 
    int inliers =0;
    int outliers= 0;
    double tasa;

    //Se generan los arreglo_pares aleatorios
    pair_cuartos_precio *get_pairs_random1=(pair_cuartos_precio*)malloc(sizeof(pair_cuartos_precio));
    pair_cuartos_precio *get_pairs_random2=(pair_cuartos_precio*)malloc(sizeof(pair_cuartos_precio));

    random_generator(get_pairs_random1);
    random_generator(get_pairs_random2);

    //Se obtiene la pendiente
    double m = (*get_pairs_random2->precio - *get_pairs_random1->precio) / (*get_pairs_random2->cuartos - *get_pairs_random1->cuartos);

    //Se obtiene el valor de b
    double b = ((-m)*(*(get_pairs_random1->cuartos))) + (*get_pairs_random1->precio);

    //Se crea la estructura para almacenar la línea del modelo
    result_recta *recta=(result_recta*)malloc(sizeof(result_recta));
    recta->a = (double*) calloc(1, sizeof(double));
    recta->b = (double*) calloc(1, sizeof(double));
    recta->c = (double*) calloc(1, sizeof(double));

    *recta->a = (-m);
    *recta->b = (1.0);
    *recta->c = (-b);

    //Se almacenan los datos de la linea del modelo en la estructura y el arreglo
    recta_puntos_tasa[i].recta->a=(double*) calloc(1, sizeof(double));
    recta_puntos_tasa[i].recta->b=(double*) calloc(1, sizeof(double));
    recta_puntos_tasa[i].recta->c=(double*) calloc(1, sizeof(double));
    recta_puntos_tasa[i].recta->a=recta->a;
    recta_puntos_tasa[i].recta->b=recta->b;
    recta_puntos_tasa[i].recta->c=recta->c;
    //Se almacenan los puntos usados para obtener la linea en la estructura y en el arreglo
    recta_puntos_tasa[i].cuartos_precio_1->cuartos=(double*) calloc(1, sizeof(double));
    recta_puntos_tasa[i].cuartos_precio_1->precio=(double*) calloc(1, sizeof(double));

    recta_puntos_tasa[i].cuartos_precio_2->cuartos=(double*) calloc(1, sizeof(double));
    recta_puntos_tasa[i].cuartos_precio_2->precio=(double*) calloc(1, sizeof(double));

    recta_puntos_tasa[i].cuartos_precio_1->cuartos=get_pairs_random1->cuartos;
    recta_puntos_tasa[i].cuartos_precio_1->precio=get_pairs_random1->precio;

    recta_puntos_tasa[i].cuartos_precio_2->cuartos=get_pairs_random2->cuartos;
    recta_puntos_tasa[i].cuartos_precio_2->precio=get_pairs_random2->precio;
    

    //Se obtienen lo inliers, outliers, se calcula la tasa y se almacena en la estructura y el arreglo
    for(int j=0; j<DATA_SIZE; j++){
        double numerador = ((*recta->a * *arreglo_pares[j].cuartos) + (*recta->b * *arreglo_pares[j].precio) + (*recta->c));
        double denominador = sqrt(((*recta->a) * (*recta->a)) + ((*recta->b) * (*recta->b)));
        double distancia = fabs(numerador/denominador);
        //printf("DISTANCIA %f - EPSILON %f \n", distancia, *p_epsilon);
        if(distancia<=*p_epsilon){
            inliers=inliers+1;
        }else{
            outliers=outliers+1;
        }

    }
    //printf("inliers %d \n", inliers);
    //printf("outliers %d \n", outliers);
    tasa=(double)inliers / (double)(inliers+outliers);
    *recta_puntos_tasa[i].tasa=tasa;

    //Se muestra el avance de la ejecución del sistema.
    printf("Se ejecutó la tarea %d, se obtuvo la tasa %f perteneciente a la recta Y = %fX + %f creada con los puntos X1 = %f y Y1 = %f y X2 = %f y Y2 = %f \n",
    i+1, *recta_puntos_tasa[i].tasa,
        -(*recta_puntos_tasa[i].recta->a), (-*recta_puntos_tasa[i].recta->c),
        *recta_puntos_tasa[i].cuartos_precio_1->cuartos, *recta_puntos_tasa[i].cuartos_precio_1->precio,
        *recta_puntos_tasa[i].cuartos_precio_2->cuartos, *recta_puntos_tasa[i].cuartos_precio_2->precio);
}