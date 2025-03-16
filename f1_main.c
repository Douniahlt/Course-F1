/*!\file f1_main.c
 * \brief Jeu de course F1 avec GL4D - Point d'entrée principal
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <GL4D/gl4dg.h>
 #include <GL4D/gl4du.h>
 #include <SDL2/SDL.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <math.h>
 
 #include "f1_types.h"
 #include "f1_render.h"
 #include "f1_input.h"
 #include "f1_physics.h"
 #include "f1_ai.h"     /* Inclusion pour l'IA */
 #include "f1_track.h"  /* Inclusion pour le système de circuits */

void updateGame(void);
void checkCarCollisions(void);
void quit(void);
 
 /* Dimensions de la fenêtre */
 int _ww = 1280;
 int _wh = 720;
 
 /* Variables pour les ressources OpenGL */
 GLuint _pId = 0;      /* Programme GLSL */
 
 /* Variables globales partagées */
 int _viewMode = 0;    /* 0: course, 1: face, 2: arrière, 3: dessus */
 Car _playerCar;       /* Voiture du joueur */
 Track* _currentTrack = NULL; /* Circuit actuel */
 
 /* Fonction principale */
 int main(int argc, char ** argv) {
     /* Création de la fenêtre GL4D */
     if(!gl4duwCreateWindow(argc, argv, "F1 Racing Game", 
                           10, 10, _ww, _wh, 
                           GL4DW_SHOWN)) {
         return 1;
     }
     
     /* Initialisation */
     initRendering();
     initPhysics();
     initTrackSystem();  /* Initialiser le système de circuit */
     initAICars();       /* Initialisation des voitures IA */
     
     /* Configuration des callbacks */
     atexit(quit);
     gl4duwDisplayFunc(draw);
     gl4duwIdleFunc(updateGame);  /* Fonction combinée pour la mise à jour */
     gl4duwKeyDownFunc(keydown);
     gl4duwKeyUpFunc(keyup);
     gl4duwMouseFunc(NULL);  /* Pas de gestion de souris par défaut */
     
     /* Boucle principale */
     gl4duwMainLoop();
     
     return 0;
 }
 
 /* Fonction de mise à jour combinée */
 void updateGame(void) {
     /* Mise à jour de la physique pour la voiture du joueur */
     updatePhysics();
     
     /* Mise à jour des voitures IA */
     updateAICars();
     
     /* Vérification des collisions entre voitures */
     checkCarCollisions();
 }
 
 /* Vérification des collisions entre voitures */
 void checkCarCollisions(void) {
     const float collisionThreshold = 1.0f; /* Distance minimale entre voitures */
     
     /* Vérifier la collision entre le joueur et chaque voiture IA */
     for(int i = 0; i < AI_CAR_COUNT; i++) {
         Car* aiCar = getAICar(i);
         if(aiCar) {
             /* Calculer la distance entre les voitures */
             float dx = _playerCar.x - aiCar->x;
             float dz = _playerCar.z - aiCar->z;
             float distance = sqrtf(dx * dx + dz * dz);
             
             /* Si collision détectée */
             if(distance < collisionThreshold) {
                 /* Calculer la direction du choc */
                 float dirX = dx / distance;
                 float dirZ = dz / distance;
                 
                 /* Calculer la force du choc (inverse de la distance) */
                 float force = (collisionThreshold - distance) * 0.5f;
                 
                 /* Appliquer la force de répulsion aux deux voitures */
                 _playerCar.x += dirX * force;
                 _playerCar.z += dirZ * force;
                 
                 aiCar->x -= dirX * force;
                 aiCar->z -= dirZ * force;
                 
                 /* Réduire légèrement la vitesse des deux voitures */
                 _playerCar.speed *= 0.9f;
                 aiCar->speed *= 0.9f;
             }
         }
     }
     
     /* Vérifier les collisions entre voitures IA (facultatif, peut être lourd en calculs) */
     for(int i = 0; i < AI_CAR_COUNT; i++) {
         Car* aiCar1 = getAICar(i);
         if(aiCar1) {
             for(int j = i + 1; j < AI_CAR_COUNT; j++) {
                 Car* aiCar2 = getAICar(j);
                 if(aiCar2) {
                     /* Calculer la distance entre les voitures */
                     float dx = aiCar1->x - aiCar2->x;
                     float dz = aiCar1->z - aiCar2->z;
                     float distance = sqrtf(dx * dx + dz * dz);
                     
                     /* Si collision détectée */
                     if(distance < collisionThreshold) {
                         /* Calculer la direction du choc */
                         float dirX = dx / distance;
                         float dirZ = dz / distance;
                         
                         /* Calculer la force du choc */
                         float force = (collisionThreshold - distance) * 0.5f;
                         
                         /* Appliquer la force de répulsion */
                         aiCar1->x += dirX * force;
                         aiCar1->z += dirZ * force;
                         
                         aiCar2->x -= dirX * force;
                         aiCar2->z -= dirZ * force;
                         
                         /* Réduire légèrement la vitesse */
                         aiCar1->speed *= 0.9f;
                         aiCar2->speed *= 0.9f;
                     }
                 }
             }
         }
     }
 }
 
 /* Fonction de nettoyage */
 void quit(void) {
     /* Libérer les ressources du circuit si nécessaire */
     if(_currentTrack) {
         destroyTrack(_currentTrack);
         _currentTrack = NULL;
     }
     
     /* Nettoyer GL4D */
     gl4duClean(GL4DU_ALL);
 }