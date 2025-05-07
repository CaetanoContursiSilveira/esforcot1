#ifndef CIDADE_H
#define CIDADE_H


typedef struct {
    char nome[100];     // nome da cidade
    double preco;       // preço do diesel em reias
    double latitude;   
    double longitude;   
} Cidade;

/**
 * Le um csv com as cidades e preenche o vetor
 */
int ler_cidades(const char *filename, Cidade cidades[]);

#endif // CIDADE_H

