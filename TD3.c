#include <stdio.h>
#include <math.h>

int filtrage(double *u, int taille_u, double *y, int taille_y, double K, double tau, double Te) {
    if (taille_u != taille_y) {
        return -1; // Erreur : les tailles des tableaux ne correspondent pas
    }

    // Calcul de z0
    double z0 = exp(-Te / tau);

    // Initialisation de la première valeur de y
    y[0] = 0; // On peut initialiser à 0 ou à une autre valeur selon le besoin

    // Calcul du filtrage passe-bas
    for (int k = 1; k < taille_u; k++) {
        y[k] = z0 * y[k - 1] + K * (1 - z0) * u[k - 1];
    }

    return 0; // Succès
}

/*int LireEntree(const char *nom_fichier, double *valeurs) {
    FILE *fichier = fopen(nom_fichier, "r"); // Ouvrir le fichier en mode lecture
    if (fichier == NULL) {
        return -1; // Erreur d'ouverture du fichier
    }

    int compteur = 0; // Compteur pour le nombre de valeurs lues
    while (compteur < 10000 && fscanf(fichier, "%lf", &valeurs[compteur]) == 1) {
        compteur++;
    }

    fclose(fichier); // Fermer le fichier
    return compteur; // Retourner le nombre de valeurs lues
}*/

int LireEntree(const char *nom_fichier, double *valeurs) {
    FILE *fichier = fopen(nom_fichier, "r"); // Ouvrir le fichier en mode lecture
    if (fichier == NULL) {
        return -1; // Erreur d'ouverture du fichier
    }

    int compteur = 0; // Compteur pour le nombre de valeurs lues
    while (compteur < 10000) {
        // Lire une valeur
        if (fscanf(fichier, "%lf", &valeurs[compteur]) != 1) {
            // Vérifier si on a atteint la fin du fichier
            if (feof(fichier)) {
                break; // Fin de fichier atteinte
            } else {
                fclose(fichier); // Fermer le fichier en cas d'erreur
                return -1; // Erreur de lecture
            }
        }
        compteur++;
    }

    fclose(fichier); // Fermer le fichier
    return compteur; // Retourner le nombre de valeurs lues
}

int EcrireSortie(const char *nom_fichier, double *valeurs, int taille) {
    FILE *fichier = fopen(nom_fichier, "w"); // Ouvrir le fichier en mode écriture
    if (fichier == NULL) {
        return -1; // Erreur d'ouverture du fichier
    }

    int compteur;
    for (compteur = 0; compteur < taille; compteur++) {
        // Écrire chaque valeur dans le fichier suivie d'un saut de ligne
        if (fprintf(fichier, "%.2f\n", valeurs[compteur]) < 0) {
            fclose(fichier); // Fermer le fichier en cas d'erreur
            return -1; // Erreur d'écriture
        }
    }

    fclose(fichier); // Fermer le fichier
    return compteur; // Retourner le nombre de valeurs écrites
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







    double valeurs[10000]; // Tableau pour stocker les valeurs
    const char *nom_fichier = "enregistrement-bruit.txt"; // Nom du fichier à lire

    int nombre_valeurs = LireEntree(nom_fichier, valeurs);
    if (nombre_valeurs == -1) {
        printf("Erreur lors de la lecture du fichier.\n");
        return -1;
    }

    // Affichage des valeurs lues
    printf("Valeurs lues (%d valeurs) :\n", nombre_valeurs);
    for (int i = 0; i < nombre_valeurs; i++) {
        printf("%.2f\n", valeurs[i]);
    }







    const char *input_file = "enregistrement-bruit.txt"; // nom du fichier entrée
    const char *output_file = "sortie.txt"; // nom du fichier sortie


    
    double bruit[10000]; // array de l'entrée
    double sortie[10000]; // array de la sortie

    // Lire l'entrée 
    int taille_bruit = LireEntree(input_file, bruit);
    if (taille_bruit < 0) {
        fprintf(stderr, "Erreur lors de la lecture du fichier d'entrée.\n");
        return 1; // Sortir si erreurr
    }

    // filtrage
    int result = filtrage(u, taille_bruit, sortie, taille_bruit, K, tau, Te);
    if (result < 0) {
        fprintf(stderr, "Erreur lors du filtrage des données.\n");
        return 1; // Sortir si erreur
    }

    // écrire le signal de sortie
    int taille_ecrite = EcrireSortie(output_file, sortie, taille_bruit);
    if (taille_ecrite < 0) {
        fprintf(stderr, "Erreur lors de l'écriture du fichier de sortie.\n");
        return 1; // Sortie si erreur
    }

    printf("Traitement terminé avec succès. %d valeurs écrites dans %s.\n", taille_ecrite, output_file);


    return 0;
}