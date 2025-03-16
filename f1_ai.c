/*!\file f1_ai.c
 * \brief Implémentation de la gestion des concurrents IA pour le jeu F1
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <math.h>
 #include <stdlib.h>
 #include <time.h>
 
 #include "f1_types.h"
 #include "f1_ai.h"
 #include "f1_physics.h"
 
 /* Tableau des voitures IA */
 static Car _aiCars[AI_CAR_COUNT];
 
 /* Points de passage pour le circuit */
 typedef struct {
     float x, z;
 } Waypoint;
 
 /* Points de passage pour un circuit simple */
 #define WAYPOINT_COUNT 8
 static Waypoint _waypoints[WAYPOINT_COUNT] = {
     {0.0f, 10.0f},    /* Point de départ (ligne d'arrivée) */
     {10.0f, 10.0f},   /* Premier virage */
     {15.0f, 5.0f},    /* Deuxième virage */
     {15.0f, -5.0f},   /* Ligne droite du fond */
     {10.0f, -10.0f},  /* Troisième virage */
     {0.0f, -10.0f},   /* Milieu du circuit */
     {-10.0f, -10.0f}, /* Quatrième virage */
     {-10.0f, 5.0f}    /* Retour vers l'arrivée */
 };
 
 /* Waypoint courant pour chaque voiture IA */
 static int _currentWaypoint[AI_CAR_COUNT];
 
 /* Cibles actuelles des voitures IA pour un lissage de la direction */
 static float _targetAngle[AI_CAR_COUNT];
 static float _steeringDamping[AI_CAR_COUNT];
 
 /* Initialise les voitures IA */
 void initAICars(void) {
     srand(time(NULL)); /* Initialisation du générateur aléatoire */
     
     /* Couleurs prédéfinies pour les équipes */
     float colors[5][3] = {
         {0.0f, 0.0f, 0.7f},  /* Bleu */
         {0.0f, 0.7f, 0.0f},  /* Vert */
         {0.7f, 0.7f, 0.0f},  /* Jaune */
         {0.7f, 0.0f, 0.7f},  /* Violet */
         {0.0f, 0.7f, 0.7f}   /* Cyan */
     };
     
     for(int i = 0; i < AI_CAR_COUNT; i++) {
         /* Position initiale décalée sur la grille de départ */
         _aiCars[i].x = 0.0f;
         _aiCars[i].y = 0.06f;
         _aiCars[i].z = 2.0f + i * 1.0f; /* Espacer les voitures */
         
         /* Orientation initiale - toutes les voitures démarrent en regardant vers le nord (vers Z positif) */
         _aiCars[i].rx = 0.0f;
         _aiCars[i].ry = 0.0f; /* 0 degré = direction Z+ (nord) */
         _aiCars[i].rz = 0.0f;
         
         /* Paramètres de conduite légèrement différents pour chaque IA */
         _aiCars[i].speed = 0.0f;
         _aiCars[i].maxSpeed = 1.5f + ((float)rand() / RAND_MAX) * 0.3f; /* Vitesse entre 1.5 et 1.8 */
         _aiCars[i].acceleration = 0.08f + ((float)rand() / RAND_MAX) * 0.02f; /* Accélération entre 0.08 et 0.1 */
         
         /* Direction */
         _aiCars[i].steering = 0.0f;
         
         /* Couleur de l'équipe */
         int colorIndex = i % 5;
         _aiCars[i].r = colors[colorIndex][0];
         _aiCars[i].g = colors[colorIndex][1];
         _aiCars[i].b = colors[colorIndex][2];
         
         /* Rotation des roues */
         _aiCars[i].wheelRotation = 0.0f;
         
         /* Premier waypoint - toutes les voitures commencent par le premier point */
         _currentWaypoint[i] = 0;
         
         /* Initialisation des paramètres de lissage */
         _targetAngle[i] = 0.0f;
         _steeringDamping[i] = 0.9f + ((float)rand() / RAND_MAX) * 0.05f; /* Facteur de lissage entre 0.9 et 0.95 */
     }
 }
 
 /* Calcule l'angle (en degrés) entre deux points */
 static float calculateAngle(float x1, float z1, float x2, float z2) {
     float dx = x2 - x1;
     float dz = z2 - z1;
     
     float angle = atan2f(dx, dz) * 180.0f / M_PI;
     
     return angle;
 }
 
 /* Calcule la distance entre deux points */
 static float calculateDistance(float x1, float z1, float x2, float z2) {
     float dx = x2 - x1;
     float dz = z2 - z1;
     
     return sqrtf(dx * dx + dz * dz);
 }
 
 /* Implémentation d'un contrôleur PID simple pour la direction */
 static float pidSteering(int carIndex, float angleError, float dt) {
     /* Constantes PID - ajustées pour un contrôle plus fluide */
     const float kp = 0.015f;  /* Composante proportionnelle (réduite pour moins d'oscillations) */
     const float kd = 0.025f;  /* Composante dérivée (pour amortir les mouvements brusques) */
     
     /* Variables statiques pour conserver l'état entre les appels */
     static float lastError[AI_CAR_COUNT] = {0};
     
     /* Calcul des termes PID */
     float proportional = kp * angleError;
     float derivative = kd * (angleError - lastError[carIndex]) / dt;
     
     /* Mise à jour de l'erreur précédente */
     lastError[carIndex] = angleError;
     
     /* Calcul de la sortie du contrôleur */
     float output = proportional + derivative;
     
     /* Limitation de la valeur de sortie entre -1 et 1 */
     if (output > 1.0f) output = 1.0f;
     if (output < -1.0f) output = -1.0f;
     
     return output;
 }
 
 /* Met à jour les comportements des voitures IA */
 void updateAICars(void) {
     /* Calcul du temps entre deux frames */
     static double t0 = 0;
     double t = gl4dGetElapsedTime(), dt = (t - t0) / 1000.0;
     t0 = t;
     
     /* Limiter dt pour éviter les sauts de physique lors de lags */
     if (dt > 0.1f) dt = 0.1f;
     
     for(int i = 0; i < AI_CAR_COUNT; i++) {
         /* Récupérer le waypoint actuel */
         int wpIndex = _currentWaypoint[i];
         Waypoint target = _waypoints[wpIndex];
         
         /* Calculer l'angle vers le waypoint */
         float rawTargetAngle = calculateAngle(_aiCars[i].x, _aiCars[i].z, target.x, target.z);
         
         /* Regarder le waypoint suivant pour anticiper les virages (look-ahead) */
         int nextWpIndex = (wpIndex + 1) % WAYPOINT_COUNT;
         Waypoint nextTarget = _waypoints[nextWpIndex];
         
         /* Calculer la distance au waypoint */
         float distance = calculateDistance(_aiCars[i].x, _aiCars[i].z, target.x, target.z);
         
         /* Si nous approchons du waypoint, commencer à regarder le suivant */
         if (distance < 3.0f) {
             /* Mélange progressif entre le waypoint actuel et le suivant */
             float blendFactor = 1.0f - (distance / 3.0f);
             float nextAngle = calculateAngle(_aiCars[i].x, _aiCars[i].z, nextTarget.x, nextTarget.z);
             
             /* Interpolation entre les deux angles */
             rawTargetAngle = rawTargetAngle * (1.0f - blendFactor) + nextAngle * blendFactor;
         }
         
         /* Lissage du target angle pour éviter les changements brusques */
         _targetAngle[i] = _targetAngle[i] * _steeringDamping[i] + rawTargetAngle * (1.0f - _steeringDamping[i]);
         
         /* Calculer la différence d'angle */
         float angleDiff = _targetAngle[i] - _aiCars[i].ry;
         
         /* Normaliser la différence d'angle entre -180 et 180 */
         while(angleDiff > 180.0f) angleDiff -= 360.0f;
         while(angleDiff < -180.0f) angleDiff += 360.0f;
         
         /* Utiliser le contrôleur PID pour la direction */
         _aiCars[i].steering = pidSteering(i, angleDiff, dt);
         
         /* Ajuster la vitesse selon la courbure du virage et la distance au waypoint */
         float speedFactor = 1.0f;
         
         /* Réduire la vitesse dans les virages serrés */
         if(fabs(angleDiff) > 25.0f) {
             speedFactor = 0.8f - (fabs(angleDiff) - 25.0f) * 0.004f;
             if (speedFactor < 0.6f) speedFactor = 0.6f;
         }
         
         /* Réduire légèrement la vitesse à l'approche des waypoints */
         if(distance < 1.5f) {
             speedFactor *= 0.95f + (distance / 1.5f) * 0.05f;
         }
         
         /* Accélérer/décélérer progressivement */
         float targetSpeed = _aiCars[i].maxSpeed * speedFactor;
         if(_aiCars[i].speed < targetSpeed) {
             _aiCars[i].speed += _aiCars[i].acceleration * dt;
             if(_aiCars[i].speed > targetSpeed) {
                 _aiCars[i].speed = targetSpeed;
             }
         } else {
             /* Freiner progressivement */
             _aiCars[i].speed -= _aiCars[i].acceleration * 1.2f * dt; /* Freine plus rapidement qu'accélération */
             if(_aiCars[i].speed < targetSpeed) {
                 _aiCars[i].speed = targetSpeed;
             }
         }
         
         /* Éviter les collisions avec d'autres voitures */
         for(int j = 0; j < AI_CAR_COUNT; j++) {
             if(i == j) continue; /* Ne pas se comparer à soi-même */
             
             float carDistance = calculateDistance(_aiCars[i].x, _aiCars[i].z, _aiCars[j].x, _aiCars[j].z);
             
             if(carDistance < 1.2f) { /* Détection de collision */
                 /* Calcul de la position relative */
                 float relX = _aiCars[j].x - _aiCars[i].x;
                 float relZ = _aiCars[j].z - _aiCars[i].z;
                 
                 /* Angle vers l'autre voiture */
                 float carAngle = atan2f(relX, relZ) * 180.0f / M_PI;
                 
                 /* Différence d'angle avec la direction de la voiture */
                 float angleDiffCar = carAngle - _aiCars[i].ry;
                 while(angleDiffCar > 180.0f) angleDiffCar -= 360.0f;
                 while(angleDiffCar < -180.0f) angleDiffCar += 360.0f;
                 
                 /* Si la voiture est devant nous */
                 if(fabs(angleDiffCar) < 45.0f) {
                     /* Ralentir progressivement selon la distance */
                     float slowFactor = 1.0f - ((1.2f - carDistance) / 1.2f) * 0.6f;
                     _aiCars[i].speed *= slowFactor;
                     
                     /* Ajuster la direction pour éviter */
                     if(fabs(angleDiffCar) > 5.0f) {
                         /* Tourner dans la direction opposée à la voiture */
                         float evadeStrength = (angleDiffCar > 0) ? -0.4f : 0.4f;
                         /* Mélange entre la direction courante et l'évitement */
                         _aiCars[i].steering = _aiCars[i].steering * 0.3f + evadeStrength * 0.7f;
                     }
                 }
             }
         }
         
         /* Position précédente (pour restaurer en cas de collision) */
         float prevX = _aiCars[i].x;
         float prevZ = _aiCars[i].z;
         
         /* Mise à jour de la position */
         float angleRad = _aiCars[i].ry * M_PI / 180.0f;
         _aiCars[i].x += _aiCars[i].speed * sinf(angleRad) * dt;
         _aiCars[i].z += _aiCars[i].speed * cosf(angleRad) * dt;
         
         /* Vérifier les collisions avec les bords du circuit */
         if(checkTrackBounds(&_aiCars[i])) {
             /* En cas de collision, restaurer la position précédente */
             _aiCars[i].x = prevX;
             _aiCars[i].z = prevZ;
             
             /* Réduire la vitesse et changer de direction plus progressivement */
             _aiCars[i].speed *= 0.7f;
             _aiCars[i].steering = -_aiCars[i].steering * 0.7f;
         }
         
         /* Appliquer la direction avec un facteur de vitesse */
         /* Plus la voiture va vite, moins elle tourne vite */
         float steeringFactor = 80.0f / (1.0f + _aiCars[i].speed * 0.4f);
         _aiCars[i].ry += _aiCars[i].steering * dt * steeringFactor;
         
         /* Faire tourner les roues - CORRECTION : inverser le signe pour tourner dans le bon sens */
         _aiCars[i].wheelRotation -= _aiCars[i].speed * dt * 480.0f;
         /* Maintenir l'angle de la roue entre 0 et 360 degrés */
         while(_aiCars[i].wheelRotation > 360.0f) _aiCars[i].wheelRotation -= 360.0f;
         while(_aiCars[i].wheelRotation < 0.0f) _aiCars[i].wheelRotation += 360.0f;
         
         /* Vérifier si la voiture a atteint le waypoint (distance < 1.0) */
         if(distance < 1.0f) {
             /* Passer au waypoint suivant */
             _currentWaypoint[i] = (wpIndex + 1) % WAYPOINT_COUNT;
         }
     }
 }
 
 /* Récupérer un tableau des voitures IA */
 Car* getAICars(void) {
     return _aiCars;
 }
 
 /* Récupérer une voiture IA spécifique */
 Car* getAICar(int index) {
     if(index >= 0 && index < AI_CAR_COUNT) {
         return &_aiCars[index];
     }
     return NULL;
 }
 
 /* Fonctions pour l'affichage des waypoints (pour le débogage) */
 float* getWaypointX(void) {
     static float x[WAYPOINT_COUNT];
     for(int i = 0; i < WAYPOINT_COUNT; i++) {
         x[i] = _waypoints[i].x;
     }
     return x;
 }
 
 float* getWaypointZ(void) {
     static float z[WAYPOINT_COUNT];
     for(int i = 0; i < WAYPOINT_COUNT; i++) {
         z[i] = _waypoints[i].z;
     }
     return z;
 }
 
 int getWaypointCount(void) {
     return WAYPOINT_COUNT;
 }