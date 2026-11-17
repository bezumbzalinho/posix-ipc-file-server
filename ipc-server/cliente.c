#include "server.h"


shdMem *ptr; //ponteiro para aceder ao segmento
int chave;
char pipeName[10];
int pipeDesc;


void trataTermina()
{
	//Desacoplamento da memoria
	shmdt(ptr);
	printf("\nDesacoplei a memória\n");
	unlink(pipeName);//Destruir o FIFO
	printf("Eliminei o FIFO\n");
	exit(0);
}


void setSignal()
{
	sighandler sa;
	
	sa.sa_handler=trataTermina;
	
	sigemptyset (&sa.sa_mask);
	sa.sa_flags=0;
	
	sigaction(2,&sa,NULL);
}




void getSemShrMem(int* idshm, int *idsem)
{
	*idshm=shmget(chave, 0,0);
	exit_on_error(*idshm,"Erro ao tentar aceder a mem. partilhada\n O server tem de estar a funcionar");

	//id semaforos
	*idsem=semget(chave,0,0);
	exit_on_error(*idsem,"Erro ao tentar aceder ao semáforo\n O server tem de estar a funcionar");
}



void sendRequest(shdMem *ptr,shdMem msg,int idsem)
{
	int check = semget(chave,0,0);
	exit_on_error(check,"Erro ao tentar aceder ao semáforo\n O server tem de estar a funcionar");
	
	sem_operacao_n(idsem,SEM_ESCREVE,-1);//espera escrever
	*ptr=msg; 
	sem_operacao_n(idsem,SEM_LER,+1);//assinala ler
}

int main(int argc, char* argv[], char* envp[]){
	int idshm,idsem,i;
	char c;
	shdMem msg;
	int pid = getpid();

	//Usage
	check_arg(argc, argv);
	chave=atoi(argv[1]);
	
	//id memória partilhada
	getSemShrMem(&idshm,&idsem);
	
	sprintf(pipeName, "%d", pid);
	//criar fifo
	pipeDesc=mknod(pipeName, S_IFIFO|0644, 0); /*ou mkfifo(NOME_DO_FIFO, 0644);*/
	exit_on_error(pipeDesc,"Erro ao criar o fifo");
	
	setSignal();	
	
	

	//acoplamento 
	ptr= (shdMem *) shmat(idshm,NULL,0);
	
	msg.pid_escritor=pid;
	
	//Obtém o path para a diretoria atual
	getcwd(msg.pathPipe, MAXTEXTO);
	
	//Vamos atualizar a variável pathPipe com o caminho do pipe nomeado creado pelo cliente
	sprintf(msg.pathPipe,"%s/%d",msg.pathPipe,pid);

	
	
	
	
	
	do{
		printf("\nQual o ficheiro que quer?\f[.] para sair\n");
		scanf("%100s", msg.fich);
		
		fflush(stdin);
		
		if(msg.fich[0] != '.'){

			sendRequest(ptr,msg,idsem);//espera e envia pedido para o server

			//abrir fifo
			pipeDesc=open(pipeName, O_RDONLY);
			exit_on_error(pipeDesc,"Erro ao abrir o fifo");

			//ler do fifo
			printf("\n["); 
			while(read(pipeDesc,&c,sizeof(char))>0) //Leitura do pipe
			{
 				printf("%c",c);
			}
			printf("]\n");
 			close(pipeDesc);
 			
		}
		
	}while(msg.fich[0] != '.');
	trataTermina();
	
	return 0;
}
