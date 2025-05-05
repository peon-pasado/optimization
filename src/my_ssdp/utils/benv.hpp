#pragma once
#include <vector>
#include <cstdlib>   // malloc, free
#include <stdexcept> // bad_alloc, invalid_argument
#include <cstddef>   // size_t
#include <cstdint>   // uint8_t

constexpr size_t DP_BLOCKSIZE = 4096; // Tamaño de cada bloque, puedes ajustar

class Benv {
public:
    explicit Benv(size_t element_size)
        : element_size_(element_size),
          n_per_block_(compute_n_per_block(element_size)),
          free_list_(nullptr),
          n_elements_(0)
    {
        if (element_size_ == 0 || element_size_ > DP_BLOCKSIZE) {
            throw std::invalid_argument("Elemento de tamaño inválido para Benv");
        }
    }

    ~Benv() {
        for (void* block : blocks_) {
            std::free(block);
        }
        blocks_.clear();
    }

    void* alloc_element() {
        if (!free_list_) {
            alloc_block();
        }
        void* p = free_list_;
        free_list_ = *reinterpret_cast<uint8_t**>(p);
        ++n_elements_;
        return p;
    }

    void free_element(void* ptr) {
        if (!ptr) return;
        *reinterpret_cast<uint8_t**>(ptr) = free_list_;
        free_list_ = static_cast<uint8_t*>(ptr);
        --n_elements_;
    }

    void reset_memory() {
        free_list_ = nullptr;
        n_elements_ = 0;
        for (void* block : blocks_) {
            uint8_t* start = static_cast<uint8_t*>(block);
            for (size_t i = 0; i < n_per_block_; ++i) {
                uint8_t* elem = start + i * element_size_;
                *reinterpret_cast<uint8_t**>(elem) = free_list_;
                free_list_ = elem;
            }
        }
    }

    size_t used_memory() const {
        return DP_BLOCKSIZE * blocks_.size();
    }

private:
    void alloc_block() {
        void* raw = std::malloc(DP_BLOCKSIZE);
        if (!raw) {
            throw std::bad_alloc();
        }
        blocks_.push_back(raw);
        // añadir nuevos elementos al free_list_
        uint8_t* start = static_cast<uint8_t*>(raw);
        for (size_t i = 0; i < n_per_block_; ++i) {
            uint8_t* elem = start + i * element_size_;
            *reinterpret_cast<uint8_t**>(elem) = free_list_;
            free_list_ = elem;
        }
    }

    static size_t compute_n_per_block(size_t element_size) {
        return DP_BLOCKSIZE / element_size;
    }

private:
    size_t element_size_;       // Tamaño de cada elemento
    size_t n_per_block_;        // Elementos que caben por bloque
    uint8_t* free_list_;        // Puntero al siguiente elemento libre
    std::vector<void*> blocks_; // Bloques reservados
    int n_elements_;            // Elementos actualmente en uso
};
