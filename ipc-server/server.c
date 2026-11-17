#include "server.h"

int idshm,idsem;

void trataTermina(int sinal){
	idshm=shmctl(idshm, IPC_RMID, NULL);//Elimina memoria partilhada
	sem_remover(idsem);//Destruir o semaforo
	printf("\nEliminei a memória partilhada e o semáforo\n");
	exit(0);
}

void printPedido(shdMem *ptr){
	printf("PID: %d\tFicheiro:  %s\tPipe: %s\n",ptr->pid_escritor,ptr->fich,ptr->pathPipe);
	printf("--------------------------------------\n");
}


void setSignal()
{
	sighandler sa;
	
	sa.sa_handler=trataTermina;
	
	sigemptyset (&sa.sa_mask);
	sa.sa_flags=0;
	
	sigaction(2,&sa,NULL);
	
	signal(SIGCHLD,SIG_IGN);//Evita que os filhos fiquem em zombie
}

//cria e inicializa os semaforos e a memoria partilhada
void setMemSem(int chave){
	//Cria memória partilhada
	idshm=shmget(chave, sizeof(struct s_shm), IPC_CREAT | PERMISSOES);
	exit_on_error(idshm,"Erro ao tentar criar/aceder a mem. partilhada\n");

	//Cria e inicializa semaforos
	idsem=sem_criar_n(chave,2,PERMISSOES);
	sem_ini_var_n(idsem,SEM_ESCREVE,1); //Para o cliente saber quando pode escrever 
	sem_ini_var_n(idsem,SEM_LER,0); //Para o servidor saber quando pode ler
}


//Escreve um ficheiro para um pipe
int writeFileToPipe(FILE *file, int pipeDesc){
	char *buffer;
	int res;
	
	//Obter o tamanho do ficheiro
	fseek(file, 0, SEEK_END);
	long int fsize = ftell(file);
	fseek(file, 0, SEEK_SET);  /* same as rewind(f); */

	
	buffer = malloc(fsize + 1);//aloca espaço suficiente para o conteúdo do ficheiro
	if(!buffer)
		return 0;
	fread(buffer, fsize, 1, file);//le o ficheiro
	

	buffer[fsize-1] = 0;//adiciona o carater terminador

	res = write(pipeDesc, buffer,fsize);// escreve e guarda o resultado
	
	free(buffer);
	

	return res > 0;//se write() der erro  retorna -1
}

int trataPedido(shdMem *ptr)
{
	int res,pipeDesc;
	char *str = malloc((strlen(ptr->fich) + 45 ) * sizeof(char));//para escrever cabeçalho
	exit_on_null(str,"Erro ao alocar memoria");

	
	
	//abrir fifo
	pipeDesc=open(ptr->pathPipe, O_WRONLY);
	exit_on_error(pipeDesc,"Erro ao abrir o fifo");//elaborar mensagem de erro?
	
	//escrever fifo
	FILE *file = fopen(ptr->fich, "r");
	
	if(file)
	{
		printPedido(ptr);
		res=writeFileToPipe(file,pipeDesc);
		fclose(file);
	}
	else
	{
		sprintf(str,"ERRO -> Server: Não tenho o ficheiro |%s|",ptr->fich); 
		write(pipeDesc, str , strlen(str));
		res=0;
	}
	
	free(str);
	close(pipeDesc);
	
	return res;
}

void erroPedido(shdMem *ptr){
	printf("\nHouve um PROBLEMA a processar o pedido:\n");
	printPedido(ptr);
}

int main(int argc, char* argv[]){
	int chave, i;
	shdMem *ptr; //ponteiro para aceder ao segmento de memoria partilhada

	//Usage
	check_arg(argc, argv);
	
	setSignal();
	
	chave=atoi(argv[1]);
	setMemSem(chave);//Cria e inicializa os semaforos e a memoria partilhada
	
	//acoplamento
	ptr= (struct s_shm *) shmat(idshm,NULL,0);
	
	printf("Server: %d (pid:%d)\nEstou pronto, venham esses pedidos!!\n",chave,getpid());
	
	
	while(1)
	{
		sem_operacao_n(idsem,SEM_LER,-1);//espera SEM_LER
		
		if(!fork())
		{
			if(!trataPedido(ptr))//se houve um erro com o pedido
			{
				erroPedido(ptr);
			}
			exit(0);
		}
		
		sem_operacao_n(idsem,SEM_ESCREVE,+1);//assinala SEM_ESCREVER
	}


	return 0;
}
