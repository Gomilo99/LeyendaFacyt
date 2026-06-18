CXX          := g++
CXXFLAGS     := -std=c++17 -Wall -Wextra -Wpedantic -I. -MMD -MP
LDFLAGS      :=
SRCDIR       := src
OBJDIR       := obj
OBJDIR_DIST  := obj-dist
TARGET       := leyenda
DISTDIR      := .dist
DISTTARGET   := $(DISTDIR)/$(TARGET)
DATA_DIRS    := json mapas assets

# Platform detection
ifeq ($(OS),Windows_NT)
TARGET       := leyenda.exe
STATIC_LIBS  := -static-libgcc -static-libstdc++ -static
MKDIR = if not exist "$(1)" mkdir "$(1)"
RMDIR = if exist "$(1)" rmdir /s /q "$(1)"
DEL   = if exist "$(1)" del /f /q "$(1)"
else
TARGET       := leyenda
STATIC_LIBS  := -static-libgcc -static-libstdc++
MKDIR = mkdir -p "$(1)"
RMDIR = rm -rf "$(1)"
DEL   = rm -f "$(1)"
endif

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

OBJS_DIST := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR_DIST)/%.o, $(SRCS))

.PHONY: all clean run dist

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@$(call MKDIR,$(OBJDIR))
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Distribution build (statically linked, optimized)
$(DISTTARGET): $(OBJS_DIST)
	@$(call MKDIR,$(DISTDIR))
	$(CXX) $(CXXFLAGS) -O2 $(STATIC_LIBS) -o $@ $^

$(OBJDIR_DIST)/%.o: $(SRCDIR)/%.cpp
	@$(call MKDIR,$(OBJDIR_DIST))
	$(CXX) $(CXXFLAGS) -O2 -c -o $@ $<

dist: $(DISTTARGET)
ifeq ($(OS),Windows_NT)
	@if exist "json" (if not exist "$(DISTDIR)/json" mkdir "$(DISTDIR)/json") & copy /y "json\*" "$(DISTDIR)/json\" >nul
	@if exist "mapas" (if not exist "$(DISTDIR)/mapas" mkdir "$(DISTDIR)/mapas") & copy /y "mapas\*" "$(DISTDIR)/mapas\" >nul
	@if exist "assets" (if not exist "$(DISTDIR)/assets" mkdir "$(DISTDIR)/assets") & copy /y "assets\*" "$(DISTDIR)/assets\" >nul
else
	@mkdir -p $(DISTDIR)/json $(DISTDIR)/mapas $(DISTDIR)/assets
	@cp json/* $(DISTDIR)/json/ 2>/dev/null || true
	@cp mapas/* $(DISTDIR)/mapas/ 2>/dev/null || true
	@cp assets/* $(DISTDIR)/assets/ 2>/dev/null || true
endif
	@echo.
	@echo ===========================================
	@echo Distribution package created in .dist/
	@echo $(DISTTARGET) is statically linked
	@echo Share the entire .dist/ folder.
	@echo ===========================================

clean:
	$(call RMDIR,$(OBJDIR))
	$(call RMDIR,$(OBJDIR_DIST))
	$(call DEL,$(TARGET))

run: $(TARGET)
	./$(TARGET)
