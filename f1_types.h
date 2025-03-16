/*!\file f1_types.h
 * \brief Définitions des structures et types pour le jeu F1
 */

#ifndef F1_TYPES_H
#define F1_TYPES_H

#include <GL4D/gl4duw_SDL2.h>

/* Structure pour une voiture */
typedef struct {
    float x, y, z;         /* Position */
    float rx, ry, rz;      /* Rotation */
    float speed;           /* Vitesse */
    float acceleration;    /* Taux d'accélération */
    float maxSpeed;        /* Vitesse maximale */
    float steering;        /* Direction */
    float r, g, b;         /* Couleur */
    float wheelRotation;   /* Angle de rotation des roues */
} Car;

/* Variables globales partagées - déclarées dans f1_main.c */
extern int _ww;            /* Largeur de la fenêtre */
extern int _wh;            /* Hauteur de la fenêtre */
extern GLuint _pId;        /* Programme GLSL */
extern int _viewMode;      /* Mode de vue */
extern Car _playerCar;     /* Voiture du joueur */

/* Fonction de mise à jour combinée */
void updateGame(void);

/* Fonction de nettoyage */
void quit(void);

#endif /* F1_TYPES_H */