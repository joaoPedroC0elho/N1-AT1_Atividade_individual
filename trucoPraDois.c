#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

// Estruturas de dados
typedef enum {
    ESPADAS, COPAS, OUROS, PAUS
} Naipe;

typedef struct {
    int valor;
    Naipe naipe;
} Carta;

typedef struct {
    Carta cartas[3];
    int num_cartas;
    int pontos;
} Jogador;

// Variaveis globais
Jogador jogador1, jogador2;
Carta monte[40];
int topo_monte;
int valor_rodada;
int rodada_atual;
int mao_atual;
bool aumento_pendente;
int ultimo_aumento_jogador;
Carta ultima_carta_jogador1, ultima_carta_jogador2;
int vitorias_mao[2]; // [0] = jogador1, [1] = jogador2
bool primeira_mao_empatada;

// Prototipos de funcoes
void inicializar_baralho();
void embaralhar();
void distribuir_cartas();
void iniciar_rodada();
void mostrar_estado_jogo(int jogador_atual);
void mostrar_carta(Carta c);
int comparar_cartas(Carta c1, Carta c2);
void jogar_turno(int jogador);
void processar_acao(int jogador, int acao);
void verificar_fim_mao();
void verificar_fim_rodada();
void verificar_fim_jogo();

// Funcoes principais
void inicializar_baralho() {
    int i = 0;
    for (Naipe n = ESPADAS; n <= PAUS; n++) {
        for (int v = 1; v <= 7; v++) {
            if (v == 8 || v == 9) continue;
            monte[i].valor = v;
            monte[i].naipe = n;
            i++;
        }
        for (int v = 10; v <= 12; v++) {
            monte[i].valor = v;
            monte[i].naipe = n;
            i++;
        }
    }
    topo_monte = 40;
}

void embaralhar() {
    srand(time(NULL));
    for (int i = 0; i < 1000; i++) {
        int a = rand() % 40;
        int b = rand() % 40;
        Carta temp = monte[a];
        monte[a] = monte[b];
        monte[b] = temp;
    }
}

void distribuir_cartas() {
    jogador1.num_cartas = 0;
    jogador2.num_cartas = 0;
    
    for (int i = 0; i < 3; i++) {
        jogador1.cartas[jogador1.num_cartas++] = monte[--topo_monte];
        jogador2.cartas[jogador2.num_cartas++] = monte[--topo_monte];
    }
}

void iniciar_rodada() {
    rodada_atual++;
    mao_atual = 1;
    valor_rodada = 1;
    aumento_pendente = false;
    primeira_mao_empatada = false;
    vitorias_mao[0] = 0;
    vitorias_mao[1] = 0;
    
    inicializar_baralho();
    embaralhar();
    distribuir_cartas();
    
    ultima_carta_jogador1.valor = 0;
    ultima_carta_jogador2.valor = 0;
}

int comparar_cartas(Carta c1, Carta c2) {
    // Cartas especiais
    if (c1.valor == 4 && c1.naipe == PAUS) return 1;  // Zap sempre ganha
    if (c2.valor == 4 && c2.naipe == PAUS) return -1;
    
    if (c1.valor == 7 && c1.naipe == COPAS) return 1; // 7 de copas
    if (c2.valor == 7 && c2.naipe == COPAS) return -1;
    
    if (c1.valor == 1 && c1.naipe == ESPADAS) return 1; // Espadao
    if (c2.valor == 1 && c2.naipe == ESPADAS) return -1;
    
    if (c1.valor == 1 && c1.naipe == PAUS) return 1;   // Pe
    if (c2.valor == 1 && c2.naipe == PAUS) return -1;
    
    // Ordem normal: 7, 6, 5, 4, 3, 2, 12, 11, 10
    int ordem[] = {0, 0, 6, 5, 4, 3, 2, 1, 0, 0, 9, 8, 7};
    
    int rank1 = ordem[c1.valor];
    int rank2 = ordem[c2.valor];
    
    if (rank1 > rank2) return 1;
    if (rank1 < rank2) return -1;
    return 0; // empate
}

void mostrar_carta(Carta c) {
    const char* naipes[] = {"Espadas", "Copas", "Ouros", "Paus"};
    const char* valores[] = {"", "1", "2", "3", "4", "5", "6", "7", "", "", "10", "11", "12"};
    
    printf("%s de %s", valores[c.valor], naipes[c.naipe]);
}

void mostrar_estado_jogo(int jogador_atual) {
    system("clear || cls");
    printf("=== TRUCO ===\n");
    printf("Rodada %d - Mao %d\n", rodada_atual, mao_atual);
    printf("Valor da rodada: %d ponto(s)\n", valor_rodada);
    printf("Jogador 1: %d pontos | Jogador 2: %d pontos\n\n", jogador1.pontos, jogador2.pontos);
    
    printf("Ultima carta jogada pelo adversario: ");
    if (jogador_atual == 1 && ultima_carta_jogador2.valor != 0) {
        mostrar_carta(ultima_carta_jogador2);
    } else if (jogador_atual == 2 && ultima_carta_jogador1.valor != 0) {
        mostrar_carta(ultima_carta_jogador1);
    } else {
        printf("Nenhuma");
    }
    printf("\n\n");
    
    printf("Suas cartas:\n");
    Jogador *j = (jogador_atual == 1) ? &jogador1 : &jogador2;
    for (int i = 0; i < j->num_cartas; i++) {
        printf("%d - ", i+1);
        mostrar_carta(j->cartas[i]);
        printf("\n");
    }
    printf("\n");
}

void jogar_turno(int jogador) {
    mostrar_estado_jogo(jogador);
    
    printf("Opcoes:\n");
    printf("1 - Jogar carta\n");
    if (!aumento_pendente || (aumento_pendente && ultimo_aumento_jogador != jogador)) {
        printf("2 - Aumentar\n");
    }
    if (aumento_pendente && ultimo_aumento_jogador != jogador) {
        printf("3 - Aceitar aumento\n");
        printf("4 - Desistir\n");
    }
    printf("0 - Terminar turno (se ja jogou carta)\n");
    
    int acao;
    do {
        printf("Escolha uma acao: ");
        scanf("%d", &acao);
    } while (acao < 0 || acao > 4);
    
    processar_acao(jogador, acao);
}

void processar_acao(int jogador, int acao) {
    Jogador *j = (jogador == 1) ? &jogador1 : &jogador2;
    Jogador *adversario = (jogador == 1) ? &jogador2 : &jogador1;
    
    switch (acao) {
        case 1: // Jogar carta
            if (aumento_pendente) {
                printf("Voce precisa primeiro aceitar ou recusar o aumento!\n");
                return;
            }
            
            int carta_idx;
            do {
                printf("Escolha uma carta (1-%d): ", j->num_cartas);
                scanf("%d", &carta_idx);
            } while (carta_idx < 1 || carta_idx > j->num_cartas);
            
            carta_idx--;
            Carta carta_jogada = j->cartas[carta_idx];
            
            // Remove carta da mao do jogador
            for (int i = carta_idx; i < j->num_cartas - 1; i++) {
                j->cartas[i] = j->cartas[i+1];
            }
            j->num_cartas--;
            
            if (jogador == 1) {
                ultima_carta_jogador1 = carta_jogada;
            } else {
                ultima_carta_jogador2 = carta_jogada;
            }
            
            printf("Voce jogou: ");
            mostrar_carta(carta_jogada);
            printf("\n");
            break;
            
        case 2: // Aumentar
            if (aumento_pendente && ultimo_aumento_jogador == jogador) {
                printf("Voce ja pediu aumento!\n");
                return;
            }
            
            int novo_valor;
            if (valor_rodada == 1) novo_valor = 3;
            else if (valor_rodada == 3) novo_valor = 6;
            else if (valor_rodada == 6) novo_valor = 9;
            else {
                printf("Nao e possivel aumentar mais!\n");
                return;
            }
            
            printf("Voce aumentou para %d pontos!\n", novo_valor);
            valor_rodada = novo_valor;
            aumento_pendente = true;
            ultimo_aumento_jogador = jogador;
            break;
            
        case 3: // Aceitar aumento
            if (!aumento_pendente || ultimo_aumento_jogador == jogador) {
                printf("Nao ha aumento para aceitar!\n");
                return;
            }
            
            printf("Voce aceitou o aumento para %d pontos!\n", valor_rodada);
            aumento_pendente = false;
            break;
            
        case 4: // Desistir
            printf("Voce desistiu da rodada. O adversario ganha %d ponto(s).\n", valor_rodada);
            adversario->pontos += valor_rodada;
            verificar_fim_jogo();
            iniciar_rodada();
            break;
            
        case 0: // Terminar turno
            if (aumento_pendente && ultimo_aumento_jogador != jogador) {
                printf("Voce precisa responder ao pedido de aumento!\n");
                return;
            }
            
            if (jogador == 1 && ultima_carta_jogador1.valor == 0 && 
                jogador == 2 && ultima_carta_jogador2.valor == 0) {
                printf("Voce precisa jogar uma carta primeiro!\n");
                return;
            }
            
            printf("Turno encerrado.\n");
            verificar_fim_mao();
            break;
    }
}

void verificar_fim_mao() {
    // Verifica se ambos jogaram cartas
    if (ultima_carta_jogador1.valor == 0 || ultima_carta_jogador2.valor == 0) {
        return;
    }
    
    int resultado = comparar_cartas(ultima_carta_jogador1, ultima_carta_jogador2);
    
    if (resultado > 0) {
        printf("Jogador 1 venceu a mao!\n");
        vitorias_mao[0]++;
    } else if (resultado < 0) {
        printf("Jogador 2 venceu a mao!\n");
        vitorias_mao[1]++;
    } else {
        printf("Mao empatada!\n");
        if (mao_atual == 1) {
            primeira_mao_empatada = true;
        }
    }
    
    // Resetar cartas jogadas
    ultima_carta_jogador1.valor = 0;
    ultima_carta_jogador2.valor = 0;
    
    // Verificar condicoes para fim da rodada
    if (mao_atual == 1 && vitorias_mao[0] + vitorias_mao[1] == 1) {
        // Primeira mao teve um vencedor
        mao_atual++;
    } else if (mao_atual == 1 && primeira_mao_empatada) {
        // Primeira mao empatou
        mao_atual++;
    } else if (mao_atual == 2) {
        verificar_fim_rodada();
    } else if (mao_atual == 3) {
        verificar_fim_rodada();
    }
}

void verificar_fim_rodada() {
    if (vitorias_mao[0] >= 2) {
        printf("Jogador 1 venceu a rodada e ganhou %d ponto(s)!\n", valor_rodada);
        jogador1.pontos += valor_rodada;
    } else if (vitorias_mao[1] >= 2) {
        printf("Jogador 2 venceu a rodada e ganhou %d ponto(s)!\n", valor_rodada);
        jogador2.pontos += valor_rodada;
    } else {
        printf("Rodada empatada! Nenhum ponto concedido.\n");
    }
    
    verificar_fim_jogo();
    iniciar_rodada();
}

void verificar_fim_jogo() {
    if (jogador1.pontos >= 12) {
        printf("Jogador 1 venceu o jogo com %d pontos!\n", jogador1.pontos);
        exit(0);
    } else if (jogador2.pontos >= 12) {
        printf("Jogador 2 venceu o jogo com %d pontos!\n", jogador2.pontos);
        exit(0);
    }
}

int main() {
    // Inicializacao do jogo
    jogador1.pontos = 0;
    jogador2.pontos = 0;
    rodada_atual = 0;
    
    iniciar_rodada();
    
    // Loop principal do jogo
    int jogador_atual = 1;
    while (1) {
        jogar_turno(jogador_atual);
        jogador_atual = (jogador_atual == 1) ? 2 : 1;
    }
    
    return 0;
}