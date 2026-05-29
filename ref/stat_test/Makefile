CC ?= cc
CFLAGS ?= -O3 -std=c11 -Wall -Wextra -I.. -I../sha3/fips202-ref -pthread
LDFLAGS ?= -pthread

BIN := stat_test_gen
SRCS := stat_test_gen.c stat_test_owf.c ../fields.c ../randomness.c ../sha3/fips202-ref/fips202.c

.PHONY: all clean

all: $(BIN)

$(BIN): $(SRCS)
	$(CC) $(CFLAGS) -o $@ $(SRCS) $(LDFLAGS)

clean:
	rm -f $(BIN)
