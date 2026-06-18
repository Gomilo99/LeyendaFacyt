CXX          := g++
CXXFLAGS     := -std=c++17 -Wall -Wextra -Wpedantic -I. -MMD -MP
LDFLAGS      :=
STATIC_LDFLAGS := -static-libgcc -static-libstdc++ -static
SRCDIR       := src
OBJDIR       := obj
OBJDIR_DIST  := obj-dist
TARGET       := leyenda.exe
DISTDIR      := .dist
DISTTARGET   := $(DISTDIR)/$(TARGET)
DATA_DIRS    := json mapas assets

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
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

# Distribution build (statically linked, optimized)
$(DISTTARGET): $(OBJS_DIST)
	@if not exist "$(DISTDIR)" mkdir "$(DISTDIR)"
	$(CXX) $(CXXFLAGS) -O2 $(STATIC_LDFLAGS) -o $@ $^

$(OBJDIR_DIST)/%.o: $(SRCDIR)/%.cpp
	@if not exist "$(OBJDIR_DIST)" mkdir "$(OBJDIR_DIST)"
	$(CXX) $(CXXFLAGS) -O2 -c -o $@ $<

dist: $(DISTTARGET)
	@if exist "json" (if not exist "$(DISTDIR)/json" mkdir "$(DISTDIR)/json") & copy /y "json\*" "$(DISTDIR)/json\" >nul
	@if exist "mapas" (if not exist "$(DISTDIR)/mapas" mkdir "$(DISTDIR)/mapas") & copy /y "mapas\*" "$(DISTDIR)/mapas\" >nul
	@if exist "assets" (if not exist "$(DISTDIR)/assets" mkdir "$(DISTDIR)/assets") & copy /y "assets\*" "$(DISTDIR)/assets\" >nul
	@echo.
	@echo ===========================================
	@echo Distribution package created in .dist/
	@echo $(DISTTARGET) is statically linked
	@echo Share the entire .dist/ folder.
	@echo ===========================================

clean:
	if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	if exist "$(OBJDIR_DIST)" rmdir /s /q "$(OBJDIR_DIST)"
	if exist "$(TARGET)" del /f /q "$(TARGET)"

run: $(TARGET)
	$(TARGET)
