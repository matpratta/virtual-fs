#include <stdio.h>
#include <time.h>
#include <string.h>

/**
 * virtual-fs
 * Sistema de arquivos virtual baseado no Linux para matéria de Sistemas Operacionais
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

// Utilitário para criar entrada
struct dirItem util_criarEntrada(char nome [8], int dirPai) {
    struct dirItem entrada;

    // Cria a entrada usando memcpy, para copiar o nome
    memcpy(entrada.nome, nome, 8);
    entrada.dirPai = dirPai;
    entrada.dataHora = time(NULL);

    return entrada;    
}
struct dirItem util_criarEntradaPtr(char* nome, int dirPai) {
    // Criar um vetor de caracteres e converter de ponteiro para vetor usando strcpy, só vai ser usada para criar o root
    char nomeVetor [8];
    strcpy(nomeVetor, nome);
    
    return util_criarEntrada(nomeVetor, dirPai);
}
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

void func_ls (int diretorio) {
    struct dirItem entrada;
    struct tm data;
    for (int i = 0; i < 1024; i++) {
        entrada = estrutura[i];

        // Verificar se o diretório pai é o atual
        if (entrada.dirPai != diretorio) continue;

        // Verificar se a entrada existe
        if (strlen(entrada.nome) == 0) continue;

        char nomeCompleto [256];
        util_gerarNomeCompleto(entrada, nomeCompleto);
        data = *localtime(&entrada.dataHora);

        printf("%4d-%02d-%02d %02d:%02d:%02d | %s\n", data.tm_year + 1900, data.tm_mon + 1, data.tm_mday, data.tm_hour, data.tm_min, data.tm_sec, nomeCompleto);
    }
}

void prompt () {
    struct dirItem dirAtualInfo = estrutura[dirAtual];
    char comando [256];

    // Gera o nome do caminho atual para exibir no prompt
    char dirAtualCaminho [256];
    util_gerarNomeCompleto(dirAtualInfo, dirAtualCaminho);

    // Exibe prompt e solicita entrada
    printf("%s $ ", dirAtualCaminho);
    fgets(comando, 256, stdin);

    // Limpar comando, caso o último char seja \n (nova linha), substituir por \0 (NULL)
    if (comando[strlen(comando) - 1] == '\n')
        comando[strlen(comando) - 1] = '\0';

    // Dividir o comando em blocos de espaço
    char* comandoPtr = strtok(comando, " ");

    // Testar comando exit
    if (0 == strcmp(comandoPtr, "exit")) return;
    else if (0 == strcmp(comandoPtr, "ls")) func_ls(dirAtual);

    prompt();
}

int main () {
    printf("+---------------------------------------------------+\n");
    printf("|            Sistema de Arquivos Virtual            |\n");
    printf("| Materia: Sistemas Operacionais I, Fatec Americana |\n");
    printf("+---------------------------------------------------+\n");

    // Criar root
    estrutura[0] = util_criarEntradaPtr("/", 0);

    estrutura[1] = util_criarEntradaPtr("dir1", 0);
    estrutura[2] = util_criarEntradaPtr("dir2", 0);
    estrutura[3] = util_criarEntradaPtr("dir3", 0);

    estrutura[4] = util_criarEntradaPtr("dir4", 2);
    estrutura[5] = util_criarEntradaPtr("dir5", 2);
    estrutura[6] = util_criarEntradaPtr("dir6", 5);

    prompt();

    return 0;
}