//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// rendezvous-hasher.h
// ===================
//
// Header-only, dependency-free implementation of Rendezvous Hashing
// in C99 (a.k.a. Highest Random Weight hashing).
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// License: MIT
//
//
// Documentation
// -------------
//
// Rendezvous Hashing is a technique for consistently mapping keys to
// a set of nodes (or servers) with minimal disruption when nodes are
// added or removed.  It works by assigning each (key, node) pair a
// deterministic "score" and selecting the node with the highest score
// for each key. This approach is widely used in distributed systems,
// load balancers, and caching systems (e.g., for partitioning keys
// across servers) due to its simplicity and excellent balance
// properties.
//
// Features:
//  - Pure C99, header-only (no external dependencies)
//  - Deterministic and reproducible hash assignment
//  - Minimal key movement on node changes
//  - Suitable for both static and dynamic node sets
//
//
// Usage
// -----
//
// Do this:
//
//   #define RENDEZVOUS_HASHER_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define RENDEZVOUS_HASHER_IMPLEMENTATION
//   #include "rendezvous-hasher.h"
//
// If you want to use the built-in hash functions, you need to also
//
//    #define RENDEZVOUS_HASHER_HASHES
//
// before including the header.
//
// You can tune the library by #defining certain values. See the
// "Config" comments under "Configuration" below.
//
// To start, you need to initialize the hasher with the init function.
//
//    RendezvousHasher rh;
//    rendezvous_init(&rh);
//
// You can add and remove nodes in the hasher:
//
//    RendezvousHasherId node1_id = 6969;
//    rendezvous_add_node(&rh, node1_id);
//    rendezvous_remove_node(&rh, node1_id);
//
// And get the node assigned to an item id:
//
//    RendezvousHasherId item_id = 6969;
//    RendezvousHasherHash chosen_node_id; // This will be set below
//    rendezvous_get_node_for(&rh, item_id, &chosen_node_id);
//
// Remember to free all allocated memory.
//
//    rendezvous_free(&rh);
//
//
// Code
// ----
//
// The official git repository of rendezvous-hasher.h is hosted at:
//
//     https://github.com/San7o/rendezvous-hasher.h
//
// This is part of a bigger collection of header-only C99 libraries
// called "micro-headers", contributions are welcome:
//
//     https://github.com/San7o/micro-headers
//

#ifndef RENDEZVOUS_HASHER
#define RENDEZVOUS_HASHER

#define RENDEZVOUS_HASHER_MAJOR 0
#define RENDEZVOUS_HASHER_MINOR 1

#ifdef __cplusplus
extern "C" {
#endif

//
// Configuration
//

// Config: the type of the identifier of a node / item
// Constraint: The id type must support the "+" and "==" operations
#ifndef RENDEZVOUS_HASHER_ID_T
  #define RENDEZVOUS_HASHER_ID_T unsigned int
#endif
  
// Config: the type of an hash returned by the hash function
#ifndef RENDEZVOUS_HASHER_HASH_T
  #define RENDEZVOUS_HASHER_HASHES
  #define RENDEZVOUS_HASHER_HASH_T unsigned int
#endif
  
// Config: hash function to use
// Note: must have the signature
//
//  RENDEZVOUS_HASHER_HASH_T my_hash(RENDEZVOUS_HASHER_ID_T id)
//
// Constraint: The hash type must support the ">" operator
#ifndef RENDEZVOUS_HASHER_HASH
  #define RENDEZVOUS_HASHER_HASHES
  #define RENDEZVOUS_HASHER_HASH rendezvous_hasher_hash_uint32
#endif

// Config: the memory allocator
// Note: should be called like malloc(3)
#ifndef RENDEZVOUS_HASHER_MALLOC
  #include <stdlib.h>
  #define RENDEZVOUS_HASHER_MALLOC malloc
#endif

// Config: function to free the memory
// Note: should be called like free(3)
#ifndef RENDEZVOUS_HASHER_FREE
  #include <stdlib.h>
  #define RENDEZVOUS_HASHER_FREE free
#endif

// Config: Prefix for all functions
// For function inlining, set this to `static inline` and then define
// the implementation in all the files
#ifndef RENDEZVOUS_HASHER_DEF
  #define RENDEZVOUS_HASHER_DEF extern
#endif

//
// Types
//

#define RENDEZVOUS_HASHER_OK                   0
#define RENDEZVOUS_HASHER_ERROR_IS_NULL       -1
#define RENDEZVOUS_HASHER_ERROR_ARGUMENT_NULL -2

typedef RENDEZVOUS_HASHER_HASH_T RendezvousHasherHash;
typedef RENDEZVOUS_HASHER_ID_T RendezvousHasherId;

struct RendezvousHasherNodeList;
typedef struct RendezvousHasherNodeList RendezvousHasherNodeList;

struct RendezvousHasherNodeList {
  RendezvousHasherNodeList* next;
  RendezvousHasherId id;
};

typedef struct {
  RendezvousHasherNodeList *nodes;
} RendezvousHasher;

//
// Function definitions
//

// Initializes the Rendezvous Hasher
RENDEZVOUS_HASHER_DEF int
rendezvous_init(RendezvousHasher *rh);
// Free all allocated memory in the Rendezvous Hasher
RENDEZVOUS_HASHER_DEF int
rendezvous_free(RendezvousHasher *rh);

// Add a node with [id] to the list of nodes of [rh]. O(1) time
RENDEZVOUS_HASHER_DEF int
rendezvous_add_node(RendezvousHasher *rh,
                    RendezvousHasherId id);

// Remove node with [id] to the list of nodes of [rh]. O(n) time
RENDEZVOUS_HASHER_DEF int
rendezvous_remove_node(RendezvousHasher *rh,
                       RendezvousHasherId id);

// Get the [node_id] assigned for [item_id] in [rg]
RENDEZVOUS_HASHER_DEF int
rendezvous_get_node_for(RendezvousHasher *rh,
                        RendezvousHasherId item_id,
                        RendezvousHasherId *node_id);

#ifdef RENDEZVOUS_HASHER_HASHES

// Hash function for unsigned int keys
RENDEZVOUS_HASHER_DEF unsigned int
rendezvous_hasher_hash_uint32(unsigned int a);

#endif // RENDEZVOUS_HASHER_HASHES

//
// Implementation
//

#ifdef RENDEZVOUS_HASHER_IMPLEMENTATION

RENDEZVOUS_HASHER_DEF int rendezvous_init(RendezvousHasher *rh)
{
  if (!rh) return RENDEZVOUS_HASHER_ERROR_IS_NULL;
  rh->nodes = NULL;
  return RENDEZVOUS_HASHER_OK;
}

RENDEZVOUS_HASHER_DEF int rendezvous_free(RendezvousHasher *rh)
{
  if (!rh) return RENDEZVOUS_HASHER_ERROR_IS_NULL;

  RendezvousHasherNodeList *it = rh->nodes;
  while (it)
  {
    RendezvousHasherNodeList *next = it->next;
    RENDEZVOUS_HASHER_FREE(it);
    it = next;
  }
  rh->nodes = NULL;
  
  return RENDEZVOUS_HASHER_OK;
}

RENDEZVOUS_HASHER_DEF int
rendezvous_add_node(RendezvousHasher *rh,
                    RendezvousHasherId id)
{
  if (!rh) return RENDEZVOUS_HASHER_ERROR_IS_NULL;

  RendezvousHasherNodeList *new_node =
    RENDEZVOUS_HASHER_MALLOC(sizeof(RendezvousHasherNodeList));
  new_node->next = rh->nodes;
  new_node->id = id;
  rh->nodes = new_node;
  
  return RENDEZVOUS_HASHER_OK;
}

RENDEZVOUS_HASHER_DEF int
rendezvous_remove_node(RendezvousHasher *rh,
                       RendezvousHasherId id)
{
  if (!rh) return RENDEZVOUS_HASHER_ERROR_IS_NULL;
  if (!rh->nodes) return RENDEZVOUS_HASHER_OK;

  RendezvousHasherNodeList *it = rh->nodes;
  RendezvousHasherNodeList *prev = NULL;
  _Bool found = 0;
  while (it)
  {
    if (it->id == id)
    {
      found = 1;
      break;
    }
    prev = it;
    it = it->next;
  }

  if (found)
  {
    if (prev == NULL)
    {
      free(rh->nodes);
      rh->nodes = NULL;
    }
    else
    {
      prev->next = it->next;
      free(it);
    }
  }
  
  return RENDEZVOUS_HASHER_OK;
}

RENDEZVOUS_HASHER_DEF int
rendezvous_get_node_for(RendezvousHasher *rh,
                        RendezvousHasherId item_id,
                        RendezvousHasherId *node_id)
{
  if (!rh) return RENDEZVOUS_HASHER_ERROR_IS_NULL;
  if (!node_id) return RENDEZVOUS_HASHER_ERROR_ARGUMENT_NULL;
  
  RendezvousHasherNodeList *it = rh->nodes;
  RendezvousHasherHash max_hash = 0;
  RendezvousHasherId chosen_node_id = {0};
  while(it)
  {
    RendezvousHasherId node_id = it->id;
    RendezvousHasherId id_sum = node_id + item_id;
    RendezvousHasherHash id_sum_hash = RENDEZVOUS_HASHER_HASH(id_sum);
    if (id_sum_hash > max_hash)
    {
      max_hash = id_sum_hash;
      chosen_node_id = node_id;
    }
    it = it->next;
  }

  *node_id = chosen_node_id;
  return RENDEZVOUS_HASHER_OK;
}

#ifdef RENDEZVOUS_HASHER_HASHES

RENDEZVOUS_HASHER_DEF unsigned int
rendezvous_hasher_hash_uint32(unsigned int a)
{
    a = (a ^ 61) ^ (a >> 16);
    a = a + (a << 3);
    a = a ^ (a >> 4);
    a = a * 0x27d4eb2d;
    a = a ^ (a >> 15);
    return a;
}

#endif // RENDEZVOUS_HASHER_HASHES

#endif // RENDEZVOUS_HASHER_IMPLEMENTATION

#ifdef __cplusplus
}
#endif

#endif // RENDEZVOUS_HASHER
