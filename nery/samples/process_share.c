#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include "rf-time.h"

float m_random_fp(float LO, float HI);

int n_workers;

int main(int argc, char **argv)
{
    //sou o PAI
    
    pid_t pid;
    key_t key;
    int m_pid;
    int shmid;
    int mode;
    long i;
    long k;

    double time1,time2;

    float *vecA;
    float *vecB;
    float *vecC;
    long sizeA;
    long sizeB;
    long sizeC;
    long blockSize;

    FILE *fp1;
    FILE *fp2;
    FILE *fp3;
  
    if(argc < 4)
    {
        printf("uso: ./shared <n_workers> <arquivo leitura A> <arquivo leitura B> <arquivo saida C>\n");
        return 1;
    }

    printf("Pai %d começou!\n",getpid());
    n_workers = atoi(argv[1]);
    if(n_workers < 1)
    {
        printf("precisa de pelo menos 1 trabalhador\n");
        return 1;
    }

    //ler arquivos
    fp1 = fopen(argv[2],"r");
    if(fp1 == NULL)
    {
        perror("fopen");
        exit(1);
    }

    fp2 = fopen(argv[3],"r");
    if(fp2 == NULL)
    {
        perror("fopen");
        exit(1);
    }
    
    fscanf(fp1,"%ld\n",&sizeA);
    fscanf(fp2,"%ld\n",&sizeB);

    if(sizeA != sizeB)
    {
        printf("os vetores devem ter o mesmo tamanho\n");
        exit(1);
    }

    printf("length(vecA) = %ld\n",sizeA);
    printf("length(vecB) = %ld\n",sizeB);

    sizeC = sizeA; //ou sizeB

    vecA = (float *) malloc(sizeA*sizeof(float));
    if(vecA == NULL)
    {
        perror("malloc");
        exit(1);
    }

    vecB = (float *) malloc(sizeB*sizeof(float));
    if(vecB == NULL)
    {
        perror("malloc");
        exit(1);
    }

    for(i = 0 ; i < sizeA ; i++)
        fscanf(fp1,"%f",&vecA[i]);

    for(i = 0 ; i < sizeB ; i++)
        fscanf(fp2,"%f",&vecB[i]);

    //fecha os arquivos de entrada
    fclose(fp1);
    fclose(fp2);

    //cria arquivo de saida
    fp3 = fopen(argv[4],"w");
    if(fp3 == NULL)
    {
        perror("fopen");
        exit(1);
    }

    srand(time(NULL));
    
    //cria uma chave única de acesso
    if((key = ftok("shared_mem.c",'R')) == -1){
        perror("ftok");
        exit(1);
    }

    //cria uma região de memória compartilhada de largura sizeA ou sizeB
    if((shmid = shmget(key, sizeC*sizeof(float), 0644 | IPC_CREAT)) == -1){
        perror("shmget");
        exit(1);
    }

    //associa o processo à região
    vecC = shmat(shmid, (void*)0, 0);
    if(vecC == (float*)(-1))
    {
        perror("shmat");
        exit(1);
    }

    time1 = get_clock_msec();
    //cria os processos filhos
    for(i = 0 ; i < n_workers ; i++)
    {
        pid = fork();
        if(pid == -1)
        {
            perror("fork");
            return 1;
        }
        else if(pid == 0) //filho
        {
            m_pid = (getpid()%getppid())-1;
            blockSize = sizeC/n_workers;
            printf("Filho %d \n",m_pid);

            //cria uma chave única de acesso
            if((key = ftok("shared_mem.c",'R')) == -1){
                perror("ftok");
                exit(1);
            }

            //todos os filhos já têm shmid que herdaram do pai
            //if((shmid = shmget(key, SHM_SIZE, 0644 | IPC_CREAT)) == -1){ 

            //cada filho se associa à região compartilhada
            vecC = shmat(shmid, (void*)0, 0);
            if(vecC == (float*)(-1))
            {
                perror("shmat");
                exit(1);
            }

            //inicio soma
            for(k = m_pid*blockSize ; k < blockSize*(m_pid+1) ; k++)
            {
                vecC[k] = vecA[k] + vecB[k];
            }
            //fim soma

            //cada filho se desassocia à região compartilhada
            if(shmdt(vecC) == -1)
            {
                perror("shmdt");
                exit(1);
            }

            exit(0);
        }
        else //pai 
        {
            //nada faz aqui
            
        }
    }

    //pai espera todos os filhos
    while(wait(NULL) > 0);
    time2 = get_clock_msec();

    printf("time:%fms\n",(time2-time1));
    printf("filhos terminaram\n");

    //escreve o resultado no arquivo de saida C
    fprintf(fp3,"%ld\n",sizeC);
    for(i = 0 ; i < sizeC ; i++)
        fprintf(fp3,"%f\n",vecC[i]);

    //fecha arquivo de saida C
    fclose(fp3);

    //pai se desassocia à região compartilhada
    if(shmdt(vecC) == -1)
    {
        perror("shmdt");
        exit(1);
    }

    //pai destrói a região compartilhada
    if(shmctl(shmid, IPC_RMID, NULL) == -1)
    {
        perror("shmctl");
        exit(1);
    }

    printf("Pai %d terminou!\n",getpid());

}

inline float m_random_fp(float LO, float HI)
{
    if(LO == HI) //evita divisao por zero
        return 0.0f;

    return (LO + (rand()) /( (RAND_MAX/(HI-LO))));
}