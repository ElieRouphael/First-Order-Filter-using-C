/* TD3.c -- first-order low-pass filter in C: the coursework program with three corrections (see README.md).
   "A first-order filter in C" (Kalman Filters with JAX, chapter 05), plus a real-time form of the filter.

   1. filtrage() is applied to the recorded signal `bruit`, not to the 51-sample self-test array `u`
      (the original read 198 cells past the end of `u` and wrote a file of zeros).
   2. The difference and its histogram cover the `taille_bruit` values actually read, not 500.
   3. The self-test is a unit step printed with four decimals, and tau is 0.02 s, chosen to pass the 1 Hz
      sine almost intact (gain 0.99, lag 9 deg) rather than to remove noise (see README.md); the original
      2 s attenuated it to 8%.

   Build:  gcc -Wall -Wextra -o td3 TD3.c -lm
   Run in the folder that holds enregistrement-bruit.txt. */
#include <stdio.h>
#include <math.h>

#define MAX_VALEURS 10000

/* Batch form: y[k] = z0 y[k-1] + K (1 - z0) u[k-1], the exact zero-order-hold discretisation of tau y' + y = K u. */
int filtrage(const double *u, int taille_u, double *y, int taille_y, double K, double tau, double Te) {
    if (taille_u != taille_y || taille_u < 1) {
        return -1;
    }
    double z0 = exp(-Te / tau);
    y[0] = 0;
    for (int k = 1; k < taille_u; k++) {
        y[k] = z0 * y[k - 1] + K * (1 - z0) * u[k - 1];
    }
    return 0;
}

/* Real-time form: one sample in, one sample out, state kept by the caller. */
typedef struct {
    double z0, b, y_prev, u_prev;
} Filtre;

void filtre_init(Filtre *f, double K, double tau, double Te) {
    f->z0 = exp(-Te / tau);
    f->b = K * (1 - f->z0);
    f->y_prev = 0;
    f->u_prev = 0;
}

double filtre_step(Filtre *f, double u) {
    double y = f->z0 * f->y_prev + f->b * f->u_prev;
    f->y_prev = y;
    f->u_prev = u;
    return y;
}

int LireEntree(const char *nom_fichier, double *valeurs, int max_valeurs) {
    FILE *fichier = fopen(nom_fichier, "r");
    if (fichier == NULL) {
        return -1;
    }
    int compteur = 0;
    while (compteur < max_valeurs) {
        if (fscanf(fichier, "%lf", &valeurs[compteur]) != 1) {
            if (feof(fichier)) {
                break;
            }
            fclose(fichier);
            return -1;
        }
        compteur++;
    }
    fclose(fichier);
    return compteur;
}

int EcrireSortie(const char *nom_fichier, const double *valeurs, int taille) {
    FILE *fichier = fopen(nom_fichier, "w");
    if (fichier == NULL) {
        return -1;
    }
    for (int i = 0; i < taille; i++) {
        if (fprintf(fichier, "%.4f\n", valeurs[i]) < 0) {
            fclose(fichier);
            return -1;
        }
    }
    fclose(fichier);
    return taille;
}

int ChercheMinMax(const double *tableau, int taille, double *min, double *max) {
    if (taille <= 0) {
        return -1;
    }
    *min = tableau[0];
    *max = tableau[0];
    for (int i = 1; i < taille; i++) {
        if (tableau[i] < *min) *min = tableau[i];
        if (tableau[i] > *max) *max = tableau[i];
    }
    return 0;
}

int CalculeHistogramme(const double *tableau, int taille, int *histogramme, int nb_bins) {
    if (taille <= 0 || nb_bins <= 0) {
        return -1;
    }
    double min, max;
    ChercheMinMax(tableau, taille, &min, &max);
    for (int i = 0; i < nb_bins; i++) {
        histogramme[i] = 0;
    }
    double largeur_bin = (max - min) / nb_bins;
    if (largeur_bin <= 0) {          /* all values equal: everything in the first bin */
        histogramme[0] = taille;
        return 0;
    }
    for (int i = 0; i < taille; i++) {
        int index = (int)((tableau[i] - min) / largeur_bin);
        if (index >= nb_bins) index = nb_bins - 1;
        if (index >= 0) histogramme[index]++;
    }
    return 0;
}

int main(void) {
    const double K = 1.0;      /* static gain */
    const double tau = 0.02;   /* time constant: passes the 1 Hz sine almost intact; see README.md */
    const double Te = 0.01;    /* sampling period */

    /* ---- self-test: unit step, 63.2% after tau/Te samples, 99.3% after 5 tau/Te ---- */
    const int n_test = 51;
    double u[51], y[51];
    for (int i = 0; i < n_test; i++) u[i] = 1.0;
    if (filtrage(u, n_test, y, n_test, K, tau, Te) != 0) {
        fprintf(stderr, "Erreur lors du filtrage de test\n");
        return 1;
    }
    int n_tau = (int)(tau / Te + 0.5);
    int ok = y[n_tau] > 0.60 * K && y[n_tau] < 0.67 * K && y[5 * n_tau] > 0.99 * K;
    printf("Test indiciel : y[%d] = %.4f (attendu 0.632), y[%d] = %.4f (attendu 0.993) : %s\n",
           n_tau, y[n_tau], 5 * n_tau, y[5 * n_tau], ok ? "PASS" : "FAIL");
    if (!ok) return 1;

    /* ---- the recording ---- */
    static double bruit[MAX_VALEURS], sortie[MAX_VALEURS], sortie_rt[MAX_VALEURS], difference[MAX_VALEURS];
    int taille_bruit = LireEntree("enregistrement-bruit.txt", bruit, MAX_VALEURS);
    if (taille_bruit < 0) {
        fprintf(stderr, "Erreur lors de la lecture du fichier d'entree.\n");
        return 1;
    }
    if (filtrage(bruit, taille_bruit, sortie, taille_bruit, K, tau, Te) < 0) {   /* fix 1: bruit, not u */
        fprintf(stderr, "Erreur lors du filtrage des donnees.\n");
        return 1;
    }

    /* the real-time form must give the same samples */
    Filtre f;
    filtre_init(&f, K, tau, Te);
    double ecart = 0;
    for (int i = 0; i < taille_bruit; i++) {
        sortie_rt[i] = filtre_step(&f, bruit[i]);
        double d = fabs(sortie_rt[i] - sortie[i]);
        if (d > ecart) ecart = d;
    }
    printf("Forme temps reel : ecart maximal avec filtrage() = %.3e\n", ecart);

    int taille_ecrite = EcrireSortie("sortie.txt", sortie, taille_bruit);
    if (taille_ecrite < 0) {
        fprintf(stderr, "Erreur lors de l'ecriture du fichier de sortie.\n");
        return 1;
    }
    printf("Traitement termine avec succes. %d valeurs ecrites dans sortie.txt.\n", taille_ecrite);

    /* ---- histogram of what the filter removed (fix 2: over the values read) ---- */
    int histogramme[16];
    for (int i = 0; i < taille_bruit; i++) {
        difference[i] = bruit[i] - sortie[i];
    }
    CalculeHistogramme(difference, taille_bruit, histogramme, 16);
    printf("Histogramme de la difference (%d valeurs, 16 classes) :\n", taille_bruit);
    for (int i = 0; i < 16; i++) {
        printf("Classe %2d : %d\n", i, histogramme[i]);
    }
    return 0;
}
