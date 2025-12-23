OUT := ./out
OBJS := $(addprefix $(OUT)/,main.o utils.o tokenizer.o)
BIN := $(OUT)/haruka
CFLAGS := -Wall -Werror -Wpedantic


$(BIN) : $(OBJS)
	cc -o $(BIN) $(OBJS)

$(OUT) :
	mkdir out

$(OBJS): | $(OUT)

$(OUT)/main.o : ./src/main.c ./src/utils.h ./src/tokenizer.h
	cc $< -c -o $@ $(CFLAGS)

$(OUT)/utils.o : ./src/utils.c ./src/utils.h
	cc $< -c -o $@ $(CFLAGS)

$(OUT)/tokenizer.o : ./src/tokenizer.c ./src/tokenizer.h ./src/utils.h
	cc $< -c -o $@ $(CFLAGS)

.PHONY : clean
clean :
	rm -r $(OUT)
