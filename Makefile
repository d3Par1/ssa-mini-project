# Top-level Makefile — будує обидві частини мініпроєкту

.PHONY: all clean pipeline minishell

all: pipeline minishell

pipeline:
	@$(MAKE) -C pipeline

minishell:
	@if [ -f minishell/Makefile ]; then \
		$(MAKE) -C minishell; \
	else \
		echo "minishell/ ще не наповнено Артемом — пропускаю."; \
	fi

clean:
	@$(MAKE) -C pipeline clean
	@if [ -f minishell/Makefile ]; then $(MAKE) -C minishell clean; fi
