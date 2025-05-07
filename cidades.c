#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CITIES 89
#include "cidades.h"


int ler_cidades(const char *arquivo, Cidade cidades[]) {
    FILE *fp = fopen(arquivo, "r");
    if (!fp) {
        perror("Erro ao abrir arquivo");
        return -1;
    }

    char linha[256];
    int count = 0;

    // ignora o cabecalho do csv
    fgets(linha, sizeof(linha), fp);

    while (fgets(linha, sizeof(linha), fp)) {
        char *token = strtok(linha, ",");
        if (token) strncpy(cidades[count].nome, token, sizeof(cidades[count].nome));
        token = strtok(NULL, ",");
        if (token) cidades[count].latitude = atof(token);
        token = strtok(NULL, ",");
        if (token) cidades[count].longitude = atof(token);
        token = strtok(NULL, ",");
        if (token) cidades[count].preco= atof(token);
        count++;
    }

    fclose(fp);
    return count;
}

// imprimir as cidades
void imprimir_cidades(Cidade cidades[], int total) {
    for (int i = 0; i < total; i++) {
        printf("Cidade: %s | Latitude: %.4f | Longitude: %.4f | Diesel: R$ %.3f\n",
               cidades[i].nome, cidades[i].latitude, cidades[i].longitude, cidades[i].preco);
    }
}


