#ifndef CACHE_H
#define CACHE_H

#include <RconCLI/debug.h>
#include <unordered_map>
#include <iostream>
#include <cstring>

class Cache {

    private:
    struct Node {

        int size = 0; // value size
        int key = 0;
        char* value = NULL;
        Node* prev = NULL;
        Node* next = NULL;

    };

    // size is the current size of the hash, whereas capacity is
    // the overal capacity of the hash map
    int size = 0;
    int capacity = 0;
    int count = 0;

    // For functions Reverse() and Forward().
    Node* ReverseHead;
    Node* ReverseSave;
    Node* ForwardTail;

    // unordered map for the hash map; this is the most useful
    // because the keys will simply be incrementing integers
    // therefore they will be organized accordingly by unordered_map
    std::unordered_map<int, Node*> HashMap;

    Node* head;
    Node* tail; 

    void Push(Node* node);

    public:
    // Constructor; initializes head, tail, and capacity.
    Cache(int CacheCapacity);
    // Deconstructor; deletes all list nodes. 
    ~Cache();
    // Reverse through list.
    char* Reverse();
    // Traverse forward through list.
    char* Forward();
    // Insert value into list at head.
    void Insert(char* value);
    // Evict a node from list.
    void Evict(Node* node);
    // Evict tail node from list.
    void EvictTail();

};

#endif