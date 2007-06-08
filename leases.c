/*
 *    natpmp - an implementation of NAT-PMP
 *    Copyright (C) 2007  Adrian Friedli
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License along
 *   with this program; if not, write to the Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <stdlib.h>
#include <string.h>

#include "die.h"
#include "leases.h"

/* list of leases */
lease * leases;
/* number of allocated leases */
int lease_a;
/* number of leases */
int lease_c;

/* function that reallocates space for at minimum amount leases */
void allocate_leases(const int amount) {
	if (amount > lease_a) {
		lease_a += ALLOCATE_AMOUNT;
	}
	else if (lease_a >= amount + 2 * ALLOCATE_AMOUNT) {
		lease_a -= ALLOCATE_AMOUNT;
	}
	else {
		return;
	}
	leases = realloc(leases, lease_a * sizeof(lease));
	if (leases == NULL) p_die("realloc");
}

/* function that adds a lease to the list of leases */
int add_lease(const lease * a) {
	allocate_leases(lease_c + 1);
	memcpy(&leases[lease_c], a, sizeof(lease));
	return lease_c++;
}

/* function that removes a lease from the list of leases */
void remove_lease(const int i) {
	lease_c--;
	memmove(&leases[i], &leases[i+1], (lease_c-i) * sizeof(lease));
	allocate_leases(lease_c);
}

/* function that returns the index of a lease pointer */
int get_index_by_pointer(const lease * a) {
	int i = (a - leases) / sizeof(lease);
	if (i >= 0 && i <= lease_c) return i;
	else {
		die("remove_lease_by_pointer(): invalid pointer");
	}
}

/* function that removes a lease from the list of leases with a given pointer to the lease */
void remove_lease_by_pointer(const lease * a) {
	remove_lease( get_index_by_pointer(a) );
}

/* function that returns a lease pointer by mapped port number, NULL if port number is still unmapped */
lease * get_lease_by_port(const uint16_t port) {
	int i;
	for (i=0; i<lease_c; i++) {
		if (leases[i].mapped_port == port) return &leases[i];
	}
	return NULL;
}

/* function that returns a lease pointer by client ip address and private port numnber, NULL if no lease found */
lease * get_lease_by_client_port(const uint32_t client, const uint16_t port) {
	int i;
	for (i=0; i<lease_c; i++) {
		if (leases[i].client == client && leases[i].private_port == port) return &leases[i];
	}
	return NULL;
}

/* function that returns a pointer to the next lease by client ip address, NULL if no leases found, prev is the pointer to the lease from where to search from, NULL to search from beginning */
lease * get_next_lease_by_client(const uint32_t client, const lease * prev) {
	int i;
	if (prev == NULL) i = 0;
	else i = get_index_by_pointer(prev);
	for (; i<lease_c; i++) {
		if (leases[i].client == client) return &leases[i];
	}
	return NULL;
}

/* function that returns a pointer to the next expired lease, NULL if no leases found, provide the actual time with now, prev is the pointer to the lease from where to search from, NULL to search from beginning */
lease * get_next_expired_lease(const uint32_t now, const lease * prev) {
	int i;
	if (prev == NULL) i = 0;
	else i = get_index_by_pointer(prev);
	for (; i<lease_c; i++) {
		if (leases[i].expires <= now) return &leases[i];
	}
	return NULL;
}