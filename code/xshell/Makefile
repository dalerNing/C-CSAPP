cc 	 = gcc
prom = xshell
deps = $(shell find ./ -name "*.h")
src  = $(shell find ./ -name "*.c")
obj  = $(src:%.c=%.o)

$(prom): $(obj)
	$(cc) -o $(prom) $(obj) -m64 -lpthread

%.o: %.c $(deps)
	$(cc) -c $< -o $@

clean:
	rm -rf $(obj) $(prom)