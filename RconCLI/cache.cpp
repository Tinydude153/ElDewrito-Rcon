#include <RconCLI/cache.h>

Cache::Cache(int CacheCapacity) {

    Cache::capacity = CacheCapacity;
    head = new Cache::Node;
    tail = new Cache::Node;
    head->next = tail;
    tail->prev = head;

    Cache::ReverseHead = 0;

}

Cache::~Cache() {

    Cache::Node* current = this->head;
    Cache::Node* cnext = NULL;
    if (current) {
        if (current != NULL) {

            if (current->next != NULL) {
                while (current->next) {

                    if (current->next) cnext = current->next;
                    if (current) delete current;
                    current = cnext;
                    if (!current->next) break;

                }
                current = 0;

            }

        }
    }

}

void Cache::Push(Cache::Node* node) {

    PRINTDEBUG("\nCURRENT_CACHE_NODE_KEY: %d", node->key);
    PRINTDEBUG("\nCURRENT_CACHE_NODE_VALUE: %s", node->value);
    PRINTDEBUG("\nCURRENT_CACHE_NODE_SIZE: %d", node->size);
    node->next = head->next;
    node->next->prev = node;
    node->prev = head;
    head->next = node;

}

void Cache::Evict(Cache::Node* node) {

    node->prev->next = node->next;
    node->next->prev = node->prev;

}

void Cache::EvictTail() {

    Cache::Node* t = Cache::tail->prev;
    t->prev->next = t->next;
    t->next->prev = t->prev;

}

void Cache::Insert(char* value) {
    
    if (!value) return;
    Cache::Node* node = new Cache::Node();
    node->size = strlen(value) + 1;
    node->key = Cache::size;
    node->value = value;
    Cache::Push(node);
    Cache::size++;

    if (Cache::size >= Cache::capacity) {
        
        Cache::Evict(tail->prev);

    }

    Cache::count = Cache::size - 1;
    Cache::ReverseHead = Cache::head;
    Cache::ForwardTail = Cache::tail;
    
    // DEBUG : View all node structures each entry to ensure that there are not
    // too many, too little, no missed-deletions, etc.
    
    #ifdef DEBUG
        Cache::Node* cNode = Cache::head;
        Cache::Node* cNext;
        while (cNode->next) {

            cNext = cNode->next;
            if (cNode->value) PRINTDEBUG("\nCACHE_NODE: %s", cNode->value);
            cNode = cNext;

        }
    #endif

}

char* Cache::Reverse() {

    char* cmd;
    if (!Cache::ReverseHead) return NULL;
    if (Cache::ReverseHead->next) {

        if (!Cache::ReverseHead->next->value) {

            return NULL;
        
        }
        cmd = Cache::ReverseHead->next->value; 
        Cache::ReverseHead = Cache::ReverseHead->next;
        Cache::ReverseSave = Cache::ReverseHead;
        Cache::count--;
        if (cmd) return cmd;

    }

    return NULL;

}

char* Cache::Forward() {

    if (!Cache::ReverseHead) return NULL;
    if (Cache::ReverseHead->prev) {

        if (!Cache::ReverseHead->prev->value) {

            return NULL;

        }
        char* cmd = Cache::ReverseHead->prev->value;
        Cache::ReverseHead = Cache::ReverseHead->prev;
        return cmd;
        
    }

    return NULL;

}