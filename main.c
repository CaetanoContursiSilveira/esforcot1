#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "agent.h"   
#include "cidades.h"   

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define CONSUMO_KM_POR_L 7.0f
#define N_EPISODIOS      10000

// distancia de haversine simplificada(pra considerar curvatura da terra)
static float haversine_km(float lat1, float lon1, float lat2, float lon2){
    const float R = 6371.0f;
    float dlat = (lat2 - lat1) * (float)M_PI / 180.0f;
    float dlon = (lon2 - lon1) * (float)M_PI / 180.0f;

    float a = sinf(dlat/2)*sinf(dlat/2) +
              cosf(lat1*M_PI/180.0f)*cosf(lat2*M_PI/180.0f) *
              sinf(dlon/2)*sinf(dlon/2);

    float c = 2.0f * atanf(sqrtf(a)/sqrtf(1.0f-a));
    return R * c;
}

// monta matriz de distâncias e vetor de precos
static void build_instance(const Cidade *C, size_t n,float dist[][MAX_CITIES],float preco[]){
    for (size_t i = 0; i < n; ++i) {
        preco[i] = (float)C[i].preco;
        for (size_t j = 0; j < n; ++j)
            dist[i][j] = i==j ? 0.0f :
                haversine_km((float)C[i].latitude, (float)C[i].longitude,(float)C[j].latitude, (float)C[j].longitude);
    }
}

int main(void){
    // carrega cidades
    Cidade cidades[MAX_CITIES];
    int n = ler_cidades("cidades.csv", cidades);
    if (n <= 0) { fprintf(stderr,"Falha ao ler cidades.csv\n"); return 1; }

    // instancia matrizes
    static float dist[MAX_CITIES][MAX_CITIES];
    static float preco[MAX_CITIES];
    build_instance(cidades, (size_t)n, dist, preco);

    //inicializa agente
    Agent ag;
    agent_init(&ag,
        (size_t)n,          // tamanho da matriz de cidades
        cidades,            // ponteiro para o vetor de cidades
        dist,               // matriz de distâncias 
        200.0f, 150.0f,     // custo do guincho, capacidade do tanque
        R3_DISTANCE_ONLY,   // funcao de reforco
        0.10f, 0.95f, 0.05f); // os hiperparametros alpha gamma e epsilon

    // treinamento
    for (int ep = 0; ep < N_EPISODIOS; ++ep) {

        agent_reset_episode(&ag, 0);
        int    visitado[MAX_CITIES]={0}, n_vis=0;
        size_t s = ag.current_city;
        int n_visited = 0;
        while (n_vis < n) {
            visitado[s] = 1; ++n_vis;
            ++n_visited;
            if (n_visited == n)               // todas visitadas termina episodio
             break;

            size_t a = agent_select_action(&ag);
            while (visitado[a]) a = (a+1)% (size_t)n;

            // combustivel e custos 
            float km = dist[s][a];
            float litros_gastos = km/CONSUMO_KM_POR_L;
            ag.fuel_level -= litros_gastos;

            int   pane = 0;
            float litros_ref=0.0f;
            if (ag.fuel_level<0){ pane=1; ag.fuel_level=0; }
            if (ag.fuel_level < ag.level_ref){
                litros_ref = ag.tank_capacity - ag.fuel_level;
                agent_refuel(&ag, litros_ref);
            }

            float r = agent_compute_reward(&ag, s, a, pane, litros_ref);
            agent_update_q(&ag, s, a, r, a, 0);

            s = a;
        }
    }

    // rota greedy final
    printf("\nRota greedy final:\n");

    agent_reset_episode(&ag, 0);
    int    vis[MAX_CITIES] = {0};
    size_t s = ag.current_city;

    float  fuel       = ag.tank_capacity;   // tanque cheio na partida  
    float  total_km   = 0.0f;
    double total_cost = 0.0;

    for (int step = 1; step <= n; ++step) {

        printf("%02d) %s\n", step, cidades[s].nome);
        vis[s] = 1;

        // escolhe próximo não visitado 
        size_t next  = n;
        float  best_q = -1e30f;
        for (size_t a = 0; a < (size_t)n; ++a)
            if (!vis[a] && ag.q[s][a] > best_q) { best_q = ag.q[s][a]; next = a; }
        if (next == n)
            for (size_t a = 0; a < (size_t)n; ++a)
                if (!vis[a]) { next = a; break; }

        //ultimo passo sai do loop
        if (step == n) break;          

        //consumo e reabastecer 
        float km     = dist[s][next];
        float litros = km / CONSUMO_KM_POR_L;

        // se faltar combustivel para o trecho, guincho 
        if (litros > fuel) {
            total_cost += ag.tow_truck_cost;  
            fuel = ag.tank_capacity;           
        }

        fuel      -= litros;                   
        total_km  += km;

        if (fuel < ag.level_ref) {             // decide abastecer na cidade 
            float litros_abast = ag.tank_capacity - fuel;
            total_cost += litros_abast * cidades[s].preco;
            fuel = ag.tank_capacity;
        }

        s = next;                              // segue para a próxima cidade  
    }

    printf("Retorna a %s\n\n", cidades[0].nome);
    printf("Total percorrido : %.2f km\n", total_km);
    printf("Gasto em diesel  : R$ %.2f\n", total_cost);


}
