# Makefile pour F1 Racing Game
# Structure multi-fichiers
CC = gcc
ECHO = echo
RM = rm -f
MKDIR = mkdir
CHMOD = chmod
CFLAGS = -Wall -O3
CPPFLAGS = -I.
LDFLAGS = -lm
PACKNAME = f1_racing
PROGNAME = f1racing
VERSION = 1.0
distdir = $(PACKNAME)-$(VERSION)
# Ajout des fichiers IA et des nouveaux fichiers pour les circuits
SOURCES = f1_main.c f1_render.c f1_physics.c f1_input.c f1_ai.c f1_track.c 
HEADERS = f1_types.h f1_render.h f1_physics.h f1_input.h f1_ai.h f1_track.h
OBJ = $(SOURCES:.c=.o)
# Traitements automatiques pour ajout de chemins et options
ifneq (,$(shell ls -d /usr/local/include 2>/dev/null | tail -n 1))
CPPFLAGS += -I/usr/local/include
endif
ifneq (,$(shell ls -d $(HOME)/local/include 2>/dev/null | tail -n 1))
CPPFLAGS += -I$(HOME)/local/include
endif
ifneq (,$(shell ls -d /usr/local/lib 2>/dev/null | tail -n 1))
LDFLAGS += -L/usr/local/lib
endif
ifneq (,$(shell ls -d $(HOME)/local/lib 2>/dev/null | tail -n 1))
LDFLAGS += -L$(HOME)/local/lib
endif
# Configuration spécifique pour MacOS
ifeq ($(shell uname),Darwin)
MACOSX_DEPLOYMENT_TARGET = 10.8
CFLAGS += -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
LDFLAGS += -framework OpenGL -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
else
LDFLAGS += -lGL
endif
# Ajout des flags pour SDL2 et GL4Dummies
CPPFLAGS += $(shell sdl2-config --cflags)
LDFLAGS += -lGL4Dummies -lGLU $(shell sdl2-config --libs)

all: shaders $(PROGNAME)

$(PROGNAME): $(OBJ)
	$(CC) $(OBJ) $(LDFLAGS) -o $(PROGNAME)

%.o: %.c $(HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Cible pour créer les dossiers shaders s'ils n'existent pas et copier les shaders
shaders:
	@$(MKDIR) -p shaders
	@if [ ! -f shaders/basic.vs ]; then \
		$(ECHO) "Creating basic.vs..."; \
		$(ECHO) '/*!\file basic.vs\n * \brief Vertex shader basique\n */\n\n#version 330\n\nlayout(location = 0) in vec3 vsiPosition;\nlayout(location = 1) in vec3 vsiNormal;\nlayout(location = 2) in vec2 vsiTexCoord;\n\nuniform mat4 projectionMatrix;\nuniform mat4 viewMatrix;\nuniform mat4 modelMatrix;\nuniform vec4 couleur;\n\nout vec4 vsoColor;\n\nvoid main(void) {\n    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vsiPosition, 1.0);\n    vsoColor = couleur;\n}' > shaders/basic.vs; \
	fi
	@if [ ! -f shaders/basic.fs ]; then \
		$(ECHO) "Creating basic.fs..."; \
		$(ECHO) '/*!\file basic.fs\n * \brief Fragment shader basique\n */\n\n#version 330\n\nin vec4 vsoColor;\n\nout vec4 fragColor;\n\nvoid main(void) {\n    fragColor = vsoColor;\n}' > shaders/basic.fs; \
	fi
	@if [ ! -f shaders/editor.vs ]; then \
		$(ECHO) "Creating editor.vs..."; \
		$(ECHO) '/*!\file editor.vs\n * \brief Vertex shader pour l édition de circuit\n */\n\n#version 330\n\nlayout(location = 0) in vec3 vsiPosition;\nlayout(location = 1) in vec3 vsiNormal;\nlayout(location = 2) in vec2 vsiTexCoord;\n\nuniform mat4 projectionMatrix;\nuniform mat4 viewMatrix;\nuniform mat4 modelMatrix;\n\nout vec2 vsoTexCoord;\n\nvoid main(void) {\n    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vsiPosition, 1.0);\n    vsoTexCoord = vsiTexCoord;\n}' > shaders/editor.vs; \
	fi
	@if [ ! -f shaders/editor.fs ]; then \
		$(ECHO) "Creating editor.fs..."; \
		$(ECHO) '/*!\file editor.fs\n * \brief Fragment shader pour l édition de circuit\n */\n\n#version 330\n\nin vec2 vsoTexCoord;\n\nuniform sampler2D textureSampler;\nuniform float gridSize;\n\nout vec4 fragColor;\n\nvoid main(void) {\n    vec2 scaledUV = vsoTexCoord * gridSize;\n    fragColor = texture(textureSampler, scaledUV);\n}' > shaders/editor.fs; \
	fi

clean:
	@$(RM) -r $(PROGNAME) $(OBJ) *~

run: all
	./$(PROGNAME)

.PHONY: all clean run shaders