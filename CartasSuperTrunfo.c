#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <locale.h>

/*
   Super Trunfo - Países (Nível Mestre)

   O programa:
   - Lê os dados de 2 cartas.
   - Calcula densidade populacional e PIB per Capita.
   - Calcula Super Poder (inclui inverso da densidade).
   - Compara carta 1 vs carta 2 atributo por atributo.

   Melhorias (sem fugir do enunciado):
   - Leitura robusta com fgets (aceita espaços no nome da cidade).
   - Validação para impedir letras em campos numéricos e vice-versa.
   - Proteção contra divisão por zero.
   - setlocale para suportar acentuação quando o ambiente/terminal permitir.
*/

typedef struct {
   char estado;                 /* 'A'..'H' */
   char codigo[4];              /* ex: A01 */
   char nomeCidade[100];
   unsigned long int populacao; /* Mestre: unsigned long int */
   float area;                  /* km² */
   float pib;                   /* em bilhões de reais */
   int pontosTuristicos;

   float densidadePopulacional;
   float pibPerCapita;          /* em reais */
   float superPoder;
} Carta;

static void strip_newline(char *s) {
   size_t n = strlen(s);
   while (n > 0 && (s[n - 1] == '\n' || s[n - 1] == '\r')) {
      s[n - 1] = '\0';
      n--;
   }
}

static void read_line(const char *prompt, char *buf, size_t bufSize) {
   for (;;) {
      printf("%s", prompt);
      if (fgets(buf, (int)bufSize, stdin) == NULL) {
         printf("\nEntrada encerrada (EOF). Finalizando.\n");
         exit(1);
      }
      strip_newline(buf);
      if (buf[0] != '\0') return;
      printf("Entrada vazia. Tente novamente.\n");
   }
}

static char read_estado(const char *prompt) {
   char line[64];
   for (;;) {
      read_line(prompt, line, sizeof(line));
      if (line[0] >= 'A' && line[0] <= 'H' && line[1] == '\0') {
         return line[0];
      }
      if (line[0] >= 'a' && line[0] <= 'h' && line[1] == '\0') {
         return (char)(line[0] - ('a' - 'A'));
      }
      printf("Valor invalido. Digite uma letra de A a H.\n");
   }
}

static void read_codigo(const char *prompt, char estadoEsperado, char out[4]) {
   char line[64];
   for (;;) {
      read_line(prompt, line, sizeof(line));
      if (strlen(line) == 3) {
         char s0 = line[0];
         if (s0 >= 'a' && s0 <= 'h') s0 = (char)(s0 - ('a' - 'A'));

         if (s0 == estadoEsperado &&
            line[1] >= '0' && line[1] <= '9' &&
            line[2] >= '0' && line[2] <= '9') {
            int n = (line[1] - '0') * 10 + (line[2] - '0');
            if (n >= 1 && n <= 4) {
               out[0] = s0;
               out[1] = line[1];
               out[2] = line[2];
               out[3] = '\0';
               return;
            }
         }
      }
      printf("Codigo invalido. Use o formato %c01 a %c04 (ex: %c01).\n", estadoEsperado, estadoEsperado, estadoEsperado);
   }
}

static unsigned long read_ulong(const char *prompt) {
   char line[128];
   for (;;) {
      read_line(prompt, line, sizeof(line));
      errno = 0;
      char *end = NULL;
      unsigned long v = strtoul(line, &end, 10);
      while (end && (*end == ' ' || *end == '\t')) end++;
      if (errno == 0 && end && *end == '\0') return v;
      printf("Valor invalido. Digite um numero inteiro (ex: 123).\n");
   }
}

static int read_int(const char *prompt) {
   char line[128];
   for (;;) {
      read_line(prompt, line, sizeof(line));
      errno = 0;
      char *end = NULL;
      long v = strtol(line, &end, 10);
      while (end && (*end == ' ' || *end == '\t')) end++;
      if (errno == 0 && end && *end == '\0') return (int)v;
      printf("Valor invalido. Digite um numero inteiro (ex: 50).\n");
   }
}

static float read_float(const char *prompt) {
   char line[128];
   for (;;) {
      read_line(prompt, line, sizeof(line));
      errno = 0;
      char *end = NULL;
      float v = strtof(line, &end);
      while (end && (*end == ' ' || *end == '\t')) end++;
      if (errno == 0 && end && *end == '\0') return v;
      printf("Valor invalido. Digite um numero (use ponto, ex: 12.5).\n");
   }
}

static void cadastrar_carta(int idx, Carta *c) {
   char prompt[128];

   printf("=== Cadastro da Carta %d ===\n", idx);

   c->estado = read_estado("Estado (A a H): ");
   snprintf(prompt, sizeof(prompt), "Codigo da Carta (ex: %c01): ", c->estado);
   read_codigo(prompt, c->estado, c->codigo);

   read_line("Nome da Cidade: ", c->nomeCidade, sizeof(c->nomeCidade));

   c->populacao = (unsigned long int)read_ulong("Populacao: ");

   /* Para evitar divisao por zero, exigimos area > 0 */
   for (;;) {
      c->area = read_float("Area (km2): ");
      if (c->area > 0.0f) break;
      printf("Area invalida. Digite um valor maior que 0.\n");
   }

   c->pib = read_float("PIB (em bilhoes de reais): ");
   c->pontosTuristicos = read_int("Numero de Pontos Turisticos: ");

   if (c->populacao == 0) {
      c->densidadePopulacional = 0.0f;
      c->pibPerCapita = 0.0f;
   } else {
      c->densidadePopulacional = (float)c->populacao / c->area;
      c->pibPerCapita = (c->pib * 1000000000.0f) / (float)c->populacao;
   }

   /* Super Poder: soma + inverso da densidade (quanto menor a densidade, maior) */
   {
      float invDensidade = 0.0f;
      if (c->densidadePopulacional > 0.0f) invDensidade = 1.0f / c->densidadePopulacional;

      c->superPoder =
         (float)c->populacao +
         c->area +
         c->pib +
         (float)c->pontosTuristicos +
         c->pibPerCapita +
         invDensidade;
   }
}

static void imprimir_carta(int idx, const Carta *c) {
   printf("\n==============================\n");
   printf("Carta %d:\n", idx);
   printf("Estado: %c\n", c->estado);
   printf("Codigo: %s\n", c->codigo);
   printf("Nome da Cidade: %s\n", c->nomeCidade);
   printf("Populacao: %lu\n", (unsigned long)c->populacao);
   printf("Area: %.2f km2\n", c->area);
   printf("PIB: %.2f bilhoes de reais\n", c->pib);
   printf("Numero de Pontos Turisticos: %d\n", c->pontosTuristicos);
   printf("Densidade Populacional: %.2f hab/km2\n", c->densidadePopulacional);
   printf("PIB per Capita: %.2f reais\n", c->pibPerCapita);
   printf("Super Poder: %.2f\n", c->superPoder);
}

static void imprimir_resultado(const char *label, int carta1Vence) {
   printf("%s: Carta %d venceu (%d)\n", label, carta1Vence ? 1 : 2, carta1Vence ? 1 : 0);
}

int main(void) {
   setlocale(LC_ALL, "");

   Carta c1;
   Carta c2;

   cadastrar_carta(1, &c1);
   printf("\n");
   cadastrar_carta(2, &c2);

   imprimir_carta(1, &c1);
   imprimir_carta(2, &c2);

   printf("\n==============================\n");
   printf("Comparacao de Cartas:\n\n");

   /* 1 = Carta 1 vence, 0 = Carta 2 vence */
   imprimir_resultado("Populacao", c1.populacao > c2.populacao);
   imprimir_resultado("Area", c1.area > c2.area);
   imprimir_resultado("PIB", c1.pib > c2.pib);
   imprimir_resultado("Pontos Turisticos", c1.pontosTuristicos > c2.pontosTuristicos);

   /* Densidade: menor vence */
   imprimir_resultado("Densidade Populacional", c1.densidadePopulacional < c2.densidadePopulacional);

   imprimir_resultado("PIB per Capita", c1.pibPerCapita > c2.pibPerCapita);
   imprimir_resultado("Super Poder", c1.superPoder > c2.superPoder);

   return 0;
}