#include <algorithm>
#include <cstddef>
#include <functional>
#include <stdexcept>
#include <vector>

using namespace std;

const int maxColision = 3;
const float maxFillFactor = 0.8;

template<typename TK, typename TV>
struct ChainHashNode {
    TK key;
    TV value;
    size_t hashcode;
    ChainHashNode<TK, TV>* next;

    ChainHashNode<TK, TV>(TK key, TV value, size_t hashcode): key(key), value(value), hashcode(hashcode), next(nullptr) {}
    ChainHashNode<TK, TV>(TK key, TV value, size_t hashcode, ChainHashNode<TK, TV>* next): key(key), value(value), hashcode(hashcode), next(next) {}
};

template<typename TK, typename TV>
class ChainHashListIterator {
    ChainHashNode<TK, TV>* current;

public:
    ChainHashListIterator(ChainHashNode<TK, TV>* head): current(head) {}

    ChainHashListIterator<TK, TV>& operator++() {
        current = current->next;
        return *this;
    }

    bool operator==(const ChainHashListIterator<TK, TV>& other) const {
        return current == other.current;
    }

    bool operator!=(const ChainHashListIterator<TK, TV>& other) const {
        return current != other.current;
    }

    ChainHashNode<TK, TV>& operator*() {
        if (!current) { throw std::out_of_range("Iterador fuera de rango"); }
        return *current;
    }
};

template<typename TK, typename TV>
class ChainHash {
private:
    typedef ChainHashNode<TK, TV> Node;
    typedef ChainHashListIterator<TK, TV> Iterator;

	Node** array;  // array de punteros a Node
    int nsize; // total de elementos <key:value> insertados
	int capacity; // tamanio del array
	int *bucket_sizes; // guarda la cantidad de elementos en cada bucket
	int usedBuckets; // cantidad de buckets ocupados (con al menos un elemento)

public:
    ChainHash(int initialCapacity = 10) {
		this->capacity = initialCapacity; 
		this->array = new Node*[capacity]();  
		this->bucket_sizes = new int[capacity]();
		this->nsize = 0;
		this->usedBuckets = 0;
	}

	TV get(TK key) {
		size_t hashcode = getHashCode(key);
		size_t index = hashcode % capacity;
		
		Node* current = this->array[index];
		while(current != nullptr){
			if(current->key == key) return current->value;
			current = current->next;
		}
		throw std::out_of_range("Key no encontrado");
	}
	
	int size() { return this->nsize; }	

	int bucket_count() { return this->capacity; }

	int bucket_size(int index) { 
		if(index < 0 || index >= this->capacity) throw std::out_of_range("Indice de bucket invalido");
		return this->bucket_sizes[index]; 
	}	
	
	// TODO: implementar los siguientes métodos
	void set(TK key, TV value) {
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* current = array[index];
        while (current != nullptr) {
            if (current->key == key) {
                current->value = value;
                return;
            }
            current = current->next;
        }

        Node* newNode = new Node(key, value, hashcode, array[index]);
        array[index] = newNode;
        
        nsize++;
        if (bucket_sizes[index] == 0) { usedBuckets++; }
        bucket_sizes[index]++;
        if (fillFactor() > maxFillFactor || bucket_sizes[index] > maxColision) { rehashing(); }
    }

	bool remove(TK key) {
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;

        Node* previous = nullptr;
        Node* current = array[index];
        while (current != nullptr) {
            if (current->key == key) {
                if (previous == nullptr) {
                    array[index] = current->next;
                } else {
                    previous->next = current->next;
                }
                delete current;

                nsize--;
                bucket_sizes[index]--;
                if (bucket_sizes[index] == 0) { usedBuckets--; }

                return true;
            }

            previous = current;
            current = current->next;
        }

        return false;
    }

	bool contains(TK key) {
        size_t hashcode = getHashCode(key);
        size_t index = hashcode % capacity;
        Node* current = array[index];
        
        while (current != nullptr) {
            if (current->key == key) return true;
            current = current->next;
        }

        return false;
    }

	Iterator begin(int index) {
        if (index < 0 || index >= capacity) { throw std::out_of_range("Indice de bucket invalido"); }
        return Iterator(array[index]);
    }

	Iterator end(int index) {
        if (index < 0 || index >= capacity) { throw std::out_of_range("Indice de bucket invalido"); }
        return Iterator(nullptr);
    }

private:
	double fillFactor() {
		return (double)this->usedBuckets / (double)this->capacity;
	}	

	size_t getHashCode(TK key) {
		std::hash<TK> ptr_hash;
		return ptr_hash(key);
	}

	//TODO: implementar rehashing
	void rehashing() {
        Node** oldArray = array;
        int* oldBucketSizes = bucket_sizes;
        int oldCapacity = capacity;

        capacity = capacity * 2;
        array = new Node*[capacity]();
        bucket_sizes = new int[capacity]();
        usedBuckets = 0;

        for (size_t i = 0; i < oldCapacity; i++) {
            Node* current = oldArray[i];
            while(current != nullptr) {
                Node* next = current->next;
                size_t new_index = current->hashcode % this->capacity;
                current->next = array[new_index];
                array[new_index] = current;
                bucket_sizes[new_index]++;
                if (bucket_sizes[new_index] == 1) {
                    usedBuckets++;
                }
                current = next;
            }
        }

        delete[] oldArray;
        delete[] oldBucketSizes;
    }

public:
	// TODO: implementar destructor
	~ChainHash() {
        for (size_t i = 0; i < capacity; i++) {
            Node* current = array[i];
            while (current != nullptr) {
                Node* next = current->next;
                delete current;
                current = next;
            }
        }

        delete[] array;
        delete[] bucket_sizes;
    }
};

