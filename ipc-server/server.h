#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>

#define exit_on_null(s,m) if (s == NULL) { perror(m); exit(1); }
#define exit_on_error(s,m) if (s < 0) { perror(m); exit(1); }

#define PERMISSOES 0600 /* admita-se que serve para o exemplo */
#define MAXTEXTO 100
#define SEM_ESCREVE 0
#define SEM_LER 1

typedef struct sigaction sighandler;

//Organização da memória partilhada
typedef struct s_shm
{
	int pid_escritor;
 	char pathPipe[MAXTEXTO];
 	char fich[MAXTEXTO];

}shdMem;

//SEMAFOROS------------------------------------------------------------------------
union semun 
{ 
	int val; //value for SETVAL
	struct semid_ds *buf; //buffer for IPC_STAT, IPC_SET
	unsigned short *array; //array for GETALL, SETALL

	//Linux specific part:
	struct seminfo *__buf; //buffer for IPC_INFO
};

int chave_val(char *str)
{
	int i;
	for(i=0;i<strlen(str);++i)
	{
		//printf("%c\t %d\n",str[i], isdigit(str[i]));
		if(!isdigit(str[i]))
			return 0;
	}
	return 1;
}

void check_arg(int argc, char *argv[])
{
	if (argc ==2)
	{
		int check= chave_val(argv[1]);
		if(!(check))
		{
			printf("Usage: %s <chave> que deve ser um inteiro\n", argv[0]);
			exit (1);
		}
	}
	else
	{
	   printf("Usage: %s <chave> que deve ser um inteiro\n", argv[0]);
	   printf("Numero de argumentos invalidos\n");
	   exit(1);
	}
}





//remove semaforo
void sem_remover(int id){
	semctl(id, 0, IPC_RMID);
}

//cria um conjunto de semáforos mas nao inicializa as respectivas variáveis de controlo
int sem_criar_n(int chave, int numero_de_semaforos, int permissoes)
{
	int id;
	id=semget(chave, numero_de_semaforos, IPC_CREAT|IPC_EXCL|permissoes);
	exit_on_error(id,"Erro ao criar semaforo");
	return id;
}

//(re)inicializa o valor da variavel de controlo do semaforo
void sem_ini_var_n(int id, int indice_do_semaforo, int valor)
{
	int r;
	union semun su;
	su.val=valor;
	r=semctl(id, indice_do_semaforo, SETVAL, su);
	exit_on_error(r,"Erro ao atribuir valor ao semaforo");
}

//executa uma operacao sobre o semaforo n do conjunto
void sem_operacao_n(int id, int indice_do_semaforo, int operacao){
	int r;
	struct sembuf sb;
	sb.sem_num = indice_do_semaforo; //Indice do semaforo no conjunto
	sb.sem_op = operacao;
	sb.sem_flg = 0; //flags
	r=semop(id, &sb, 1 );
	exit_on_error(r," Erro ao realizar a operacao sobre o semaforo");
}

