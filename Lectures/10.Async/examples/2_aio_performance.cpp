#include "common.hpp"
#include <aio.h>
#include <random>
#include <unistd.h>
#include <fcntl.h>
#include <span>
#include <optional>

const size_t SIZE = 128*1024*1024;
const size_t BLOCK_SIZE = 64*1024;
const char FILENAME[] = "testfile_";

static_assert(SIZE >= BLOCK_SIZE);
static std::default_random_engine e{0};
static std::bernoulli_distribution d{0.5};

void fill_random(std::span<char> s){
    for(auto& c: s)
        c = (char)(d(e));
}

int create_file(){
    unlink(FILENAME);
    int fd = check(open(FILENAME, O_RDWR|O_CREAT|O_EXCL, S_IRUSR|S_IWUSR));
    return fd;
}

void sequential(){
    ScopedTimer _{"Sequential"};
    std::vector<char> buffer;
    buffer.resize(BLOCK_SIZE);
    auto fd = create_file();

    for(size_t i = 0; i < SIZE; i+= BLOCK_SIZE){
        auto size = std::min(SIZE - i, BLOCK_SIZE);
        fill_random({buffer.data(), size});
        check(write(fd, buffer.data(), size));
    }
}

void asynchronous(){
    ScopedTimer _{"Asynchronous"};

    auto fd = create_file();
    std::vector<char> data;
    data.reserve(BLOCK_SIZE*2);

    aiocb cb{};
    char* buffers[2] = {&data[0], &data[BLOCK_SIZE]};

    bool has_request = false;
    int block_idx = 0;
    for(size_t i = 0; i < SIZE; i+= BLOCK_SIZE){

        auto size = std::min(SIZE - i, BLOCK_SIZE);
        fill_random({buffers[block_idx], size});

        if(has_request){
            aiocb* waitlist[] = {&cb};
            check(aio_suspend(waitlist, 1, nullptr));
            check(aio_return(&cb));
        }
        cb = aiocb{.aio_fildes = fd,
                   .aio_buf = buffers[block_idx],
                   .aio_nbytes = size,
                   .aio_offset=(off_t)i};
        check(aio_write(&cb));
        has_request = true;
        block_idx ^= 1;

    }
}

int main(int argc, char** argv){
    e.seed(0);
    sequential();
    e.seed(0);
    asynchronous();
}