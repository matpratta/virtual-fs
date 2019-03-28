#include <stdio.h>
#include <time.h>
#include <string.h>

/**
 * virtual-fs
 * Sistema de arquivos virtual baseado no Linux para matéria de Sistemas Operacionais
 */

/*******************************************************************************
 * Definições e Estruturas
 */

// Struct para entrada do diretório
struct dirItem {
    char nome [8];
    time_t dataHora;
    int dirPai;
};

// Estrutura de diretórios global
struct dirItem estrutura [1024];
int dirAtual = 0;

/*******************************************************************************
 * Utilitários
 */

// Utilitário para gerar struct representando entrada
struct dirItem util_criarEntrada(char nome [8], int dirPai) {
    struct dirItem entrada;

    // Cria a entrada usando memcpy, para copiar o nome
    memcpy(entrada.nome, nome, 8);
    entrada.dirPai = dirPai;
    entrada.dataHora = time(NULL);

    return entrada;    
}

// Mesma função anterior, porém com suporte a char* para inserir direto do código 
struct dirItem util_criarEntradaPtr(char* nome, int dirPai) {
    // Criar um vetor de caracteres e converter de ponteiro para vetor usando strcpy, só vai ser usada para criar o root
    char nomeVetor [8];
    strcpy(nomeVetor, nome);
    
    return util_criarEntrada(nomeVetor, dirPai);
}

// Função para gerar uma string contendo o caminho completo para a entrada atual
void util_gerarNomeCompleto (struct dirItem entrada, char* nomeCompleto) {
    char caminho [256];

    if (entrada.dirPai == 0) {
        // Estamos na root
        
        if (entrada.nome[0] == '/' && strlen(entrada.nome) == 1) {
            // Estamos consultando a raíz, retornar diretório atual como /
            snprintf(nomeCompleto, 256, "/");
            return;
        } else {
            // Deixar caminho atual em branco para ser preenchiddo pela barra nas linhas abaixo
            snprintf(caminho, sizeof caminho, "");
        }
    } else {
        util_gerarNomeCompleto(estrutura[entrada.dirPai], caminho);
    }

    // Retornamos o nome completo
    snprintf(nomeCompleto, 256, "%s/%s", caminho, entrada.nome);
}

// Função para obter a próxima entrada vazia na listagem de arquivos
int util_proximoIndex () {
    for (int i = 0; i < 1024; i++) {
        if (strlen(estrutura[i].nome) == 0) return i;
    }
    return -1;
}

// Função para buscar entrada em diretório
int util_buscarEntrada (char* nome, int diretorio) {
    for (int i = 0; i < 1024; i++) {
        if (0 == strcmp(estrutura[i].nome, nome) && (diretorio == -1 || estrutura[i].dirPai == diretorio))
            return i;
    }
    return -1;
}


/*******************************************************************************
 * Funções de Comandos
 */

// ls ()
void func_ls (char* args) {
    struct dirItem entrada;
    struct tm data;

    // Verificar se passamos argumentos modificadores
    int isList = 0, isDetailed = 0, isFullPath = 0;
    if (args != NULL && args[0] == '-') {
        for (int iChar = 0; iChar < strlen(args); iChar++) {
            switch (args[iChar]) {
                case 'l': isList = 1; break;
                case 's': isDetailed = 1; break;
                case 'f': isFullPath = 1; break;
            }
        }
    }

    // Preparativos
    if (isList && isDetailed) {
        // Exibir cabeçalho da tabela detalhada
        printf("    DATA     HORA   Arquivo\n");
    }

    // Loop principal
    for (int i = 0; i < 1024; i++) {
        entrada = estrutura[i];

        // Verificar se o diretório pai é o atual
        if (entrada.dirPai != dirAtual) continue;

        // Verificar se a entrada existe
        if (strlen(entrada.nome) == 0) continue;

        if (isList) {
            // Modificador lista
            if (isDetailed) {
                // Pegar data do arquivo
                data = *localtime(&entrada.dataHora);

                char* fileName = entrada.nome;
                if (isFullPath) {
                    // Exibir caminho completo da pasta ao invés de apenas o nome
                    char nomeCompleto [256];
                    util_gerarNomeCompleto(entrada, nomeCompleto);
                    fileName = nomeCompleto;
                }

                // Exibe a linha
                printf("%4d-%02d-%02d %02d:%02d:%02d %s\n", data.tm_year + 1900, data.tm_mon + 1, data.tm_mday, data.tm_hour, data.tm_min, data.tm_sec, fileName);
            } else {
                printf("%s\n", entrada.nome);
            }
        } else {
            // Exibir lado a lado
            printf("%s ", entrada.nome);
        }
    }

    // Encerrar comando com uma nova linha
    printf("\n");
}

// mkdir (dirname)
void func_mkdir (char* nome) {
    // Testar se o diretorio já existe
    if (util_buscarEntrada(nome, dirAtual) > -1) {
        printf("Erro: diretorio ja existente.\n");
        return;
    }

    int iEstrutura = util_proximoIndex();
    estrutura[iEstrutura] = util_criarEntrada(nome, dirAtual);
}

// rmdir (dirname)
void func_rmdir (char* nome) {
    // Testar se o diretorio já existe
    int iEstrutura = util_buscarEntrada(nome, dirAtual);
    if (iEstrutura == -1) {
        printf("Erro: diretorio nao encontrado.\n");
        return;
    }

    // Sobrescrever dados e zerar nome
    estrutura[iEstrutura] = util_criarEntradaPtr("", 0);
}

// cd (dirname)
void func_cd (char* diretorio) {
    if (0 == strcmp(diretorio, "..")) {
        // Voltamos ao parent
        dirAtual = estrutura[dirAtual].dirPai;
    } else {
        // Procuramos o index
        int iDiretorio = util_buscarEntrada(diretorio, dirAtual);

        if (iDiretorio == -1) {
            // Não encontrado, retornar erro
            printf("Erro: diretorio nao encontrado.\n");
        } else {
            // Encontrado, apenas trocamos o index do atual
            dirAtual = iDiretorio;
        }
    }
}

/*******************************************************************************
 * Prompt de Comando
 */
void prompt () {
    // Gera o nome do caminho atual para exibir no prompt
    char dirAtualCaminho [256];
    struct dirItem dirAtualInfo = estrutura[dirAtual];
    util_gerarNomeCompleto(dirAtualInfo, dirAtualCaminho);

    // Exibe prompt e solicita entrada
    char comando [256];
    printf("%s $ ", dirAtualCaminho);
    fgets(comando, 256, stdin);

    // Limpar comando, caso o último char seja \n (nova linha), substituir por \0 (NULL)
    if (comando[strlen(comando) - 1] == '\n')
        comando[strlen(comando) - 1] = '\0';

    // Dividir o comando em blocos de espaço
    char* comandoPtr = strtok(comando, " ");
    char* arg0 = strtok(NULL, " ");

    // Testar comando exit
    if (0 == strcmp(comandoPtr, "exit")) return;
    else if (0 == strcmp(comandoPtr, "ls")) func_ls(arg0);
    else if (0 == strcmp(comandoPtr, "mkdir")) func_mkdir(arg0);
    else if (0 == strcmp(comandoPtr, "rmdir")) func_rmdir(arg0);
    else if (0 == strcmp(comandoPtr, "cd")) func_cd(arg0);

    prompt();
}


/*******************************************************************************
 * Main
 */
int main () {
    printf("+---------------------------------------------------+\n");
    printf("|            Sistema de Arquivos Virtual            |\n");
    printf("| Materia: Sistemas Operacionais I, Fatec Americana |\n");
    printf("+---------------------------------------------------+\n");

    // Criar root
    estrutura[0] = util_criarEntradaPtr("/", 0);

    prompt();

    return 0;
}