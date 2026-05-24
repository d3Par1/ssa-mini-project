# Top-level Makefile — будує обидві частини мініпроєкту.

.PHONY: all clean pipeline minishell

all: pipeline minishell

pipeline:
	@$(MAKE) -C pipeline

minishell:
	@$(MAKE) -C minishell

clean:
	@$(MAKE) -C pipeline clean
	@$(MAKE) -C minishell clean
