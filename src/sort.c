/* vim: set tabstop=4 expandtab shiftwidth=4 softtabstop=4: */

/*
 * Copyright 2015 University of Piemonte Orientale, Computer Science Institute
 *
 * This file is part of UPOalglib.
 *
 * UPOalglib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * UPOalglib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with UPOalglib.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <assert.h>
#include "sort_private.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <upo/utility.h>

/* Definizione macro per il calcolo sicuro degli indirizzi di memoria */
#define ELEMENT_ADDR(base, index, size) ((void *)((char *)(base) + ((index) * (size))))

/* Prototipi per le funzioni interne (static) */
static void upo_merge_sort_rec(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp);
static void upo_merge_sort_merge(void *base, size_t size, size_t lo, size_t mid, size_t hi, upo_sort_comparator_t cmp);
static size_t upo_quick_sort_partition(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp);
static void upo_quick_sort_rec(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp);


void upo_insertion_sort(void *base, size_t n, size_t size, upo_sort_comparator_t cmp)
{
    // Controllo che gli elementi principali siano validi
    if(base == NULL || n <= 1 || size == 0 || cmp == NULL){
        return; 
    }

    // Iterazione per gli elementi
    for(size_t i = 0; i < n; i++){
        size_t j = i;

        // Controllo e swap degli elementi
        while(j > 0 && cmp(ELEMENT_ADDR(base, j, size), ELEMENT_ADDR(base, j-1, size)) < 0){
            upo_swap(ELEMENT_ADDR(base, j, size), ELEMENT_ADDR(base, j-1, size), size);
            j--;
        }
    }
}


void upo_merge_sort(void *base, size_t n, size_t size, upo_sort_comparator_t cmp)
{
    if(base == NULL || n <= 1 || size == 0 || cmp == NULL){
        return;
    }

    upo_merge_sort_rec(base, 0, n-1, size, cmp);
}

static void upo_merge_sort_rec(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp){

    if(lo >= hi){
        return;
    }

    // Ricerca della meta' del vettore (calcolo sicuro per evitare overflow)
    size_t mid = lo + (hi - lo) / 2;

    // Chiamate ricorsive per le parti del vettore 
    upo_merge_sort_rec(base, lo, mid, size, cmp);
    upo_merge_sort_rec(base, mid+1, hi, size, cmp);
    
    // Fusione delle due metà
    upo_merge_sort_merge(base, size, lo, mid, hi, cmp);
}

// Procedura di Merge
static void upo_merge_sort_merge(void *base, size_t size, size_t lo, size_t mid, size_t hi, upo_sort_comparator_t cmp){

    size_t i = 0;
    size_t j = mid + 1 - lo;
    size_t total_elements = hi - lo + 1;

    // Allocazione dell'array ausiliario
    void *aux = malloc(total_elements * size);
    // Controllo che l'array sia stato allocato correttamente
    if(aux == NULL){
        perror("Errore di allocazione");
        abort();
    }

    // Copia elementi nell'array ausiliario
    memcpy(aux, ELEMENT_ADDR(base, lo, size), total_elements * size);

    // Iterazione per ordinamento
    for(size_t k = lo; k <= hi; k++){
        if(i > (mid - lo)){
            memcpy(ELEMENT_ADDR(base, k, size), ELEMENT_ADDR(aux, j, size), size);
            j++;
        }else if(j > (hi - lo)){
            memcpy(ELEMENT_ADDR(base, k, size), ELEMENT_ADDR(aux, i, size), size);
            i++;
        }else if(cmp(ELEMENT_ADDR(aux, j, size), ELEMENT_ADDR(aux, i, size)) < 0){
            memcpy(ELEMENT_ADDR(base, k, size), ELEMENT_ADDR(aux, j, size), size);
            j++;
        }else{
            memcpy(ELEMENT_ADDR(base, k, size), ELEMENT_ADDR(aux, i, size), size);
            i++;
        }          
    }

    // Al termine della procedura bisogna eliminare lo spazio del vettore ausiliario
    free(aux);
}


void upo_quick_sort(void *base, size_t n, size_t size, upo_sort_comparator_t cmp)
{
    if(base == NULL || n <= 1 || size == 0 || cmp == NULL){
        return;
    }

    upo_quick_sort_rec(base, 0, n-1, size, cmp);
}

static size_t upo_quick_sort_partition(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp){

    size_t p = lo;
    size_t i = lo;
    size_t j = hi+1;

    while (1)
    {
        /* Avanza i fino a trovare un elemento >= pivot */
        do
        {
            i++;
        } while (i < hi && cmp(ELEMENT_ADDR(base, i, size), ELEMENT_ADDR(base, p, size)) < 0);

        /* Arretra j fino a trovare un elemento <= pivot */
        do
        {
            j--;
        } while (j > lo && cmp(ELEMENT_ADDR(base, j, size), ELEMENT_ADDR(base, p, size)) > 0);

        if (i >= j)
        {
            break;
        }

        upo_swap(ELEMENT_ADDR(base, i, size), ELEMENT_ADDR(base, j, size), size);
    }

    /* Posiziona il pivot nella sua collocazione corretta */
    upo_swap(ELEMENT_ADDR(base, p, size), ELEMENT_ADDR(base, j, size), size);

    return j;
}

static void upo_quick_sort_rec(void *base, size_t lo, size_t hi, size_t size, upo_sort_comparator_t cmp)
{
    if (lo >= hi)
    {
        return;
    }

    size_t j = upo_quick_sort_partition(base, lo, hi, size, cmp);

    // j > 0 previene underflow per interi unsigned (size_t)
    if (j > 0 && j - 1 >= lo)
    {
        upo_quick_sort_rec(base, lo, j - 1, size, cmp);
    }
    
    upo_quick_sort_rec(base, j + 1, hi, size, cmp);
}