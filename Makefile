SRCDIR      := src
INCDIR      := include
OBJDIR      := obj

NAME        := totp
VERSION     := 1.0.0

CXX         := g++
CXXFLAGS    := -std=c++17 -Wall -Wextra -Werror -O2 -I$(INCDIR)
LDFLAGS     := -lssl -lcrypto

PREFIX      := /usr/local
BINDIR      := $(PREFIX)/bin

SOURCES     := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS     := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))
HEADERS     := $(wildcard $(INCDIR)/*.hpp)

RED         := \033[0;31m
GREEN       := \033[0;32m
YELLOW      := \033[0;33m
BLUE        := \033[0;34m
MAGENTA     := \033[0;35m
CYAN        := \033[0;36m
RESET       := \033[0m
BOLD        := \033[1m

.PHONY: all clean fclean re install uninstall debug release

all: $(NAME)
	@printf "\n$(GREEN)$(BOLD)  ✓ build complete$(RESET)\n"
	@printf "$(CYAN)  → run: ./$(NAME) <secret>$(RESET)\n\n"

$(NAME): $(OBJECTS)
	@printf "$(MAGENTA)[link]$(RESET) $(NAME)\n"
	@$(CXX) $(OBJECTS) -o $(NAME) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJDIR)
	@printf "$(BLUE)[compile]$(RESET) $<\n"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJDIR):
	@mkdir -p $(OBJDIR)

clean:
	@printf "$(YELLOW)[clean]$(RESET) removing object files\n"
	@rm -rf $(OBJDIR)

fclean: clean
	@printf "$(YELLOW)[clean]$(RESET) removing $(NAME)\n"
	@rm -f $(NAME)

re: fclean all

install: $(NAME)
	@printf "$(GREEN)[install]$(RESET) installing $(NAME) to $(BINDIR)\n"
	@install -d $(BINDIR)
	@install -m 755 $(NAME) $(BINDIR)/$(NAME)
	@printf "\n$(GREEN)$(BOLD)  ✓ installed$(RESET)\n\n"

uninstall:
	@printf "$(RED)[uninstall]$(RESET) removing $(NAME) from $(BINDIR)\n"
	@rm -f $(BINDIR)/$(NAME)

debug: CXXFLAGS += -g -DDEBUG -O0
debug: re

release: CXXFLAGS += -O3 -DNDEBUG -march=native
release: re
	@echo ""
