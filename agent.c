#include "agent.h"
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <time.h>

/* utilitário: máximo da linha Q(s',·) ------------------------------------ */
static inline float max_q(const Agent *ag, size_t s_next){
    float best = -FLT_MAX;
    for (size_t a = 0; a < ag->n_cities; ++a)
        if (ag->q[s_next][a] > best)
            best = ag->q[s_next][a];
    return best;
}

void agent_clear_q(Agent *ag) { memset(ag->q, 0, sizeof(ag->q)); }

/* ----------------------------------------------------------------------- */
void agent_init(Agent              *ag,
                size_t              n_cities,
                const Cidade       *cities,
                const float         dist[][MAX_CITIES],
                float               tow_truck_cost,
                float               tank_capacity,
                ReinforcementFn     rfn,
                float               alpha, //lr
                float               gamma, //fator desconto
                float               epsilon) //fator exploracao
{
    srand((unsigned)time(NULL));

    ag->n_cities       = n_cities;
    ag->cities         = cities;          
    ag->dist           = dist;
    ag->tow_truck_cost = tow_truck_cost;
    ag->tank_capacity  = tank_capacity;
    ag->level_ref      = 0.25f * tank_capacity;
    ag->rfn            = rfn;

    ag->alpha   = alpha; 
    ag->gamma   = gamma;
    ag->epsilon = epsilon;

    agent_clear_q(ag);
    agent_reset_episode(ag, 0);
}

void agent_reset_episode(Agent *ag, size_t start_city){
    ag->current_city = start_city % ag->n_cities;
    ag->fuel_level   = ag->tank_capacity;
}

// devolve cidade aleatoria valida
static size_t random_city(const Agent *ag, size_t exclude){
    size_t c;
    do { c = (size_t)(rand() % ag->n_cities); } while (c == exclude);
    return c;
}

size_t agent_select_action(const Agent *ag){
    float r = (float)rand() / (float)RAND_MAX;
    if (r < ag->epsilon)                       // exploracao 
        return random_city(ag, ag->current_city);
    //busca do estado otimo
    size_t best_a = 0; //melhor acao
    float  best_q = -FLT_MAX; //melhor estado q
    for (size_t a = 0; a < ag->n_cities; ++a) {
        if (a == ag->current_city) continue;
        float qsa = ag->q[ag->current_city][a];
        if (qsa > best_q) { best_q = qsa; best_a = a; }
    }
    return best_a; //greedy
}

void agent_update_q(Agent *ag, size_t s_t, size_t a_t, float  reward, size_t s_tp1,int    use_sarsa){
    float target = reward +
                   ag->gamma *
                   (use_sarsa
                        ? ag->q[s_tp1][agent_select_action(ag)]//sarsa, e on-policy usa a encontrada sem futurologia
                        : max_q(ag, s_tp1)); //pega argmax, q-learn

    ag->q[s_t][a_t] += ag->alpha * (target - ag->q[s_t][a_t]);
}

//parametros e as funcoes pra calcular a recompensa
float agent_compute_reward(const Agent *ag, size_t from, size_t to, int used_tow_truck,float  litres_refuelled){
    float d   = ag->dist[from][to];
    float fc  = (float)ag->cities[to].preco * litres_refuelled;
    float tow = used_tow_truck ? ag->tow_truck_cost : 0.0f;

    switch (ag->rfn) {
        case R1_DISTANCE_FUEL: return -(d + fc);
        case R2_FUEL_ONLY:     return -fc;
        case R3_DISTANCE_ONLY: return -d;
        case R4_FUEL_TOW:      return -(fc + tow);
        case R5_FULL_COST:     return -(d + fc + tow);
        default:               return 0.0f;
    }
}

void agent_refuel(Agent *ag, float litres){
    if (litres <= 0.0f) ag->fuel_level = ag->tank_capacity;
    else {
        ag->fuel_level += litres;
        if (ag->fuel_level > ag->tank_capacity)
            ag->fuel_level = ag->tank_capacity;
    }
}
