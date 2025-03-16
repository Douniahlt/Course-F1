/*!\file f1_physics.h
 * \brief Fonctions de physique pour le jeu F1
 */

 #ifndef F1_PHYSICS_H
 #define F1_PHYSICS_H
 
 /* Initialisation de la physique */
 void initPhysics(void);
 
 /* Mise à jour de la physique */
 void updatePhysics(void);
 
 /* Collision avec les bords du circuit */
 int checkTrackBounds(Car *car);
 
 #endif /* F1_PHYSICS_H */