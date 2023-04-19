#ifndef H_GREY_CACHE_SCHEME_LRU
#define H_GREY_CACHE_SCHEME_LRU


#include "../cache_server.h"
#include <map>
#include <list>
#include <iostream>
#include <mutex>

template <class K>
class lru_cache: public cache_scheme_abs<K>{

private:
    std::list<K> _list;
    std::map<K, typename std::list<K>::iterator> _map;
    std::mutex mtx;

public:

    bool enter(K &key)
    {
        mtx.lock(); 
    	// Check whether the key exists.
        auto iter = _map.find(key);
        // If the key is in the cache, remove it.
        if(iter != _map.end()){
            _list.erase(iter->second);
        }
        // Add the key at the front of the LRU queue
        _list.push_front(key);
        // insert the pointer into the hash table
        _map[key] = _list.begin();
        mtx.unlock(); 
        return true;
    }

    bool pre_enter(K &key){
        return enter(key);
    }

    void evict(uint64_t evict_number, K* p_request){
        mtx.lock(); 
        for(int i = 0; i < evict_number; i++){
            // the tailed items in the list is to be evicted
            p_request[i] = _list.back();
            // Remove the keys from hash table
            _map.erase(_list.back());
            // Drop out the keys from list
            _list.pop_back();    
        }
        mtx.unlock();
    }
};


#endif