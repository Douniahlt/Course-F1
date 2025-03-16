/*!\file f1_physics.c
 * \brief Implémentation des fonctions de physique pour le jeu F1
 */

 #include <math.h>
 #include "f1_types.h"
 #include "f1_physics.h"
 
 /* Taille du circuit */
 #define TRACK_SIZE 19.0f
 
 /* Initialisation de la physique */
 void initPhysics(void) {
     /* Initialisation des paramètres de physique si nécessaire */
 }
 
 /* Mise à jour de la physique */
 void updatePhysics(void) {
     /* Calcul du temps entre deux frames */
     static double t0 = 0;
     double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
     t0 = t;
     
     /* Limiter dt pour éviter les sauts de physique lors de lags */
     if (dt > 0.1f) dt = 0.1f;
     
     /* Mise à jour de la position de la voiture */
     float angleRad = _playerCar.ry * M_PI / 180.0f;
     
     /* Position précédente (pour restaurer en cas de collision) */
     float prevX = _playerCar.x;
     float prevZ = _playerCar.z;
     
     /* Mise à jour */
     _playerCar.x += _playerCar.speed * sinf(angleRad) * dt;
     _playerCar.z += _playerCar.speed * cosf(angleRad) * dt;
     
     /* Vérifier les collisions avec les bords du circuit */
     if(checkTrackBounds(&_playerCar)) {
         /* En cas de collision, restaurer la position précédente */
         _playerCar.x = prevX;
         _playerCar.z = prevZ;
         
         /* Réduire la vitesse pour simuler un freinage */
         _playerCar.speed *= 0.5f;
     }
     
     /* Appliquer la direction */
     _playerCar.ry += _playerCar.steering * dt * 100.0f;
     
     /* Faire tourner les roues en fonction de la vitesse */
     _playerCar.wheelRotation += _playerCar.speed * dt * 480.0f; /* Rotation plus rapide pour être visible */
     while(_playerCar.wheelRotation > 360.0f) _playerCar.wheelRotation -= 360.0f;
     while(_playerCar.wheelRotation < 0.0f) _playerCar.wheelRotation += 360.0f;
     
     /* Friction */
     _playerCar.speed *= 0.99f;
 }
 
 /* Vérification des collisions avec les bords du circuit */
 int checkTrackBounds(Car *car) {
     /* Simple vérification des limites du circuit plat */
     if(fabs(car->x) > TRACK_SIZE || fabs(car->z) > TRACK_SIZE) {
         return 1; /* Collision détectée */
     }
     return 0; /* Pas de collision */
 }