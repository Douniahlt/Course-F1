/*!\file f1_track.h
 * \brief Gestion des circuits pour le jeu F1
 */

 #ifndef F1_TRACK_H
 #define F1_TRACK_H
 
 #include <GL4D/gl4duw_SDL2.h>
 
 /* Définition des types de segments de piste */
 typedef enum {
     TRACK_STRAIGHT,    /* Ligne droite */
     TRACK_CURVE_LEFT,  /* Virage à gauche */
     TRACK_CURVE_RIGHT, /* Virage à droite */
     TRACK_HAIRPIN_LEFT, /* Épingle à gauche */
     TRACK_HAIRPIN_RIGHT /* Épingle à droite */
 } TrackSegmentType;
 
 /* Structure pour un segment de piste */
 typedef struct {
     TrackSegmentType type;    /* Type de segment */
     float length;              /* Longueur du segment */
     float curvature;           /* Courbure (pour les virages) */
     float width;               /* Largeur de la piste */
     int textureId;             /* ID de texture pour ce segment */
 } TrackSegment;
 
 /* Structure pour un point de trajectoire (waypoint) */
 typedef struct {
     float x, z;               /* Position 2D (hauteur y est toujours 0) */
     float width;              /* Largeur de la piste à ce point */
 } TrackWaypoint;
 
 /* Structure pour un circuit complet */
 typedef struct {
     char name[64];            /* Nom du circuit */
     int segmentCount;         /* Nombre de segments */
     TrackSegment* segments;   /* Tableau de segments */
     int waypointCount;        /* Nombre de waypoints */
     TrackWaypoint* waypoints; /* Tableau des waypoints */
     GLuint trackTexture;      /* Texture principale du circuit */
     GLuint trackMesh;         /* Mesh 3D du circuit */
     int isModified;           /* Indique si le circuit a été modifié */
 } Track;
 
 /* Initialisation du système de circuit */
 void initTrackSystem(void);
 
 /* Création d'un nouveau circuit vide */
 Track* createEmptyTrack(const char* name);
 
 /* Libération de la mémoire d'un circuit */
 void destroyTrack(Track* track);
 
 /* Ajout d'un segment à un circuit */
 int addTrackSegment(Track* track, TrackSegmentType type, float length, float curvature, float width);
 
 /* Génération des waypoints à partir des segments */
 void generateWaypoints(Track* track);
 
 /* Génération du mesh 3D du circuit */
 void generateTrackMesh(Track* track);
 
 /* Passage du jeu à l'éditeur de circuit */
 void enterTrackEditor(void);
 
 #endif /* F1_TRACK_H */