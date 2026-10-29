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
#include "hashtable_private.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <upo/error.h>
#include <upo/utility.h>


/*** HASH TABLE with SEPARATE CHAINING ***/

// Creazione ht con separate chain
upo_ht_sepchain_t upo_ht_sepchain_create(size_t m, upo_ht_hasher_t key_hash, upo_ht_comparator_t key_cmp)
{
    upo_ht_sepchain_t ht = NULL;
    size_t i = 0;

    /* preconditions */
    assert( key_hash != NULL );
    assert( key_cmp != NULL );

    /* Allocate memory for the hash table type */
    ht = malloc(sizeof(struct upo_ht_sepchain_s));
    if (ht == NULL)
    {
        perror("Unable to allocate memory for Hash Table with Separate Chaining");
        abort();
    }

    if (m > 0)
    {
        /* Allocate memory for the array of slots */
        ht->slots = malloc(m*sizeof(upo_ht_sepchain_slot_t));
        if (ht->slots == NULL)
        {
            perror("Unable to allocate memory for slots of the Hash Table with Separate Chaining");
            abort();
        }

        /* Initialize slots */
        for (i = 0; i < m; ++i)
        {
            ht->slots[i].head = NULL;
        }
    }
    else
    {
        ht->slots = NULL;
    }

    /* Initialize the other fields */
    ht->capacity = m;
    ht->size = 0;
    ht->key_hash = key_hash;
    ht->key_cmp = key_cmp;

    return ht;
}

// Cancellazione ht con separate chain
void upo_ht_sepchain_destroy(upo_ht_sepchain_t ht, int destroy_data)
{
    if (ht != NULL)
    {
        upo_ht_sepchain_clear(ht, destroy_data);
        free(ht->slots);
        free(ht);
    }
}

// Svuotamento ht con separate chain
void upo_ht_sepchain_clear(upo_ht_sepchain_t ht, int destroy_data)
{
    if (ht != NULL && ht->slots != NULL)
    {
        size_t i = 0;

        /* For each slot, clear the associated list of collisions */
        for (i = 0; i < ht->capacity; ++i)
        {
            upo_ht_sepchain_list_node_t *list = NULL;

            list = ht->slots[i].head;
            while (list != NULL)
            {
                upo_ht_sepchain_list_node_t *node = list;

                list = list->next;

                if (destroy_data)
                {
                    free(node->key);
                    free(node->value);
                }

                free(node);
            }
            ht->slots[i].head = NULL;
        }
        ht->size = 0;
    }
}

// Inserimento valore valore nella ht (sostituzione vecchio valore)
void* upo_ht_sepchain_put(upo_ht_sepchain_t ht, void *key, void *value)
{
    void *old_value = NULL;

    // Variabile per il salvataggio della funzione di hashing
    size_t h;

    // Controllo su tabella e slot
    if(ht == NULL || ht->slots == NULL){

        return NULL;
    }

    // Si trova la cella da popolare tramite la funzione di hashing
    h = ht->key_hash(key, ht->capacity);

    // Creazione puntatore di appoggio per non apportare modifiche irreparabili
    upo_ht_sepchain_list_node_t *node = ht->slots[h].head;

    // Scorro la tabella finché non sono arrivato alla fine o finché la chiave che cerco non é presente nella tabella
    while(node != NULL && ht->key_cmp(key, node->key) != 0){

        node = node->next;
    }

    // Esce dal ciclo o perche' la tabella e' finita o perche' lo slot aveva la stessa chiave
    if(node != NULL){

        // Se e' diverso da value allora il nodo ha la stessa chiave
        old_value = node->value;
        node->value = value;

    }else{// Se non trova la chiave

        // Si crea un nuovo nodo perche' la chiave non e' nella tabella
        upo_ht_sepchain_list_node_t *new_node = malloc(sizeof(struct upo_ht_sepchain_list_node_s));

        // Controllo allocazione
        if(new_node == NULL){

            abort();
        }

        new_node->key = key;
        new_node->value = value;

        // Inserimento in testa del nuovo nodo
        new_node->next = ht->slots[h].head;
        ht->slots[h].head = new_node;
        ht->size += 1; // Incremento dimensione tabella

    }

    return old_value;
}

// Inserimento valore valore nella ht 
void upo_ht_sepchain_insert(upo_ht_sepchain_t ht, void *key, void *value)
{
    size_t h; //Variabile dove salveró il risultato della funzione di hashing (lo slot)

    if(ht == NULL || ht->slots == NULL){ //Controllo che la tabella e i suoi slot non siano vuoti
        return;
    }

    h = ht->key_hash(key, ht->capacity); //Trovo la cella dove inserire la chiave mediante la funzione di hashing
    upo_ht_sepchain_list_node_t *node = ht->slots[h].head; //Creo un puntatore di appoggio per non modificare irreparabilmente la testa della tabella
    while(node != NULL && ht->key_cmp(key, node->key) != 0){ //Scorro la tabella finché non sono arrivato alla fine o finché la chiave che cerco non é presente nella tabella
        node = node->next;
    }

    if(node == NULL){ // Se node é NULL sono arrivato fino alla fine della tabella e la chiave non c'era
        upo_ht_sepchain_list_node_t *newnode = malloc(sizeof(struct upo_ht_sepchain_list_node_s)); //Alloco dinamicamente un nuovo nodo da inserire nella tabella
        if(newnode == NULL){ //Controllo che l'allocazione sia andata a buon fine
            abort();
        }
        newnode->key = key; //Assegno la chiave da inserire al nodo che inseriró nella tabella
        newnode->value = value; //Assegno il valore da inserire al nodo che inseriró nella tabella
        //Faccio un inserimento in testa
        newnode->next = ht->slots[h].head;
        ht->slots[h].head = newnode;
        ht->size+=1; //Incremento di 1 la size
    }
}

// Restituzione valore associato ad una chiave nella tabella hash
void* upo_ht_sepchain_get(const upo_ht_sepchain_t ht, const void *key)
{
    size_t h;

    // Controllo su tabella e slot
    if(ht == NULL || ht->slots == NULL){

        return NULL;
    }

    //Ricerca cella da inserire
    h = ht->key_hash(key, ht->capacity);

    // Creazione puntatore di appoggio
    upo_ht_sepchain_list_node_t *node = ht->slots[h].head;

    // Scorre la tabella per cercare la chiave
    while(node != NULL && ht->key_cmp(key, node->key) != 0){

        node = node->next;
    }

    if(node == NULL){

        // Non trovo il valore
        return NULL;

    }else{

        // Trovo il valore e lo ritorno
        return node->value;
    }
}

// Ricerca di un determinato valore all'interno della ht (se presente o meno)
int upo_ht_sepchain_contains(const upo_ht_sepchain_t ht, const void *key)
{
    if(ht == NULL || ht->slots == NULL){ //Controllo che la tabella e i suoi slot non siano vuoti
        return 0;
    }

    //Utilizzo la funzione get che ha lo stesso funzionamento per cercare se la chiave é presente nella tabella
    void* value = upo_ht_sepchain_get(ht, key); //Salvo il valore ritornato

    if(value != NULL){ //Se il valore é diverso da NULL la chiave é presente nella tabella
        return 1; //Ritorno 1
        
    } else { //Sennó la chiave non era presente nella tabella 
        return 0;
    }
}

// Eliminazione dalla ht della coppia associata ad una determinata chiave
void upo_ht_sepchain_delete(upo_ht_sepchain_t ht, const void *key, int destroy_data)
{
    // Variabile di appoggio
    size_t h;

    // Controllo su tabella e su slot
    if(ht == NULL || ht->slots == NULL){

        return;
    }

    // Funzione di hash per inserire il valore nella tabella
    h = ht->key_hash(key, ht->capacity);

    // Dichiarazione nodi ausiliari per la cancellazione
    upo_ht_sepchain_list_node_t *current = ht->slots[h].head;
    upo_ht_sepchain_list_node_t *prev = NULL;

    // Ciclo per iterare sulla tabella
    while(current != NULL && ht->key_cmp(key, current->key)){
        
        // Salvo il nodo prima dell'aggiornamento per non perdere il riferimento
        prev = current;
        // Avanzo il nodo corrente a quello successivo
        current  = current->next;
    }

    // Controllo che la chiave sia stata trovata
    if(current != NULL){
        
        // Se prev == NULL allora il valore da rimuovere e' la testa
        if(prev == NULL){

            ht->slots[h].head = current->next;

        }else{

            prev->next = current->next;
        }

        if(destroy_data){

            free((void*)current->value);
            free(current->key);
        }

        free(current);
        ht->size -= 1;
    }    
}

// Calcolo dimensione sepchain
size_t upo_ht_sepchain_size(const upo_ht_sepchain_t ht)
{
    // Controllo che la tabella non sia vuota
    if(ht == NULL){

        return 0;
    
    }else{

        return ht->size;
    }
}

// Controllo ht se vuota
int upo_ht_sepchain_is_empty(const upo_ht_sepchain_t ht)
{
    return upo_ht_sepchain_size(ht) == 0 ? 1 : 0;
}

// Calcolo capacita' ht
size_t upo_ht_sepchain_capacity(const upo_ht_sepchain_t ht)
{
    return (ht != NULL) ? ht->capacity : 0;
}

// Calcolo fattore di carico (dimensione / capacita')
double upo_ht_sepchain_load_factor(const upo_ht_sepchain_t ht)
{
    return upo_ht_sepchain_size(ht) / (double) upo_ht_sepchain_capacity(ht);
}

upo_ht_comparator_t upo_ht_sepchain_get_comparator(const upo_ht_sepchain_t ht)
{
    return ht->key_cmp;
}

upo_ht_hasher_t upo_ht_sepchain_get_hasher(const upo_ht_sepchain_t ht)
{
    return ht->key_hash;
}

/*** HASH TABLE with LINEAR PROBING ***/

// Creazione ht con linprob
upo_ht_linprob_t upo_ht_linprob_create(size_t m, upo_ht_hasher_t key_hash, upo_ht_comparator_t key_cmp)
{
    upo_ht_linprob_t ht = NULL;
    size_t i = 0;

    /* preconditions */
    assert( key_hash != NULL );
    assert( key_cmp != NULL );

    /* Allocate memory for the hash table type */
    ht = malloc(sizeof(struct upo_ht_linprob_s));
    if (ht == NULL)
    {
        perror("Unable to allocate memory for Hash Table with Linear Probing");
        abort();
    }

    /* Allocate memory for the array of slots */
    if (m > 0)
    {
        ht->slots = malloc(m*sizeof(upo_ht_linprob_slot_t));
        if (ht->slots == NULL)
        {
            perror("Unable to allocate memory for slots of the Hash Table with Separate Chaining");
            abort();
        }

        /* Initialize the slots */
        for (i = 0; i < m; ++i)
        {
            ht->slots[i].key = NULL;
            ht->slots[i].value = NULL;
            ht->slots[i].tombstone = 0;
        }
    }
    else
    {
        ht->slots = NULL;
    }

    /* Initialize the other fields */
    ht->capacity = m;
    ht->size = 0;
    ht->key_hash = key_hash;
    ht->key_cmp = key_cmp;

    return ht;
}

// Cancellazione totale della ht con linprob
void upo_ht_linprob_destroy(upo_ht_linprob_t ht, int destroy_data)
{
    if (ht != NULL)
    {
        upo_ht_linprob_clear(ht, destroy_data);
        free(ht->slots);
        free(ht);
    }
}

// Svota ht con linprob
void upo_ht_linprob_clear(upo_ht_linprob_t ht, int destroy_data)
{
    if (ht != NULL && ht->slots != NULL)
    {
        size_t i = 0;

        /* For each slot, clear the associated list of collisions */
        for (i = 0; i < ht->capacity; ++i)
        {
            if (ht->slots[i].key != NULL)
            {
                if (destroy_data)
                {
                    free(ht->slots[i].key);
                    free(ht->slots[i].value);
                }
                ht->slots[i].key = NULL;
                ht->slots[i].value = NULL;
                ht->slots[i].tombstone = 0;
            }
        }
        ht->size = 0;
    }
}

// Inserimento valore nella ht con linprob (sostituzione valore)
void* upo_ht_linprob_put(upo_ht_linprob_t ht, void *key, void *value)
{
    void *old_value = NULL;

    // Controllo sulla tabella e sugli slot
    if(ht == NULL || ht->slots == NULL){

        return NULL;
    }

    // Se il fattore di carico e' >= 0.5 bisogna ricalibrare la dimensione
    if(upo_ht_linprob_load_factor(ht) >= 0.5){
        upo_ht_linprob_resize(ht, 2 * ht->capacity);
    }

    size_t h;

    h = ht->key_hash(key, ht->capacity);

    // Da usare se si incontra una tombstone
    int found_tombstone = 0;
    // Variabile per salvare l'indice della tombstone
    size_t h_tomb;

    //Ciclo nel quale itero finché non arrivo alla fine e la chiave non corrisponde a quello che cerco o se lo slot corrente é un tombstone
    while((ht->slots[h].key != NULL && ht->key_cmp(key, ht->slots[h].key) != 0) || ht->slots[h].tombstone != 0){

        if(ht->slots[h].tombstone && !found_tombstone){

            // Salvataggio indice tombstone
            h_tomb = h;
            // Modifica del flag per notificare la presenza di una tombstone
            found_tombstone = 1;
        }

        h = (h+1) % ht->capacity;
    }

    // Alla fine del ciclo
    if(ht->slots[h].key != NULL){

        old_value = ht->slots[h].value;
        ht->slots[h].value = value;

    }else if(ht->slots[h].key == NULL){

        if(found_tombstone){

            h = h_tomb;
            // Flag resettato
            found_tombstone = 0;
            // Resetto tombstone perche' e' stato appena trovato
            ht->slots[h].tombstone = 0;
        }

        ht->slots[h].value = value;
        ht->slots[h].key = key;
        ht->size += 1;
    }

    return old_value;
}

// Inserimento valore nella ht con linprob 
void upo_ht_linprob_insert(upo_ht_linprob_t ht, void *key, void *value)
{
    // Controllo su tabella e slots
    if(ht == NULL || ht->slots == NULL){

        return;
    }

    if(upo_ht_linprob_load_factor(ht) >= 0.5){
        upo_ht_linprob_resize(ht, 2 * ht->capacity);
    }

    size_t h;

    h = ht->key_hash(key, ht->capacity);

    int found_tombstone = 0;
    size_t h_tomb;

    while((ht->slots[h].key != NULL && ht->key_cmp(key, ht->slots[h].key) != 0) || ht->slots[h].tombstone != 0){
        if(!found_tombstone && ht->slots[h].tombstone){ //Se abbiamo trovato una tombstone inseriremo lí il valore
            h_tomb = h; //Mi salvo l'indice del tombstone
            found_tombstone = 1; //Imposto a 1 la variabile booleana 
        }

        h = (h + 1) % ht->capacity; //Aggiorno l'indice al valore successivo
    }

    if(ht->slots[h].key == NULL){ //Se key é uguale a NULL siamo arrivati infondo alla tabella
        if(found_tombstone){ //Controllo se é mai stata trovata una tombstone
            h = h_tomb; //Salvo l'indice della tombstone
            found_tombstone = 0; //Resetto la variabile booleana
            ht->slots[h].tombstone = 0; //Resetto il campo tombstone
        }
        //Aggiorno i campi value e key
        ht->slots[h].key = key;
        ht->slots[h].value = value;
        ht->size+=1;
    }
}

// Restituisce un determinato valore della ht associato ad una chiave
void* upo_ht_linprob_get(const upo_ht_linprob_t ht, const void *key)
{
    if(ht == NULL || ht->slots == NULL){
        return NULL;
    }

    size_t h; //Variabile dove vado a salvare il risultato dell'operazione di hashing (lo slot da dove iniziare la ricerca)
    h = ht->key_hash(key, ht->capacity); //Vado a fare una operazione di hash per trovare l'indice dove cercare la chiave


    while(ht->slots[h].key != NULL && ht->key_cmp(key, ht->slots[h].key) != 0 || ht->slots[h].tombstone != 0){
        h = (h + 1) % ht->capacity; //Avanzo l'indice
    }

    if(ht->slots[h].key != NULL){ // Se la chiave non é NULL vuol dire che é la chiave che sto cercando
        return ht->slots[h].value; //Ritorno il suo valore
    } else {
        return NULL;
    }
}

// Restituisce True(1) o False(0) se il valore è presente nella ht
int upo_ht_linprob_contains(const upo_ht_linprob_t ht, const void *key)
{
   if(ht == NULL || ht->slots == NULL){
        return 0;
    }

    void *value = upo_ht_linprob_get(ht, key);; //Variabile helper che mi servirá per riutilizzare la get che ha la stessa logica

    return (value != NULL) ? 1 : 0; //Se value ha un valore la chiave é contenuta nella tabella e allora ritorno 1 sennó 0
}

// Cancellazione di un valore dalla ht 
void upo_ht_linprob_delete(upo_ht_linprob_t ht, const void *key, int destroy_data)
{
       if(ht == NULL || ht->slots == NULL){
        return;
    }

    size_t h; //Variabile dove vado a salvare il risultato dell'operazione di hashing (lo slot da dove iniziare la ricerca)
    h = ht->key_hash(key, ht->capacity); //Vado a fare una operazione di hash per trovare l'indice dove cercare la chiave

    //Ciclo nel quale itero finché non arrivo alla fine e la chiave non corrisponde a quello che cerco o se lo slot corrente é un tombstone
    while((ht->slots[h].key != NULL && ht->key_cmp(key, ht->slots[h].key) != 0) || ht->slots[h].tombstone != 0){
        h = (h + 1) % ht->capacity; //Avanzo l'indice
    }

    if(ht->slots[h].key != NULL){ //Controllo che la chiave sia stata trovata
        if(destroy_data){ //Se destroy_data é a 1 libero sia value che key dalla memoria
            free((void*)ht->slots[h].value);
            free(ht->slots[h].key);
        }

        //Faccio puntare a NULL chiave e valore poiché quel nodo verrá scollegato
        ht->slots[h].key = NULL;
        ht->slots[h].value = NULL;
        ht->slots[h].tombstone = 1; //Imposto il campo tombstone a 1
        ht->size-=1; //Diminuisco di 1 la size della tabella

        if(upo_ht_linprob_load_factor(ht) <= 0.125){ //Controllo del fattore di carico
            upo_ht_linprob_resize(ht, ht->capacity / 2); //Dimezzo la capacitá della tabella
        }
    }
}

// Calcolo size della ht
size_t upo_ht_linprob_size(const upo_ht_linprob_t ht)
{
    if(ht == NULL){ //Se la tabella é vuota ritorno 0
        return 0;
    } else {
        return ht->size; //Sennó ritorno la sua size
    }
}

// Controllo se ht e' vuota
int upo_ht_linprob_is_empty(const upo_ht_linprob_t ht)
{
    return upo_ht_linprob_size(ht) == 0 ? 1 : 0;
}

// Calcolo della capacita' della ht
size_t upo_ht_linprob_capacity(const upo_ht_linprob_t ht)
{
    return (ht != NULL) ? ht->capacity : 0;
}

// Calcolo fattore di carico ht con linprob (dim / capacita')
double upo_ht_linprob_load_factor(const upo_ht_linprob_t ht)
{
    return upo_ht_linprob_size(ht) / (double) upo_ht_linprob_capacity(ht);
}

// Cambiamento capacità della tabella hash che usa il linear probing, ricostruendola con un nuovo numero di slots
void upo_ht_linprob_resize(upo_ht_linprob_t ht, size_t n)
{
    /* preconditions */
    assert( n > 0 );

    if (ht != NULL)
    {
        /* The hash table must be rebuilt from scratch since the hash value of
         * keys will be in general different (due to the change in the
         * capacity). */

        size_t i = 0;
        upo_ht_linprob_t new_ht = NULL;

        /* Create a new temporary hash table */
        new_ht = upo_ht_linprob_create(n, ht->key_hash, ht->key_cmp);
        if (new_ht == NULL)
        {
            perror("Unable to allocate memory for slots of the Hash Table with Separate Chaining");
            abort();
        }

        /* Put in the temporary hash table the key-value pairs stored in the
         * hash table to resize.
         * Note: by calling function 'put' we are also rehashing the keys
         * according to the new capacity. */
        for (i = 0; i < ht->capacity; ++i)
        {
            if (ht->slots[i].key != NULL)
            {
                upo_ht_linprob_put(new_ht, ht->slots[i].key, ht->slots[i].value);
            }
        }

        /* Copy the new slots in the old hash table.
         * To do so we use a trick that avoids to loop for each key-value pair:
         * swap the array of slots, the size and the capacity between new and
         * old hash tables.
         * This way the memory that was allocated for ht is moved to new_ht and
         * is deallocated when new_ht is destroyed. */
        upo_swap(&ht->slots, &new_ht->slots, sizeof ht->slots);
        upo_swap(&ht->capacity, &new_ht->capacity, sizeof ht->capacity);
        upo_swap(&ht->size, &new_ht->size, sizeof ht->size);

        /* Destroy temporary hash table */
        upo_ht_linprob_destroy(new_ht, 0);
    }
}

/*** HASH TABLE - EXTRA OPERATIONS ***/

// Creazione lista concatenata contenente tutte le chiavi presenti nella tabella hash con separate chaining
upo_ht_key_list_t upo_ht_sepchain_keys(const upo_ht_sepchain_t ht)
{
    if(ht == NULL || ht->size == 0 || ht->slots == NULL){
        return NULL;
    }

    upo_ht_key_list_node_t *list_head = NULL; //Alloco dinamicamente una lista dove andró a salvare tutte le chiavi della tabella
    
    for(size_t i = 0; i<ht->capacity; i++){ //Ciclo esterno che scorre verticalmente la tabella
       for(upo_ht_sepchain_list_node_t *head = ht->slots[i].head; head!= NULL; head = head->next){ //Ciclo interno dove scorro verticalmente la lista con un puntatore alla testa dello slot corrente
            upo_ht_key_list_node_t *node = malloc(sizeof(struct upo_ht_key_list_node_s)); //Creo un nodo che andrá inserito nella lista
            if(node == NULL){
                perror("Errore nella allocazione del nodo");
                abort();
            }
            node->key = head->key; //Salvo la chiave dalla testa e la metto nel nodo da inserire nella lista

            //Inserisco in testa il nuovo valore nella lista
            node->next = list_head;
            list_head = node;
       }
    }

    return list_head; //Ritorno la testa della lista
}

// Visita in ordine della ht (sepchain)
void upo_ht_sepchain_traverse(const upo_ht_sepchain_t ht, upo_ht_visitor_t visit, void *visit_context)
{
    if(visit == NULL){ //Controllo che la funzione di visita non sia nulla
        perror("Richiamata funzione di visita nulla");
        return;
    }

    if(ht == NULL || ht->size == 0 || ht->slots == NULL){
        return;
    }

    //Ciclo esterno di visita dove visiteremo verticalmente la tabella
    for(size_t i = 0; i < ht->capacity ; i++){
        upo_ht_sepchain_list_node_t *head = ht->slots[i].head; //Creo un puntatore alla testa dello slot corrente
        for(; head!=NULL; head = head->next){ //Ciclo interno di visita dove visiteremo orizzontalmente la tabella
            visit(head->key, head->value, visit_context); //Chiamo la funzione di visita
        }
    }
}

// Creazione lista concatenata contenente tutte le chiavi presenti nella tabella hash con linear probing
upo_ht_key_list_t upo_ht_linprob_keys(const upo_ht_linprob_t ht)
{
    if(ht == NULL || ht->slots == NULL || ht->size == 0){
        return NULL;
    }

    upo_ht_key_list_node_t *list_head = NULL; //Creo la lista che conterrá tutte le chiavi

    //Scansione lineare della tabella per trovare tutte le chiavi
    for(size_t i = 0; i < ht->capacity; i++){
        if(ht->slots[i].key != NULL){ //Controllo che la chiave non sia vuota né una tombstone nello slot corrente
            upo_ht_key_list_node_t *newnode = malloc(sizeof(struct upo_ht_key_list_node_s)); //Alloco dinamicamente memoria per un nuovo nodo da inserire nella lista
            if(newnode == NULL){ //Controllo che la allocazione sia andata a buon fine
                perror("Errore nella allocazione del nodo");
                abort();
            }
            newnode->key = ht->slots[i].key; //Salvo nel nuovo nodo il valore della chiave dello slot corrente
            //Inserisco in testa il nuovo nodo
            newnode->next = list_head;
            list_head = newnode;
        }
    }

    return list_head; //Ritorno la lista contententi tutte le chiavi
}

// Visita in ordine della ht (linprob)
void upo_ht_linprob_traverse(const upo_ht_linprob_t ht, upo_ht_visitor_t visit, void *visit_context)
{
    if(visit == NULL){ //Controllo che la funzione di visita non sia nulla
        perror("Richiamata funzione di visita nulla");
        return;
    }

    if(ht == NULL || ht->slots == NULL || ht->size == 0){
        return;
    }

    for(size_t i = 0; i < ht->capacity; i++){
        if(ht->slots[i].key != NULL){ //Controllo che la chiave non sia vuota né una tombstone nello slot corrente
            visit(ht->slots[i].key, ht->slots[i].value, visit_context); //Richiamo la funzione di visita
        }
    }    
}

/*** HASH FUNCTIONS ***/
size_t upo_ht_hash_int_div(const void *x, size_t m)
{
    /* preconditions */
    assert( x != NULL );
    assert( m > 0 );

    return *((int*) x) % m;
}

size_t upo_ht_hash_int_mult(const void *x, double a, size_t m)
{
    /* preconditions */
    assert( x != NULL );
    assert( a > 0 && a < 1 );
    assert( m > 0 );

    return floor( m * fmod(a * *((int*) x), 1.0) );
}

size_t upo_ht_hash_int_mult_knuth(const void *x, size_t m)
{
    return upo_ht_hash_int_mult(x, 0.5*(sqrt(5)-1), m);
}

size_t upo_ht_hash_str(const void *x, size_t h0, size_t a, size_t m)
{
    const char *s = NULL;
    size_t h = h0; 

    /* preconditions */
    assert( x != NULL );
    assert( m > 0 );
/*
    assert( a < m );
*/
    assert( h0 < m );

    s = *((const char**) x);
    for (; *s; ++s)
    {
        h = (a*h + *s) % m;
    }

    return h;
}

size_t upo_ht_hash_str_djb2(const void *x, size_t m)
{
    return upo_ht_hash_str(x, 5381U, 33U, m);
}

size_t upo_ht_hash_str_djb2a(const void *x, size_t m)
{
    const char *s = NULL;
    size_t h = 5381U; 

    /* preconditions */
    assert( x != NULL );
    assert( m > 0 );

    s = *((const char**) x);
    for (; *s; ++s)
    {
        h = (33U*h ^ *s) % m;
    }

    return h;
}

size_t upo_ht_hash_str_java(const void *x, size_t m)
{
    return upo_ht_hash_str(x, 0U, 31U, m);
}

size_t upo_ht_hash_str_kr2e(const void *x, size_t m)
{
    return upo_ht_hash_str(x, 0U, 31U, m);
}

size_t upo_ht_hash_str_sgistl(const void *x, size_t m)
{
    return upo_ht_hash_str(x, 0U, 5U, m);
}

size_t upo_ht_hash_str_stlport(const void *x, size_t m)
{
    return upo_ht_hash_str(x, 0U, 33U, m);
}