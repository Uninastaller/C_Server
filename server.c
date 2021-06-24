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




int main(int argc, char* argv[]) 
{  
    if (argc != 2) { printf("\n\t[SERVER]\n"); printf("skuste prosim dodrzat format(nazov + port)\n");return 1; }
    /////////////////////////////////////////////////////////////////////////////////////
    typedef struct sphere{
    float  S, D, V, M, P;
    int semafor;
    }SPHERE;
    SPHERE* prva;
    
    ///////////////////////////////////////////////////////////////////////////////////////
    key_t key = key=9; 
    int shmid;
     if((shmid = shmget(key,sizeof(SPHERE),IPC_CREAT | 0666))<0){
        perror("shmget");
        exit(1);
    }
    if((prva = shmat(shmid,NULL,0))==(SPHERE *) -1){
        perror("shmat");
        exit(1);
    }
    prva->semafor = 0;
    /////////////////////////////////////////////////////////////////////////////
    int server_fd, new_socket; 
    struct sockaddr_in address; 
    struct sockaddr_in newAdress;
    socklen_t addr_size;
    int opt = 1;
    int addrlen = sizeof(address); 
    char buffer[] = "send_next";
    int identificator;
    pid_t childpid;
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } printf("Server socked is created\n");
       
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) 
        //If SO_REUSEADDR is enabled on a socket prior to binding it, the socket can be successfully bound unless there is a conflict with another socket
        //SO_REUSEPORT allows you to bind an arbitrary number of sockets to exactly the same source address and port
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET;  //tpc/udp
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( atoi(argv[1]) ); 
       
    
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } printf("Binded to port: %d\n",atoi(argv[1]));
    if (listen(server_fd, 5) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } printf("Listening...\n");
    
    while(1){
        new_socket =accept(server_fd,(struct sockaddr*)&newAdress, &addr_size);
        if(new_socket< 0){
            exit(1);
        }printf("Conection accepted from: %s:%d\n", inet_ntoa(newAdress.sin_addr),ntohs(newAdress.sin_port));
        
        if((childpid = fork())==0){
            close(server_fd);
            recv(new_socket,&identificator,sizeof(int),0);
            printf("%s:%d is client %d\n", inet_ntoa(newAdress.sin_addr),ntohs(newAdress.sin_port),identificator);
            if(identificator==2){
                    printf("\n\t[SERVER %d]\n",ntohs(newAdress.sin_port));
                    printf("prijmam data od klienta %d\n",identificator);
                    printf("Klient %d caka kym server %d ziska data\n",identificator,ntohs(newAdress.sin_port));
                    send(new_socket,buffer,strlen(buffer),0);
                    recv(new_socket,&prva->S,sizeof(float),0);
                    
                    send(new_socket,buffer,strlen(buffer),0);
                    recv(new_socket,&prva->D,sizeof(float),0);
                    
                    send(new_socket,buffer,strlen(buffer),0);
                    recv(new_socket,&prva->V,sizeof(float),0);
                    
                    send(new_socket,buffer,strlen(buffer),0);
                    recv(new_socket,&prva->M,sizeof(float),0);
                    
                    send(new_socket,buffer,strlen(buffer),0);
                    recv(new_socket,&prva->P,sizeof(float),0);
                    
                    prva->semafor=1;
                    send(new_socket,"exit",strlen(buffer),0);
                    printf("Data uspesne prijate\n");
                    printf("Client %d - %s:%d - Ended the communication with server\n",identificator,inet_ntoa(newAdress.sin_addr),ntohs(newAdress.sin_port));
                    break;
            }
            if(identificator==3){
                bzero(buffer, sizeof(buffer)); 
                while(prva->semafor==0){
                     sleep(5);
                }
                printf("\n\t[SERVER %d]\n",ntohs(newAdress.sin_port));
                printf("posielam data klientovi %d\n",identificator);
                send(new_socket,&prva->S, sizeof(float),0);
                
                recv(new_socket,buffer,sizeof(buffer),0);
                if((strncmp(buffer, "send_next", 9)) == 0){
                    send(new_socket,&prva->D, sizeof(float),0);
                }
                else{printf("WEIRD ERRRRRROR"); exit(-555);}
                
                recv(new_socket,buffer,sizeof(buffer),0);
                if((strncmp(buffer, "send_next", 9)) == 0){
                    send(new_socket,&prva->V, sizeof(float),0);
                }
                else{printf("WEIRD ERRRRRROR"); exit(-555);}
                
                recv(new_socket,buffer,sizeof(buffer),0);
                if((strncmp(buffer, "send_next", 9)) == 0){
                    send(new_socket,&prva->M, sizeof(float),0);
                }
                else{printf("WEIRD ERRRRRROR"); exit(-555);}
                
                recv(new_socket,buffer,sizeof(buffer),0);
                if((strncmp(buffer, "send_next", 9)) == 0){
                    send(new_socket,&prva->P, sizeof(float),0);
                }
                else{printf("WEIRD ERRRRRROR"); exit(-555);}
                
                recv(new_socket,buffer,sizeof(buffer),0);
                if((strncmp(buffer, "exit", 4)) == 0){
                    printf("\n\t[SERVER %d]\n",ntohs(newAdress.sin_port));
                    printf("vypocet uspesne odoslany klientovi %d\n",identificator);
                }
                else{printf("WEIRD ERRRRRROR"); exit(-555);}
                
                printf("Client %d - %s:%d - Ended the communication with server\n",identificator,inet_ntoa(newAdress.sin_addr),ntohs(newAdress.sin_port));
                prva->semafor = 0;
                break;
            }
        }else if(childpid<0){
            perror("fork");
            exit(-1024);
        }
    }
   
printf("\nclosing [SERVER %d]\n",ntohs(newAdress.sin_port));
//sem_destroy(&semafor);
close(new_socket); 
shmdt(prva);
shmctl (shmid, IPC_RMID, 0);
return 0; 
}

