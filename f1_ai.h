/*!\file f1_ai.h
 * \brief Gestion des concurrents IA pour le jeu F1
 */

 #ifndef F1_AI_H
 #define F1_AI_H
 
 #include "f1_types.h"
 
 /* Nombre de voitures IA */
 #define AI_CAR_COUNT 3
 
 /* Initialise les voitures IA */
 void initAICars(void);
 
 /* Met à jour les comportements des voitures IA */
 void updateAICars(void);
 
 /* Récupérer un tableau des voitures IA */
 Car* getAICars(void);
 
 /* Récupérer une voiture IA spécifique */
 Car* getAICar(int index);
 
 #endif /* F1_AI_H */