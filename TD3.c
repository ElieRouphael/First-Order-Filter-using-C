#include <stdio.h>

int filtrage(double *u, int taille_u, double *y, int taille_y, double K, double tau, double Te) {
    if (taille_u != taille_y) {
        return -1; // Erreur : les tailles des tableaux ne correspondent pas
    }

    // Initialisation de la première valeur de y
    y[0] = 0; // On peut initialiser à 0 ou à une autre valeur selon le besoin

    // Calcul du filtrage passe-bas
    for (int n = 1; n < taille_u; n++) {
        y[n] = y[n - 1] + (Te / tau) * (K * u[n] - y[n - 1]);
    }

    return 0; // Succès
}

int main() {
    // Paramètres du filtre
    double K = 2.0; // Gain statique
    double tau = 0.15; // Constante de temps
    double Te = 0.01; // Période d'échantillonnage
    double duree = 0.5; // Durée de l'échantillonnage
    int taille = (int)(duree / Te) + 1; // Nombre d'échantillons

    // Allocation des tableaux
    double u[taille]; // Signal d'entrée
    double y[taille]; // Signal de sortie

    // Remplissage du tableau d'entrée avec une réponse indicielle
    for (int i = 0; i < taille; i++) {
        u[i] = (i == 0) ? 1.0 : 0.0; // Impulsion à t=0
    }

    // Appel de la fonction de filtrage
    if (filtrage(u, taille, y, taille, K, tau, Te) != 0) {
        printf("Erreur lors du filtrage\n");
        return -1;
    }

    // Affichage des résultats
    printf("Temps (s)\tSignal d'entrée (u)\tSignal de sortie (y)\n");
    for (int i = 0; i < taille; i++) {
        printf("%.2f\t\t%.2f\t\t\t%.2f\n", i * Te, u[i], y[i]);
    }

    return 0;
}