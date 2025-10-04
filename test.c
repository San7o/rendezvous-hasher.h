// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define RENDEZVOUS_HASHER_IMPLEMENTATION
#include "rendezvous-hasher.h"

#include <assert.h>

#include <stdio.h>

void print_and_check(RendezvousHasher *rh, RendezvousHasherId item_id)
{
  printf("========================================================\n");
  printf("Calculating node for item %u\n", item_id);
  
  RendezvousHasherHash chosen_node_id;
  assert(rendezvous_get_node_for(rh, item_id, &chosen_node_id) == RENDEZVOUS_HASHER_OK);

  printf("Assigned node id: %u\n", chosen_node_id);
  printf("Node ids and the hash of node_id + item_id:\n");
  RendezvousHasherNodeList *it = rh->nodes;
  RendezvousHasherId max_id = {0};
  RendezvousHasherHash max_hash = {0};
  while (it)
  {
    RendezvousHasherId id_sum = it->id + item_id;
    RendezvousHasherHash id_sum_hash = RENDEZVOUS_HASHER_HASH(id_sum);
    printf("  - node_id: %-7u id_sum_hash: %u\n", it->id, id_sum_hash);

    if (id_sum_hash > max_hash)
    {
      max_hash = id_sum_hash;
      max_id = it->id;
    }
    it = it->next;
  }
  assert(chosen_node_id == max_id);

  printf("Test successful\n");
  return;
}

int main(void)
{
  RendezvousHasher rh;
  assert(rendezvous_init(&rh) == RENDEZVOUS_HASHER_OK);
  
  assert(rendezvous_add_node(&rh, 6969) == RENDEZVOUS_HASHER_OK);
  assert(rendezvous_add_node(&rh, 420) == RENDEZVOUS_HASHER_OK);
  assert(rendezvous_add_node(&rh, 7777) == RENDEZVOUS_HASHER_OK);

  RendezvousHasherId item_id = 123;
  print_and_check(&rh, item_id);

  RendezvousHasherId item_id2 = 456;
  print_and_check(&rh, item_id2); 

  RendezvousHasherId item_id3 = 23748274;
  print_and_check(&rh, item_id3); 
  
  assert(rendezvous_free(&rh) == RENDEZVOUS_HASHER_OK);
  assert(rh.nodes == NULL);
  return 0;
}
