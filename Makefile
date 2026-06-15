CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -I. -MMD -MP
SRCDIR   := src
OBJDIR   := obj
TARGET   := batalla.exe

SRCS := $(wildcard $(SRCDIR)/*.cpp)
OBJS := $(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))
DEPS := $(OBJS:.o=.d)

.PHONY: all clean run

all: $(TARGET)

-include $(DEPS)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@if not exist "$(OBJDIR)" mkdir "$(OBJDIR)"
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	if exist "$(OBJDIR)" rmdir /s /q "$(OBJDIR)"
	if exist "$(TARGET)" del /f /q "$(TARGET)"

run: $(TARGET)
	$(TARGET)
