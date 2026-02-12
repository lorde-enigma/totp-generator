# ═══════════════════════════════════════════════════════════════════════════════
#  TOTP Generator - Makefile
#  RFC 6238 compliant TOTP code generator
# ═══════════════════════════════════════════════════════════════════════════════

# ─────────────────────────────────────────────────────────────────────────────────
#  Configuration
# ─────────────────────────────────────────────────────────────────────────────────

NAME        := totp
VERSION     := 1.0.0

CXX         := g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -Werror -O2 -I$(INCDIR)
LDFLAGS     := -lssl -lcrypto

# ─────────────────────────────────────────────────────────────────────────────────
#  Directories
# ─────────────────────────────────────────────────────────────────────────────────

SRCDIR      := src
INCDIR      := include
OBJDIR      := obj

PREFIX      := /usr/local
BINDIR      := $(PREFIX)/bin
MANDIR      := $(PREFIX)/share/man/man1

# ─────────────────────────────────────────────────────────────────────────────────
#  Sources and Objects
# ─────────────────────────────────────────────────────────────────────────────────

SOURCES     := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS     := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
HEADERS     := $(wildcard $(INCDIR)/*.hpp)

# ─────────────────────────────────────────────────────────────────────────────────
#  Colors
# ─────────────────────────────────────────────────────────────────────────────────

RED         := \033[0;31m
GREEN       := \033[0;32m
YELLOW      := \033[0;33m
BLUE        := \033[0;34m
MAGENTA     := \033[0;35m
CYAN        := \033[0;36m
RESET       := \033[0m
BOLD        := \033[1m

# ─────────────────────────────────────────────────────────────────────────────────
#  Targets
# ─────────────────────────────────────────────────────────────────────────────────

.PHONY: all clean fclean re install uninstall debug release info help

all: $(NAME)
	@echo ""
	@echo "$(GREEN)$(BOLD)  ✓ build complete$(RESET)"
	@echo "$(CYAN)  → run: ./$(NAME) <secret>$(RESET)"
	@echo ""

$(NAME): $(OBJECTS)
	@echo "$(MAGENTA)[link]$(RESET) $(NAME)"
	@$(CXX) $(OBJECTS) -o $(NAME) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJDIR)
	@echo "$(BLUE)[compile]$(RESET) $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

# ─────────────────────────────────────────────────────────────────────────────────
#  Cleaning
# ─────────────────────────────────────────────────────────────────────────────────

clean:
	@echo "$(YELLOW)[clean]$(RESET) removing object files"
	@rm -rf $(OBJDIR)

fclean: clean
	@echo "$(YELLOW)[clean]$(RESET) removing $(NAME)"
	@rm -f $(NAME)

re: fclean all

# ─────────────────────────────────────────────────────────────────────────────────
#  Installation
# ─────────────────────────────────────────────────────────────────────────────────

install: $(NAME)
	@echo "$(GREEN)[install]$(RESET) installing $(NAME) to $(BINDIR)"
	@install -d $(BINDIR)
	@install -m 755 $(NAME) $(BINDIR)/$(NAME)
	@echo ""
	@echo "$(GREEN)$(BOLD)  ✓ installed successfully$(RESET)"
	@echo "$(CYAN)  → run: $(NAME) <secret>$(RESET)"
	@echo ""

uninstall:
	@echo "$(RED)[uninstall]$(RESET) removing $(NAME) from $(BINDIR)"
	@rm -f $(BINDIR)/$(NAME)
	@echo ""
	@echo "$(YELLOW)  ✓ uninstalled$(RESET)"
	@echo ""

# ─────────────────────────────────────────────────────────────────────────────────
#  Debug/Release builds
# ─────────────────────────────────────────────────────────────────────────────────

debug: CXXFLAGS += -g -DDEBUG -O0
debug: re
	@echo "$(YELLOW)  [debug build]$(RESET)"

release: CXXFLAGS += -O3 -DNDEBUG -march=native
release: re
	@echo "$(GREEN)  [release build]$(RESET)"

# ─────────────────────────────────────────────────────────────────────────────────
#  Info and Help
# ─────────────────────────────────────────────────────────────────────────────────

info:
	@echo ""
	@echo "$(BOLD)  ┌─────────────────────────────────────┐$(RESET)"
	@echo "$(BOLD)  │         TOTP Generator v$(VERSION)       │$(RESET)"
	@echo "$(BOLD)  └─────────────────────────────────────┘$(RESET)"
	@echo ""
	@echo "  $(CYAN)name:$(RESET)      $(NAME)"
	@echo "  $(CYAN)version:$(RESET)   $(VERSION)"
	@echo "  $(CYAN)compiler:$(RESET)  $(CXX)"
	@echo "  $(CYAN)flags:$(RESET)     $(CXXFLAGS)"
	@echo "  $(CYAN)libs:$(RESET)      $(LDFLAGS)"
	@echo "  $(CYAN)prefix:$(RESET)    $(PREFIX)"
	@echo ""

help:
	@echo ""
	@echo "$(BOLD)  available targets:$(RESET)"
	@echo ""
	@echo "    $(GREEN)make$(RESET)           build the project"
	@echo "    $(GREEN)make clean$(RESET)     remove object files"
	@echo "    $(GREEN)make fclean$(RESET)    remove all build artifacts"
	@echo "    $(GREEN)make re$(RESET)        full rebuild"
	@echo "    $(GREEN)make install$(RESET)   install to $(BINDIR)"
	@echo "    $(GREEN)make uninstall$(RESET) remove from $(BINDIR)"
	@echo "    $(GREEN)make debug$(RESET)     build with debug symbols"
	@echo "    $(GREEN)make release$(RESET)   optimized release build"
	@echo "    $(GREEN)make info$(RESET)      show build configuration"
	@echo "    $(GREEN)make help$(RESET)      show this help"
	@echo ""
