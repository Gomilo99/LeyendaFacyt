CXX      := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -Wpedantic -I.
SRCDIR   := src
OBJDIR   := obj
TARGET   := batalla.exe

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	if exist "$(TARGET)" del /f /q "$(TARGET)"

run: $(TARGET)
	./$(TARGET)
