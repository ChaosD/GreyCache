
ROOT_DIR=$(CURDIR)
CACHE_SCHEME=${ROOT_DIR}/cache_scheme
PREFETCHER=${ROOT_DIR}/prefetcher
BACKEND=${ROOT_DIR}/backend
OBJ_DIR=${ROOT_DIR}/obj


EXTENSION=cpp
COMMON_OBJS= $(patsubst $(ROOT_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(ROOT_DIR)/*.$(EXTENSION)))
CACHE_OBJS= $(patsubst $(CACHE_SCHEME)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(CACHE_SCHEME)/*.$(EXTENSION)))
PREFETCH_OBJS= $(patsubst $(PREFETCHER)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(PREFETCHER)/*.$(EXTENSION)))
BACKEND_OBJS= $(patsubst $(BACKEND)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(BACKEND)/*.$(EXTENSION)))

CXX = g++
CFLAGS = -c -Wall --std=c++11 -pthread -fPIC -D_FILE_OFFSET_BITS=64
LFLAGS = -Wall --std=c++11 -shared -fPIC
LIBS = -lpthread -lcephfs

.PHONY: all clean rebuild

all: Duplex_test

${OBJ_DIR}/%.o:${ROOT_DIR}/%.${EXTENSION}
	${CXX} $< -o $@ ${CFLAGS} ${INCLUDE_DIR}

${OBJ_DIR}/%.o:${CACHE_SCHEME}/%.${EXTENSION}
	${CXX} $< -o $@ ${CFLAGS} ${INCLUDE_DIR}

${OBJ_DIR}/%.o:${PREFETCHER}/%.${EXTENSION}
	${CXX} $< -o $@ ${CFLAGS} ${INCLUDE_DIR}

${OBJ_DIR}/%.o:${BACKEND}/%.${EXTENSION}
	${CXX} $< -o $@ ${CFLAGS} ${INCLUDE_DIR}


libgreycache.so: $(COMMON_OBJS) $(CACHE_OBJS) $(PREFETCH_OBJS) $(BACKEND_OBJS)
	$(CXX) $^ -o $@ $(LFLAGS) $(LIBS)

rebuild: clean all

clean:
	rm -rf ${COMMON_OBJS} ${CACHE_OBJS} ${PREFETCH_OBJS} ${BACKEND_OBJS} libgreycache.so

install:
	cp -f libgreycache.so /usr/local/lib/
	cp -f libgreycache.h /usr/local/include