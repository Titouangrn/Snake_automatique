#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <time.h>

// taille du serpent
#define TAILLE 10
// dimensions du plateau
#define LARGEUR_PLATEAU 80
#define HAUTEUR_PLATEAU 40
// position initiale de la tête du serpent
#define X_INITIAL_SERPENT1 40
#define Y_INITIAL_SERPENT1 13
#define X_INITIAL_SERPENT2 40
#define Y_INITIAL_SERPENT2 27
// nombre de pommes à manger pour gagner
#define NB_POMMES 10
// nombre de pavés sur le plateau
#define NB_PAVES 6
#define TAILLE_PAVE 5
// temporisation entre deux déplacements du serpent (en microsecondes)
#define ATTENTE 200000
// caractères pour représenter le serpent
#define CORPS 'X'
#define TETE1 '1'
#define TETE2 '2'
// touches de direction ou d'arrêt du jeu
#define HAUT 'z'
#define BAS 's'
#define GAUCHE 'q'
#define DROITE 'd'
#define STOP 'a'
// caractères pour les éléments du plateau
#define BORDURE '#'
#define VIDE ' '
#define POMME '6'

// définition d'un type pour le plateau : tPlateau
typedef char tPlateau[LARGEUR_PLATEAU + 1][HAUTEUR_PLATEAU + 1];
typedef int coord[2];

// Variables globales pour les pommes
int lesPommesX[NB_POMMES] = {40, 75, 78, 2, 9, 78, 74, 2, 72, 5};
int lesPommesY[NB_POMMES] = {20, 38, 2, 2, 5, 38, 32, 38, 32, 2};
int PommeX;
int PommeY;
// Variablbes pour les paves
int lesPavesX[NB_PAVES] = { 4, 73, 4, 73, 38, 38};
int lesPavesY[NB_PAVES] = { 4, 4, 33, 33, 14, 22};
int PaveX;
int PaveY;

char prochain = HAUT;

// Définition des coordonnées à utiliser plus tard
coord cible = {
    75,
    8
};
coord haut = {
    LARGEUR_PLATEAU / 2,
    1
};
coord bas = {
    LARGEUR_PLATEAU / 2,
    HAUTEUR_PLATEAU
};
coord gauche = {
    1,
    HAUTEUR_PLATEAU / 2
};
coord droite = {
    LARGEUR_PLATEAU,
    HAUTEUR_PLATEAU / 2
};
int nbDeplacements1 = 0;
int nbDeplacements2 = 0;
int nbpomme1 = 0;
int nbpomme2 = 0;

void initPlateau(tPlateau plateau);
void dessinerPlateau(tPlateau plateau);
void ajouterPomme(tPlateau plateau);
void afficher(int x, int y, char car);
void effacer(int x, int y);
void dessinerSerpent1(int lesX[], int lesY[]);
void dessinerSerpent2(int lesX[], int lesY[]);
void progresser1(int lesX[], int lesY[], char * direction1, tPlateau plateau, bool * collision, bool * pomme);
void progresser2(int lesX[], int lesY[], char * direction2, tPlateau plateau, bool * collision, bool * pomme);
void gotoxy(int x, int y);
int kbhit();
void disable_echo();
void enable_echo();
int distance(int x1, int y1, int x2, int y2);

int main() {
    int lesX1[TAILLE];
    int lesY1[TAILLE];
    int lesX2[TAILLE];
    int lesY2[TAILLE];
    PommeX = lesPommesX[0];
    PommeY = lesPommesY[0];
    cible[0] = PommeX;
    cible[1] = PommeY;
    int nbPommes = 0;
    char touche;
    char direction1;
    char direction2;
    int tour = 1;
    tPlateau lePlateau;
    bool collision = false;
    bool gagne = false;
    bool pommeMangee = false;

    for (int i = 0; i < TAILLE; i++) {
        lesX1[i] = X_INITIAL_SERPENT1 - i;
        lesY1[i] = Y_INITIAL_SERPENT1;
    }
    for (int i = 0; i < TAILLE; i++) {
        lesX2[i] = X_INITIAL_SERPENT2 + i;
        lesY2[i] = Y_INITIAL_SERPENT2;
    }

    initPlateau(lePlateau);
    system("clear");
    dessinerPlateau(lePlateau);

    srand(time(NULL));
    ajouterPomme(lePlateau);

    dessinerSerpent1(lesX1, lesY1);
    dessinerSerpent2(lesX2, lesY2);
    disable_echo();
    direction1 = DROITE;
    direction2 = GAUCHE;
    touche = DROITE;

  clock_t begin = clock();
    do {
        if (tour ==1){
            progresser1(lesX1, lesY1, & direction1, lePlateau, & collision, & pommeMangee);
            tour--;
            nbDeplacements1++;
            if(pommeMangee){
                nbpomme1++;
            }
        }
        else{
            progresser2(lesX2, lesY2, & direction2, lePlateau, & collision, & pommeMangee);
            tour++;
            nbDeplacements2++;
            if(pommeMangee){
                nbpomme2++;
            }
        }
        //Si une pomme est mangée, il affiche la suivante et véririfie si on a gagné ou
        if (pommeMangee) {
            nbPommes++;
            gagne = (nbPommes == NB_POMMES);
            PommeX = lesPommesX[nbPommes];
            PommeY = lesPommesY[nbPommes];
            if (!gagne) {
                ajouterPomme(lePlateau);
                pommeMangee = false;
            }
        }

        if (!gagne && !collision) {
            usleep(ATTENTE);
            if (kbhit() == 1) {
                touche = getchar();
            }
        }
  } while (touche != STOP && !collision && !gagne);

    clock_t end = clock();
    enable_echo();
    gotoxy(1, HAUTEUR_PLATEAU + 1);
    double tmpsCPU = ((end - begin) * 1.) / CLOCKS_PER_SEC;
    printf("Temps CPU : %.3f seconde(s)\n", tmpsCPU);
    printf("Serpent 1: nb déplacement = %d,nb pommes = %d.\n", nbDeplacements1,nbpomme1);
    printf("Serpent 2: nb déplacement = %d,nb pommes = %d.\n", nbDeplacements2,nbpomme2);
    return EXIT_SUCCESS;
}

void initPlateau(tPlateau plateau) {
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
            plateau[i][j] = VIDE;
        }
    }
    //Création des bordures
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        plateau[i][1] = BORDURE;
    }
    for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
        plateau[1][j] = BORDURE;
        plateau[LARGEUR_PLATEAU][j] = BORDURE;
    }
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        plateau[i][HAUTEUR_PLATEAU] = BORDURE;
    }
    //Création des portails dans le plateau
    plateau[LARGEUR_PLATEAU / 2][1] = VIDE;
    plateau[LARGEUR_PLATEAU / 2][HAUTEUR_PLATEAU] = VIDE;
    plateau[1][HAUTEUR_PLATEAU / 2] = VIDE;
    plateau[LARGEUR_PLATEAU][HAUTEUR_PLATEAU / 2] = VIDE;
}

void dessinerPlateau(tPlateau plateau) {
    // Dessin des bordures et cases vides
    for (int i = 1; i <= LARGEUR_PLATEAU; i++) {
        for (int j = 1; j <= HAUTEUR_PLATEAU; j++) {
        afficher(i, j, plateau[i][j]);
        }
    }

    // Dessin des pavés
    for (int i = 0; i < NB_PAVES; i++) {
        for (int j = 0; j < TAILLE_PAVE; j++) {
            for (int y = 0; y < TAILLE_PAVE; y++) {
                int x_pos = lesPavesX[i] + j;
                int y_pos = lesPavesY[i] + y;

                // Vérifier que les coordonnées sont valides
                if (x_pos <= LARGEUR_PLATEAU && y_pos <= HAUTEUR_PLATEAU) {
                    plateau[x_pos][y_pos] = BORDURE;
                    afficher(x_pos, y_pos, BORDURE);
                }
            }
        }
    }
}

void ajouterPomme(tPlateau plateau) {
    plateau[PommeX][PommeY] = POMME;
    afficher(PommeX, PommeY, POMME);
}

void afficher(int x, int y, char car) {
    gotoxy(x, y);
    printf("%c", car);
    gotoxy(1, 1);
}

void effacer(int x, int y) {
    gotoxy(x, y);
    printf(" ");
    gotoxy(1, 1);
}

void dessinerSerpent1(int lesX[], int lesY[]) {
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE1);
}
void dessinerSerpent2(int lesX[], int lesY[]) {
    for (int i = 1; i < TAILLE; i++) {
        afficher(lesX[i], lesY[i], CORPS);
    }
    afficher(lesX[0], lesY[0], TETE2);
}

void progresser1(int lesX[], int lesY[], char * direction, tPlateau plateau, bool * collision, bool * pomme) {

    //Calcul de la distance directe ou par un portail entre la pomme et la tête du serpent
    int distPomme = distance(lesX[0], lesY[0], PommeX, PommeY);
    int portHaut = distance(lesX[0], lesY[0], haut[0], haut[1]) + distance(bas[0], bas[1], PommeX, PommeY);
    int portBas = distance(lesX[0], lesY[0], bas[0], bas[1]) + distance(haut[0], haut[1], PommeX, PommeY);
    int portGauche = distance(lesX[0], lesY[0], gauche[0], gauche[1]) + distance(droite[0], droite[1], PommeX, PommeY);
    int portDroite = distance(lesX[0], lesY[0], droite[0], droite[1]) + distance(gauche[0], gauche[1], PommeX, PommeY);

    /*Si la distance de la tête vers la pomme est plus petite en passant par l'un 
    des portails, il passe par celui ci*/
    if (portHaut < distPomme) {
        cible[0] = haut[0];
        cible[1] = haut[1] + 1;
        if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
            cible[1] = haut[1];
        }
    } else if (portBas < distPomme) {
        cible[0] = bas[0];
        cible[1] = bas[1] - 1;
            if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[1] = bas[1];
        }
    } else if (portGauche < distPomme) {
        cible[0] = gauche[0] + 1;
        cible[1] = gauche[1];
            if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[0] = gauche[0];
        }
    } else if (portDroite < distPomme) {
        cible[0] = droite[0] - 1;

        cible[1] = droite[1];
            if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[0] = droite[0];
        }
    }
    /*Sinon la pomme reste la cible*/
    else {
        cible[0] = PommeX;
        cible[1] = PommeY;

    }

    /*Si le serpent entre dans un portail, il réapparait du côté opposé
    et sa tête est effacée du trou.*/
    if (lesX[0] == 1 && lesY[0] == HAUTEUR_PLATEAU / 2) {
        effacer(lesX[0], lesY[0]);
        lesX[0] = LARGEUR_PLATEAU;
    } 
    else if (lesX[0] == LARGEUR_PLATEAU && lesY[0] == HAUTEUR_PLATEAU / 2) {
        effacer(lesX[0], lesY[0]);
        lesX[0] = 1;
    } 
    else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 1) {
        effacer(lesX[0], lesY[0]);
        lesY[0] = HAUTEUR_PLATEAU;
    } 
    else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU) {
        effacer(lesX[0], lesY[0]);
        lesY[0] = 1;
    }

    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);
    plateau[lesX[TAILLE - 1]][lesY[TAILLE - 1]] = ' '; //enleve le dernier bout du serpent dans le tableau

    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
        plateau[lesX[i]][lesY[i]] = CORPS;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (lesX[0] < cible[0]) {
        if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
            lesX[0]++;
        } 
        else {
            if (lesY[0] <= cible[1]) {
                if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
                    lesY[0]++;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]--;
                } 
                else{
                    lesX[0]--;
                }
                }
            else {
                if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else{
                    lesX[0]++;
                }
            }
        }
    }
    else if (lesX[0] > cible[0]) {
        if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
            lesX[0]--;
        } 
        else {
            if (lesY[0] <= cible[1]) {
                if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
                lesY[0]++;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else{
                    lesX[0]--;
                }
        }
            else {
                if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else{
                    lesX[0]++;
                }
            }
        }
    }
    else if (lesY[0] < cible[1]) {
        if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
            lesY[0]++;
        } 
        else {
            if(lesX[0] <= cible[0]) {
                if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
                    lesX[0]++ ;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else{
                    lesY[0]--;
                }
            }
            else {
                if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else{
                    lesY[0]++;
                }
            }
        }
    }
    else if (lesY[0] > cible[1]) {
        if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
            lesY[0]--;
        } 
        else {
            if (lesX[0] <= cible[0]) {
                if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){///////////////////////////////////////////////////
                    lesY[0]++;
                } 
                else{
                    lesY[0]--;
                }
            }
            else {
                if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else{
                    lesY[0]++;
                }
            }
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    /* choisi la direction ou le serpent doit aller en empechant les demi tours et
    en prennant en compte les son propre corps*/

    * pomme = false;

    if (plateau[lesX[0]][lesY[0]] == POMME) {
        * pomme = true;
        plateau[lesX[0]][lesY[0]] = VIDE;
    } 
    else if (plateau[lesX[0]][lesY[0]] == CORPS || plateau[lesX[0]][lesY[0]] == BORDURE) {
        * collision = true;
    }
    dessinerSerpent1(lesX, lesY);
}




void progresser2(int lesX[], int lesY[], char * direction, tPlateau plateau, bool * collision, bool * pomme) {

    //Calcul de la distance directe ou par un portail entre la pomme et la tête du serpent
    int distPomme = distance(lesX[0], lesY[0], PommeX, PommeY);
    int portHaut = distance(lesX[0], lesY[0], haut[0], haut[1]) + distance(bas[0], bas[1], PommeX, PommeY);
    int portBas = distance(lesX[0], lesY[0], bas[0], bas[1]) + distance(haut[0], haut[1], PommeX, PommeY);
    int portGauche = distance(lesX[0], lesY[0], gauche[0], gauche[1]) + distance(droite[0], droite[1], PommeX, PommeY);
    int portDroite = distance(lesX[0], lesY[0], droite[0], droite[1]) + distance(gauche[0], gauche[1], PommeX, PommeY);

    /*Si la distance de la tête vers la pomme est plus petite en passant par l'un 
    des portails, il passe par celui ci*/
    if (portHaut < distPomme) {
        cible[0] = haut[0];
        cible[1] = haut[1] + 1;
        if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[1] = haut[1];
        }
    } else if (portBas < distPomme) {
        cible[0] = bas[0];
        cible[1] = bas[1] - 1;
        if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[1] = bas[1];
        }
    } else if (portGauche < distPomme) {
        cible[0] = gauche[0] + 1;
        cible[1] = gauche[1];
        if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[0] = gauche[0];
        }
    } else if (portDroite < distPomme) {
        cible[0] = droite[0] - 1;

        cible[1] = droite[1];
        if (lesX[0] == cible[0] && lesY[0] == cible[1]) {
        cible[0] = droite[0];
        }
    }
    /*Sinon la pomme reste la cible*/
    else {
        cible[0] = PommeX;
        cible[1] = PommeY;

    }

    /*Si le serpent entre dans un portail, il réapparait du côté opposé
    et sa tête est effacée du trou.*/
    if (lesX[0] == 1 && lesY[0] == HAUTEUR_PLATEAU / 2) {
        effacer(lesX[0], lesY[0]);
        lesX[0] = LARGEUR_PLATEAU;
    } else if (lesX[0] == LARGEUR_PLATEAU && lesY[0] == HAUTEUR_PLATEAU / 2) {
        effacer(lesX[0], lesY[0]);
        lesX[0] = 1;
    } else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == 1) {
        effacer(lesX[0], lesY[0]);
        lesY[0] = HAUTEUR_PLATEAU;
    } else if (lesX[0] == LARGEUR_PLATEAU / 2 && lesY[0] == HAUTEUR_PLATEAU) {
        effacer(lesX[0], lesY[0]);
        lesY[0] = 1;
    }

    effacer(lesX[TAILLE - 1], lesY[TAILLE - 1]);
    plateau[lesX[TAILLE - 1]][lesY[TAILLE - 1]] = ' '; //enleve le dernier bout du serpent dans le tableau

    for (int i = TAILLE - 1; i > 0; i--) {
        lesX[i] = lesX[i - 1];
        lesY[i] = lesY[i - 1];
        plateau[lesX[i]][lesY[i]] = CORPS;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if (lesX[0] < cible[0]) {
        if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
            lesX[0]++;
        } 
        else {
            if (lesY[0] <= cible[1]) {
                if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
                    lesY[0]++;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]--;
                } 
                else{
                    lesX[0]--;
                }
            }
            else {
                if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else{
                    lesX[0]++;
                }
            }
        }
    }
    else if (lesX[0] > cible[0]) {
        if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
            lesX[0]--;
        } 
        else {
            if (lesY[0] <= cible[1]) {
                if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
                    lesY[0]++;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else{
                    lesX[0]--;
                }
            }
            else {
                if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else{
                    lesX[0]++;
                }
            }
        }
    }
    else if (lesY[0] < cible[1]) {
        if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS) {
            lesY[0]++;
        } 
        else {
            if(lesX[0] <= cible[0]) {
                if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
                    lesX[0]++ ;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){
                    lesY[0]++;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else{
                    lesY[0]--;
                }
            }
            else {
                if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else{
                    lesY[0]++;
                }
            }
        }
    }
    else if (lesY[0] > cible[1]) {
        if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS) {
            lesY[0]--;
        } 
        else {
            if (lesX[0] <= cible[0]) {
                if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS) {
                    lesX[0]++;
                } 
                else if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS){
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]+1] != BORDURE && plateau[lesX[0]][lesY[0]+1] != CORPS){///////////////////////////////////////////////////
                    lesY[0]++;
                } 
                else{
                    lesY[0]--;
                }
            }
            else {
                if (plateau[lesX[0]-1][lesY[0]] != BORDURE && plateau[lesX[0]-1][lesY[0]] != CORPS) {
                    lesX[0]--;
                } 
                else if (plateau[lesX[0]][lesY[0]-1] != BORDURE && plateau[lesX[0]][lesY[0]-1] != CORPS){
                    lesY[0]--;
                } 
                else if (plateau[lesX[0]+1][lesY[0]] != BORDURE && plateau[lesX[0]+1][lesY[0]] != CORPS){
                    lesX[0]++;
                } 
                else{
                    lesY[0]++;
                }
            }
        }
    }


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    /* choisi la direction ou le serpent doit aller en empechant les demi tours et
    en prennant en compte les son propre corps*/

    * pomme = false;

    if (plateau[lesX[0]][lesY[0]] == POMME) {
        * pomme = true;
        plateau[lesX[0]][lesY[0]] = VIDE;
    } else if (plateau[lesX[0]][lesY[0]] == CORPS || plateau[lesX[0]][lesY[0]] == BORDURE) {
        * collision = true;
    }
    dessinerSerpent2(lesX, lesY);
}




void gotoxy(int x, int y) {
    printf("\033[%d;%df", y, x);
}

int kbhit() {
    int unCaractere = 0;
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, & oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, & newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, & oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        unCaractere = 1;
    }
    return unCaractere;
}

void disable_echo() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, & tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, & tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

void enable_echo() {
    struct termios tty;
    if (tcgetattr(STDIN_FILENO, & tty) == -1) {
        perror("tcgetattr");
        exit(EXIT_FAILURE);
    }
    tty.c_lflag |= ECHO;
    if (tcsetattr(STDIN_FILENO, TCSANOW, & tty) == -1) {
        perror("tcsetattr");
        exit(EXIT_FAILURE);
    }
}

int distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}
