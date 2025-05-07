#ifndef AGENT_TSPWR_H
#define AGENT_TSPWR_H

#include <stddef.h>
#include "cidades.h"   

/* limite prático de cidades ------------------------------------------------ */
#define MAX_CITIES 89

/* funções‑de‑reforço (Ottoni et al., 2022) --------------------------------- */
typedef enum {
    R1_DISTANCE_FUEL,
    R2_FUEL_ONLY,
    R3_DISTANCE_ONLY,
    R4_FUEL_TOW,
    R5_FULL_COST
} ReinforcementFn;

// struct do agente
typedef struct {
    size_t      n_cities;                       // |S|                        
    const Cidade *cities;                       // ponteiro p vetor de cidades
    const float (*dist)[MAX_CITIES];            // matriz de distancias    
    float  tow_truck_cost;                      // custo de guincho           
    float  tank_capacity;                       // capacidade do tanque (L)   
    float  level_ref;                           // 25 % do tanque, quando dar refil             
    ReinforcementFn rfn;                        // funcao‑de‑reforço que vai usar    

    // hiperparâmetros de aprendizado 
    float alpha, gamma, epsilon;

    // estad atual do agente
    size_t current_city;
    float  fuel_level;

    // q-table
    float q[MAX_CITIES][MAX_CITIES];
} Agent;

void agent_init(Agent *ag,
    size_t n_cities,
    const Cidade *cities,
    const float dist[][MAX_CITIES],
    float tow_truck_cost,
    float tank_capacity,
    ReinforcementFn rfn,
    float alpha,
    float gamma,
    float epsilon);

void  agent_reset_episode(Agent *ag, size_t start_city);
size_t agent_select_action(const Agent *ag);

void  agent_update_q(Agent *ag,
                    size_t s_t,
                     size_t a_t,
                     float  reward,
                     size_t s_tp1,
                     int    use_sarsa);          // 0 = q‑learning; 1 = sarsa

float agent_compute_reward(const Agent *ag,
                           size_t from,
                           size_t to,
                           int    used_tow_truck,
                           float  litres_refuelled);

void  agent_refuel(Agent *ag, float litres);
void  agent_clear_q(Agent *ag);

#endif /* AGENT_TSPWR_H */
