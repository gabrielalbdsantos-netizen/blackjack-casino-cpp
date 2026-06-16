#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <string>
#include <thread>
#include <chrono>

// ============================================================
//  CARTA
// ============================================================
struct Carta {
    std::string valor;
    std::string naipe;

    std::string exibir() const {
        return "[" + valor + " " + naipe + "]";
    }

    int pontos() const {
        if (valor == "A")                          return 11;
        if (valor == "J" || valor == "Q" || valor == "K") return 10;
        return std::stoi(valor);
    }
};

// ============================================================
//  BARALHO
// ============================================================
class Baralho {
    std::vector<Carta> cartas;
    std::mt19937 rng;

public:
    Baralho() : rng(std::random_device{}()) {
        std::vector<std::string> valores = {"2","3","4","5","6","7","8","9","10","J","Q","K","A"};
        std::vector<std::string> naipes  = {"♠","♥","♦","♣"};
        for (auto& n : naipes)
            for (auto& v : valores)
                cartas.push_back({v, n});
        embaralhar();
    }

    void embaralhar() {
        std::shuffle(cartas.begin(), cartas.end(), rng);
    }

    Carta comprar() {
        if (cartas.empty()) {
            // Reembaralha se acabar
            *this = Baralho();
        }
        Carta c = cartas.back();
        cartas.pop_back();
        return c;
    }
};

// ============================================================
//  MÃO DO JOGADOR
// ============================================================
class Mao {
    std::vector<Carta> cartas;

public:
    void adicionar(Carta c) { cartas.push_back(c); }

    void limpar() { cartas.clear(); }

    int pontuacao() const {
        int total = 0;
        int ases  = 0;
        for (auto& c : cartas) {
            total += c.pontos();
            if (c.valor == "A") ases++;
        }
        while (total > 21 && ases > 0) {
            total -= 10;
            ases--;
        }
        return total;
    }

    bool estourou() const { return pontuacao() > 21; }
    bool blackjack() const { return cartas.size() == 2 && pontuacao() == 21; }
    int  tamanho()   const { return (int)cartas.size(); }

    void exibir(bool esconderPrimeira = false) const {
        for (int i = 0; i < (int)cartas.size(); i++) {
            if (i == 0 && esconderPrimeira)
                std::cout << "[???] ";
            else
                std::cout << cartas[i].exibir() << " ";
        }
    }
};

// ============================================================
//  UTILITÁRIOS DE TELA
// ============================================================
void pausa(int ms = 800) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void cabecalho() {
    std::cout << "\n";
    std::cout << "  ╔══════════════════════════════════════╗\n";
    std::cout << "  ║         ♠  BLACKJACK 21  ♠           ║\n";
    std::cout << "  ╚══════════════════════════════════════╝\n\n";
}

void linha() {
    std::cout << "  ----------------------------------------\n";
}

// ============================================================
//  JOGO PRINCIPAL
// ============================================================
class Blackjack {
    Baralho baralho;
    Mao     maoJogador;
    Mao     maoDealer;
    int     fichasJogador;
    int     apostas;

    void exibirMesas(bool dealerEscondido) {
        std::cout << "\n  DEALER: ";
        maoDealer.exibir(dealerEscondido);
        if (!dealerEscondido)
            std::cout << "  => " << maoDealer.pontuacao() << " pts";
        std::cout << "\n";

        linha();

        std::cout << "  VOCE:   ";
        maoJogador.exibir();
        std::cout << "  => " << maoJogador.pontuacao() << " pts\n\n";
    }

    int pedirAposta() {
        int aposta = 0;
        while (true) {
            std::cout << "  Suas fichas: " << fichasJogador << "\n";
            std::cout << "  Quanto quer apostar? ";
            std::cin >> aposta;
            if (aposta > 0 && aposta <= fichasJogador) return aposta;
            std::cout << "  Aposta invalida! Tente novamente.\n";
        }
    }

    char pedirAcao() {
        char acao;
        while (true) {
            std::cout << "  [H] Pedir carta   [S] Parar   [D] Dobrar: ";
            std::cin >> acao;
            acao = toupper(acao);
            if (acao == 'H' || acao == 'S' || acao == 'D') return acao;
            std::cout << "  Opcao invalida!\n";
        }
    }

    void turnoDealer() {
        std::cout << "\n  -- Vez do Dealer --\n";
        pausa();
        exibirMesas(false);

        // Dealer compra até ter >= 17
        while (maoDealer.pontuacao() < 17) {
            std::cout << "  Dealer compra carta...\n";
            pausa(900);
            maoDealer.adicionar(baralho.comprar());
            exibirMesas(false);
        }
    }

    void resolverRodada() {
        int pJ = maoJogador.pontuacao();
        int pD = maoDealer.pontuacao();

        linha();
        std::cout << "\n  === RESULTADO ===\n\n";

        if (maoJogador.blackjack() && !maoDealer.blackjack()) {
            std::cout << "  ★ BLACKJACK! Voce ganhou!\n";
            fichasJogador += (int)(apostas * 1.5);
        } else if (maoDealer.blackjack() && !maoJogador.blackjack()) {
            std::cout << "  Dealer fez Blackjack. Voce perdeu!\n";
            fichasJogador -= apostas;
        } else if (maoJogador.estourou()) {
            std::cout << "  Voce estourou! Perdeu " << apostas << " fichas.\n";
            fichasJogador -= apostas;
        } else if (maoDealer.estourou()) {
            std::cout << "  Dealer estourou! Voce ganhou " << apostas << " fichas!\n";
            fichasJogador += apostas;
        } else if (pJ > pD) {
            std::cout << "  Voce venceu! +" << apostas << " fichas.\n";
            fichasJogador += apostas;
        } else if (pD > pJ) {
            std::cout << "  Dealer venceu. -" << apostas << " fichas.\n";
            fichasJogador -= apostas;
        } else {
            std::cout << "  Empate! Fichas devolvidas.\n";
        }

        std::cout << "  Fichas restantes: " << fichasJogador << "\n\n";
    }

public:
    Blackjack() : fichasJogador(500), apostas(0) {}

    void jogar() {
        cabecalho();
        std::cout << "  Bem-vindo ao Blackjack!\n";
        std::cout << "  Voce comeca com 500 fichas.\n\n";

        while (fichasJogador > 0) {
            linha();
            std::cout << "\n  Nova rodada!\n\n";

            // Limpar mãos
            maoJogador.limpar();
            maoDealer.limpar();

            // Aposta
            apostas = pedirAposta();
            std::cout << "\n";

            // Distribuir cartas iniciais
            maoJogador.adicionar(baralho.comprar());
            maoDealer.adicionar(baralho.comprar());
            maoJogador.adicionar(baralho.comprar());
            maoDealer.adicionar(baralho.comprar());

            exibirMesas(true);

            // Turno do jogador
            bool terminou = false;
            while (!terminou && !maoJogador.estourou()) {
                if (maoJogador.blackjack()) break;

                char acao = pedirAcao();

                if (acao == 'H') {
                    maoJogador.adicionar(baralho.comprar());
                    std::cout << "\n";
                    exibirMesas(true);
                } else if (acao == 'D') {
                    if (apostas * 2 <= fichasJogador) {
                        apostas *= 2;
                        std::cout << "  Aposta dobrada para " << apostas << "!\n\n";
                        maoJogador.adicionar(baralho.comprar());
                        exibirMesas(true);
                        terminou = true;
                    } else {
                        std::cout << "  Fichas insuficientes para dobrar!\n";
                    }
                } else {
                    terminou = true;
                }
            }

            // Turno do dealer (só se jogador não estourou)
            if (!maoJogador.estourou() && !maoJogador.blackjack()) {
                turnoDealer();
            } else {
                std::cout << "\n  Dealer revela: ";
                maoDealer.exibir(false);
                std::cout << "=> " << maoDealer.pontuacao() << " pts\n";
            }

            resolverRodada();

            if (fichasJogador <= 0) {
                std::cout << "  Voce ficou sem fichas! Game Over.\n\n";
                break;
            }

            char continuar;
            std::cout << "  Jogar novamente? [S/N]: ";
            std::cin >> continuar;
            if (toupper(continuar) != 'S') break;
        }

        cabecalho();
        std::cout << "  Obrigado por jogar! Fichas finais: "
                  << fichasJogador << "\n\n";
    }
};

// ============================================================
//  MAIN
// ============================================================
int main() {
    Blackjack jogo;
    jogo.jogar();
    return 0;
}