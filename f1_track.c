/*!\file f1_track.c
 * \brief Implémentation des fonctions de gestion des circuits pour le jeu F1
 */

 #include <GL4D/gl4duw_SDL2.h>
 #include <GL4D/gl4dg.h>
 #include <GL4D/gl4du.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <math.h>
 #include <time.h>  /* Pour time() */
 
 #include "f1_types.h"
 #include "f1_track.h"
 
 /* Circuit actuel (déclaré dans f1_main.c, on la retrouve ici) */
 extern Track* _currentTrack;
 
 /* Prototypes des fonctions locales */
 Track* generateDefaultTrack(const char* name);
 
 /* Initialisation du système de circuit */
 void initTrackSystem(void) {
     /* Création d'un circuit par défaut si aucun n'est chargé */
     _currentTrack = generateDefaultTrack("Circuit par défaut");
 }
 
 /* Création d'un nouveau circuit vide */
 Track* createEmptyTrack(const char* name) {
     Track* track = (Track*)malloc(sizeof(Track));
     if (!track) return NULL;
     
     strncpy(track->name, name, sizeof(track->name) - 1);
     track->name[sizeof(track->name) - 1] = '\0';
     
     track->segmentCount = 0;
     track->segments = NULL;
     track->waypointCount = 0;
     track->waypoints = NULL;
     track->trackTexture = 0;
     track->trackMesh = 0;
     track->isModified = 1;
     
     return track;
 }
 
 /* Génération d'un circuit par défaut */
 Track* generateDefaultTrack(const char* name) {
     Track* track = createEmptyTrack(name);
     if (!track) return NULL;
     
     /* Ajouter des segments pour former un circuit simple */
     addTrackSegment(track, TRACK_STRAIGHT, 20.0f, 0.0f, 5.0f);
     addTrackSegment(track, TRACK_CURVE_RIGHT, 15.0f, 0.2f, 5.0f);
     addTrackSegment(track, TRACK_STRAIGHT, 30.0f, 0.0f, 5.0f);
     addTrackSegment(track, TRACK_CURVE_LEFT, 15.0f, 0.2f, 5.0f);
     addTrackSegment(track, TRACK_STRAIGHT, 20.0f, 0.0f, 5.0f);
     addTrackSegment(track, TRACK_CURVE_LEFT, 15.0f, 0.2f, 5.0f);
     addTrackSegment(track, TRACK_STRAIGHT, 30.0f, 0.0f, 5.0f);
     addTrackSegment(track, TRACK_CURVE_RIGHT, 15.0f, 0.2f, 5.0f);
     
     /* Générer les waypoints et le mesh du circuit */
     generateWaypoints(track);
     generateTrackMesh(track);
     
     track->isModified = 0;
     
     return track;
 }
 
 /* Libération de la mémoire d'un circuit */
 void destroyTrack(Track* track) {
     if (!track) return;
     
     if (track->segments) {
         free(track->segments);
         track->segments = NULL;
     }
     
     if (track->waypoints) {
         free(track->waypoints);
         track->waypoints = NULL;
     }
     
     if (track->trackMesh) {
         gl4dgDelete(track->trackMesh);
         track->trackMesh = 0;
     }
     
     free(track);
 }
 
 /* Ajout d'un segment à un circuit */
 int addTrackSegment(Track* track, TrackSegmentType type, float length, float curvature, float width) {
     if (!track) return 0;
     
     /* Réallocation du tableau des segments */
     TrackSegment* newSegments = (TrackSegment*)realloc(track->segments, 
                                                     (track->segmentCount + 1) * sizeof(TrackSegment));
     if (!newSegments) {
         fprintf(stderr, "Erreur d'allocation mémoire pour l'ajout d'un segment\n");
         return 0;
     }
     
     track->segments = newSegments;
     
     /* Ajout du nouveau segment */
     track->segments[track->segmentCount].type = type;
     track->segments[track->segmentCount].length = length;
     track->segments[track->segmentCount].curvature = curvature;
     track->segments[track->segmentCount].width = width;
     track->segments[track->segmentCount].textureId = 0; /* Texture par défaut */
     
     track->segmentCount++;
     track->isModified = 1;
     
     return 1;
 }
 
 /* Génération des waypoints à partir des segments */
 void generateWaypoints(Track* track) {
     if (!track || track->segmentCount == 0) return;
     
     /* Libération des waypoints existants */
     if (track->waypoints) {
         free(track->waypoints);
         track->waypoints = NULL;
         track->waypointCount = 0;
     }
     
     /* Version simplifiée : 8 waypoints formant un circuit rectangulaire */
     track->waypointCount = 8;
     track->waypoints = (TrackWaypoint*)malloc(track->waypointCount * sizeof(TrackWaypoint));
     if (!track->waypoints) {
         fprintf(stderr, "Erreur d'allocation mémoire pour les waypoints\n");
         return;
     }
     
     /* Définition d'un circuit rectangulaire simple */
     track->waypoints[0].x = 0.0f;
     track->waypoints[0].z = 10.0f;
     track->waypoints[0].width = 5.0f;
     
     track->waypoints[1].x = 10.0f;
     track->waypoints[1].z = 10.0f;
     track->waypoints[1].width = 5.0f;
     
     track->waypoints[2].x = 15.0f;
     track->waypoints[2].z = 5.0f;
     track->waypoints[2].width = 5.0f;
     
     track->waypoints[3].x = 15.0f;
     track->waypoints[3].z = -5.0f;
     track->waypoints[3].width = 5.0f;
     
     track->waypoints[4].x = 10.0f;
     track->waypoints[4].z = -10.0f;
     track->waypoints[4].width = 5.0f;
     
     track->waypoints[5].x = 0.0f;
     track->waypoints[5].z = -10.0f;
     track->waypoints[5].width = 5.0f;
     
     track->waypoints[6].x = -10.0f;
     track->waypoints[6].z = -10.0f;
     track->waypoints[6].width = 5.0f;
     
     track->waypoints[7].x = -10.0f;
     track->waypoints[7].z = 5.0f;
     track->waypoints[7].width = 5.0f;
 }
 
 /* Génération du mesh 3D du circuit */
 void generateTrackMesh(Track* track) {
     if (!track || track->waypointCount < 2) return;
     
     /* Version simplifiée : utilisation d'un quad pour la piste */
     track->trackMesh = gl4dgGenQuadf();
 }
 
 /* Passage du jeu à l'éditeur de circuit */
 void enterTrackEditor(void) {
     printf("Éditeur de circuit non implémenté\n");
 }