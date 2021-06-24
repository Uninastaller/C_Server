#include <sys/socket.h>  
#include <netinet/in.h>  
#include <sys/types.h>
#include <stdio.h>  
#include <string.h> 
#include <stdlib.h>  
#include <arpa/inet.h> 
#include <unistd.h> 
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#define DEFAULT_BUFLEN 80
#define PI 3.14159265358979
#include <time.h>

sem_t semafor;

int port;
char *IP;

void sig(){
    sem_post(&semafor);
}

void casovac_start(timer_t casovac, int sekundy){
    struct itimerspec casik;
    casik.it_value.tv_sec=sekundy;
    casik.it_value.tv_nsec=0;
    casik.it_interval.tv_sec=0;
    casik.it_interval.tv_nsec=0;
    timer_settime(casovac,CLOCK_REALTIME,&casik,NULL);
}

timer_t vytvorCasovac(int signal)
{
  struct sigevent kam;
  kam.sigev_notify=SIGEV_SIGNAL;
  kam.sigev_signo=signal;
  
  timer_t casovac;
  timer_create(CLOCK_REALTIME, &kam, &casovac);
  return(casovac); 
}

////////////////////////////////////////////////////////////////////////////////////////////
// first client

void gula(float r,float* V, float* S, float* D) {

    *V = PI * r * r * r * 4 / 3;
    *S = 4 * PI * r * r;
    *D = 2 * r;
}
void *first_client_thread(){  
    printf("[KLIENT 1] bol vytvoreny\n");
    //share memory segment
    key_t key = key = port;
    int shmid;
    
    typedef struct sphere{
    float r, S, D, V, M, hustota, P;
    int material,price;
    }SPHERE;
    SPHERE* prva;
    
    if((shmid = shmget(key,sizeof(SPHERE),0666))<0){
        perror("shmget");
        exit(1);
    }
    if((prva = shmat(shmid,NULL,0))==(SPHERE *) -1){
        perror("shmat");
        exit(1);
    }
    /////////////////////////////////////////////////////////////////////////////
    sem_wait(&semafor);
    /*
    int value; 
    sem_getvalue(&semafor, &value); 
    printf(" zapisujem,The value of the semaphors is %d\n", value);
    */
    gula(prva->r, &prva->V, &prva->S, &prva->D);
    printf("\n\t[KLIENT 1]\n");
    printf("Dokoncil som vypocet\n");
    sem_post(&semafor);
    printf("[KLIENT 1] bol ukonceny\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// secound client

void gulaV2(float r, float hustota,int price, float V, float* M,float* P) {

    *M = hustota * V;
    *P = price * (V*0.0000001);

}

void *secound_client_thread(){  
    printf("[KLIENT 2] bol vytvoreny\n");
    //share memory segment
    key_t key = key = port;
    int shmid;
    
    typedef struct sphere{
    float r, S, D, V, M, hustota, P;
    int material,price;
    }SPHERE;
    SPHERE* prva;
    
    if((shmid = shmget(key,sizeof(SPHERE),0666))<0){
        perror("shmget");
        exit(1);
    }
    if((prva = shmat(shmid,NULL,0))==(SPHERE *) -1){
        perror("shmat");
        exit(1);
    }
    ///////////////////////////////////////////////////////////
    while(prva->D==0){
        sleep(1);
        if(prva->material!=0){
            printf("\n\t[KLIENT 2]\n");
            printf("Druhý klient čaká, lebo prvý klient ešte neurobil výpočet\n");
        }
    }
    sem_wait(&semafor);
    switch (prva->material) {
    case 1: //beton 70eur/m^3
        prva->hustota = 2.35;
        prva->price = 70;
        break;
    case 2: // neriesme cenu 
        prva->hustota = 3.5;
        prva->price = 0;
        break;
    case 3: //drevo 100eur/m^3
        prva->hustota = 0.68;
        prva->price = 100;
        break;
    case 4: //guma 90eur/m^3
        prva->hustota = 1.1;
        prva->price = 90;
        break;
    case 5: //hlinik 4860eur/m^3
        prva->hustota = 2.7;
        prva->price = 4860;
        break;
    case 6: //olovo 13608eur/m^3
        prva->hustota = 11.34;
        prva->price = 13608;
        break;
    case 7: //striebro 4935000eur/m^3
        prva->hustota = 10.5;
        prva->price = 4935000;
        break;
    case 8: //zelezo 944eur/m^3
        prva->hustota = 7.87;
        prva->price = 944;
        break;
    default:
        printf("\n\t[KLIENT 2]\n");printf("wrong entry");
        if(shmctl(shmid,IPC_RMID,NULL)==-1)
           printf("IPC_RMID error");
        exit(-999);
    }
    gulaV2(prva->r, prva->hustota, prva->price, prva->V, &prva->M, &prva->P);
    //sem_post(&semafor);
////////////////////////////////////////////////////////////////////////////////////////////////
//conection to server segment
    char buffer[DEFAULT_BUFLEN];
    bzero(buffer, sizeof(buffer)); 
    int sock = 0; 
    int identificator=2;
    struct sockaddr_in serv_addr; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n\t[KLIENT 2]\n");
        printf("\n Socket creation error \n"); 
        exit (-1); 
    } printf("\n\t[KLIENT 2]\n");printf("Client_2 socket is created\n");
   
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(port); 
       
    if(inet_pton(AF_INET,IP, &serv_addr.sin_addr)<=0)  
    { 
        printf("\n\t[KLIENT 2]\n");
        printf("\nInvalid address/ Address not supported \n"); 
        exit(-1); 
    } 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\n\t[KLIENT 2]\n");
        printf("\nConnection Failed \n"); 
        exit(-1); 
    } printf("\n\t[KLIENT 2]\n");printf("Client_2 connected to server\n");
    
       send(sock,&identificator, sizeof(int),0);
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "send_next", 9)) == 0){
       send(sock,&prva->S, sizeof(float),0);
       }
       else{printf("WEIRD ER1"); exit(-555);}
       
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "send_next", 9)) == 0){
       send(sock,&prva->D, sizeof(float),0);
       }
       else{printf("WEIRD ERR2"); exit(-555);}
       
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "send_next", 9)) == 0){
       send(sock,&prva->V, sizeof(float),0);
       }
       else{printf("WEIRD ERR3"); exit(-555);}
       
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "send_next", 9)) == 0){
       send(sock,&prva->M, sizeof(float),0);
       }
       else{printf("WEIRD ERR4"); exit(-555);}
       
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "send_next", 9)) == 0){
       send(sock,&prva->P, sizeof(float),0);
       }
       else{printf("WEIRD ERR5"); exit(-555);}
       
       recv(sock,buffer,sizeof(buffer),0);
       if((strncmp(buffer, "exit", 4)) == 0){
       printf("\n\t[KLIENT 2]\n");
       printf("vypocet uspesne odoslany servru\n");
       }
       else{printf("WEIRD ERR6"); exit(-555);}
       
    // close the socket 
    close(sock); 
    printf("[KLIENT 2] bol ukonceny\n");
}

/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// third client

void *third_client_thread(){  
    printf("[KLIENT 3] bol vytvoreny\n");
    typedef struct sphere{
    float  S, D, V, M, P;
    }SPHERE;
    SPHERE* prva;
    
    if((prva = (SPHERE*) malloc(sizeof(SPHERE))) == NULL){
    printf("\n\t[KLIENT 3]\n");
    printf("Malo pamate.\n");
    exit(2);
    }
    
    int identificator = 3;
    char buffer[]="send_next";
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n\t[KLIENT 3]\n");
        printf("\n Socket creation error \n"); 
        exit (-1); 
    } printf("\n\t[KLIENT 3]\n"); printf("Client_3 socket is created\n");
   
    serv_addr.sin_family = AF_INET; 
    //serv_addr.sin_port = htons(one->port_server); 
    serv_addr.sin_port = htons(port); 
    if(inet_pton(AF_INET,IP, &serv_addr.sin_addr)<=0)  
    { 
        printf("\n\t[KLIENT 3]\n");
        printf("\nInvalid address/ Address not supported \n"); 
        exit(-1); 
    } 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\n\t[KLIENT 3]\n");
        printf("\nConnection Failed \n"); 
        exit(-1); 
    } printf("\n\t[KLIENT 3]\n"); printf("Client_3 connected to server\n");
    
       send(sock,&identificator,sizeof(int),0);
       recv(sock,&prva->S,sizeof(float),0);
       
       send(sock,&buffer, strlen(buffer),0);
       recv(sock,&prva->D,sizeof(float),0);
       
       send(sock,buffer,strlen(buffer),0);
       recv(sock,&prva->V,sizeof(float),0);
       
       send(sock,buffer,strlen(buffer),0);
       recv(sock,&prva->M,sizeof(float),0);
       
       send(sock,buffer,strlen(buffer),0);
       recv(sock,&prva->P,sizeof(float),0);
       
       send(sock,"exit",strlen(buffer),0);
       
       printf("\n\t[KLIENT 3]\n");
       printf("Data uspesne prijate zo servera\n"); 
       
    // close the socket 
  close(sock);
    
  printf("\n\t[KLIENT 3]\nNa zobrazenie výsledku musíte počkať 10 sekúnd\nAk nechcete čakať, kúpte si VIP!!!\n");
  signal(SIGUSR1,sig);
  timer_t casovac;
  casovac=vytvorCasovac(SIGUSR1);
  casovac_start(casovac,10);
  
  sem_wait(&semafor);
  printf("\n\t[KLIENT 3]\n");
  printf("Objem gule:%fcm^3\nPovrch gule:%fcm^2\nPriemer gule:%fcm\nHmotnost gule:%fg\nCena:%feur\n", prva->V, prva->S, prva->D, prva->M,prva->P);
  printf("[KLIENT 3] bol ukonceny\n");
}


/////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////
// MAIN SEGMENT
/////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) 
{   
    if (argc != 3) { printf("\n\t[KLIENT 4]\n");printf("skuste prosim dodrzat format(nazov + ip + port)\n");return 1; }
    printf("[KLIENT 4] bol vytvoreny\n");
    port = atoi(argv[2]);
    IP = argv[1];
    
    //share memory segment
    key_t key = key = port; 
    int shmid;
    
    
    typedef struct sphere{
    float r, S, D, V, M, hustota, P;
    int material,price;
    }SPHERE;
    
    SPHERE* prva;
    
    if((shmid = shmget(key,sizeof(SPHERE),IPC_CREAT | 0666))<0){
        perror("shmget");
        exit(1);
    }
    if((prva = shmat(shmid,NULL,0))==(SPHERE *) -1){
        perror("shmat");
        exit(1);
    }prva->material=0;
    ////////////////////////////////////////////////////////////////////
    //semaphore segment
    
    if(sem_init(&semafor,0,0)!=0){
       perror("Semaphore init failed");
       if(shmctl(shmid,IPC_RMID,NULL)==-1)
           printf("IPC_RMID error");
       exit(-999);
    }

    ////////////////////////////////////////////////////////////////////////
    //thread segment
    
    pthread_t first_thread,secound_thread,third_thread;
     /////////////////////////////////////////////////////////////////////////////////
    //fourth client
    printf("\n\t[KLIENT 4]\n");
    printf("Zadajte polomer gule v cm:\n");
    scanf("%f", &prva->r);
    printf("\n\t[KLIENT 4]\n");
    printf("Material z ktoreho je gula vyrobenaz podla cisla ktore ma priradene\n(Beton-1;Diamant-2;Drevo[dub]-3;Guma-4;Hlinik-5;Olovo-6;Striebro-7;Zelezo-8):\n");
    scanf("%d", &prva->material);
    prva->D=0;
    sem_post(&semafor);
    /////////////////////////////////////////////////////////////////////////////////////////////
    //void *thread_result;
    if(pthread_create(&first_thread,NULL,first_client_thread,NULL)!=0){
        perror("pthread_create faild");
        if(shmctl(shmid,IPC_RMID,NULL)==-1)
            printf("IPC_RMID error");
        exit(-998);
    }
    if(pthread_create(&secound_thread,NULL,secound_client_thread,NULL)!=0){
        perror("pthread_create faild");
        if(shmctl(shmid,IPC_RMID,NULL)==-1)
            printf("IPC_RMID error");
        exit(-997);
    }
    if(pthread_create(&third_thread,NULL,third_client_thread,NULL)!=0){
        perror("pthread_create faild");
        if(shmctl(shmid,IPC_RMID,NULL)==-1)
            printf("IPC_RMID error");
        exit(-997);
    }
    
    if(pthread_join(first_thread,NULL)!=0){
        perror("pthread_join faild");
        exit(-996);
    }
    if(pthread_join(secound_thread,NULL)!=0){
        perror("pthread_join faild");
        exit(-995);
    }
    if(pthread_join(third_thread,NULL)!=0){
        perror("pthread_join faild");
        exit(-995);
    }
    if(shmctl(shmid,IPC_RMID,NULL)==-1)
        printf("IPC_RMID error");

  sem_destroy(&semafor);

  printf("[KLIENT 4] bol ukonceny\n");
  return 0; 
}

