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

#include "bst_private.h"
#include <stdio.h>
#include <stdlib.h>


/**** FUNDAMENTAL OPERATIONS ****/

// Creazione di un albero BST 
upo_bst_t upo_bst_create(upo_bst_comparator_t key_cmp)
{
    // Allocazione della struttura dell'albero
    upo_bst_t tree = malloc(sizeof(struct upo_bst_s));
    if (tree == NULL)
    {
        perror("Unable to create a binary search tree");
        abort();
    }

    // Impostazione a NULL della radice
    tree->root = NULL;
    // Comparatore
    tree->key_cmp = key_cmp;

    return tree;
}

// Cancellazione dell'albero
void upo_bst_destroy(upo_bst_t tree, int destroy_data)
{
    if (tree != NULL)
    {
        upo_bst_clear(tree, destroy_data);
        free(tree);
    }
}

// Implementazione cancellazione albero
void upo_bst_clear_impl(upo_bst_node_t *node, int destroy_data)
{
    // Controllo che non ci siano nodi NULL
    if (node != NULL)
    {
        // Clear richiamata ricorsivamente sui nodi sx e dx (figli del nodo controllato)
        upo_bst_clear_impl(node->left, destroy_data);
        upo_bst_clear_impl(node->right, destroy_data);

        // Se la destroy ha successo
        if (destroy_data)
        {
            // Si elimina il nodo con la key e il value
            free(node->key);
            free(node->value);
        }

        // Free del nodo dalla struttura dell'albero
        free(node);
    }
}

// Svuotamento dell'albero
void upo_bst_clear(upo_bst_t tree, int destroy_data)
{
    if (tree != NULL)
    {
        upo_bst_clear_impl(tree->root, destroy_data);
        tree->root = NULL;
    }
}

// Implementazione inserimento nodo (sostituisce il valore vecchio di un nodo)
void* upo_bst_put_impl(upo_bst_node_t *node, void *key, void *value, upo_bst_comparator_t key_cmp, void **old_value){

    // Comparatore per i vari confronti
    int cmp;

    // Controllo che il nodo sia vuoto
    if(node == NULL){

        // Creazione nuovo nodo da inserire
        upo_bst_node_t *new_node = malloc(sizeof(struct upo_bst_node_s));

        // Controllo che l'allocazione sia andata a buon fine
        if(new_node == NULL){

            perror("Allocazione nuovo nodo fallita");
            abort();
        }

        // Assegnazione valori al nuovo nodo
        new_node->key = key;
        new_node->value = value;
        new_node->left = NULL;
        new_node->right = NULL;

        return new_node;

    }else{

        // Comparatore usato per sapere dove dover inserire il nodo appena creato
        cmp = key_cmp(key, node->key);
        
        // Controllo se la chiave da cercare è più piccola di quella attuale
        if(cmp < 0){

            // Se lo e', continuo l'operazione nel sottoalbero sinistro
            node->left = upo_bst_put_impl(node->left, key, value, key_cmp, old_value);

        }else if(cmp > 0){

            // Continuo l'operazione nel sottoalbero destro
            node->right = upo_bst_put_impl(node->right, key, value, key_cmp, old_value);

        }else{

            // Se la chiave da cercare è uguale a quella corrente
            if(old_value != NULL){

                // Si aggiorna il valore vecchio con quello nuovo
                *old_value = node->value;

                // Aggiorno il nuovo valore
                node->value = value;
            }
        }
    } 

    return node;
}

// Inserimento nuovo valore di nodo
void* upo_bst_put(upo_bst_t tree, void *key, void *value)
{
   // Controllo che l'albero non sia vuoto
   if(tree == NULL){

    return NULL;
   }

   // Puntatore al valore dei nodi vecchi
   void *old_value = NULL;
   // Si richiama la funzione di inserimento sulla radice dell'albero
   tree->root = upo_bst_put_impl(tree->root, key, value, tree->key_cmp, &old_value);
   return old_value;
}

// Implementazione inserimento nodo (inserisce il valore senza sostiturlo)
static upo_bst_node_t *upo_bst_insert_impl(upo_bst_node_t *node, void *key, void *value, upo_bst_comparator_t key_cmp){

    int cmp;

    // Controllo che il nodo da inserire non sia nullo
    if(node == NULL){

        // Allocazione di spazio per il nuovo nodo
        upo_bst_node_t *new_node = malloc(sizeof(struct upo_bst_node_s));

        // Controllo che l'allocazione sia andata a buon fine
        if(new_node == NULL){

            perror("Allocazione fallita");
            abort();
        }

        // Assegnazione valori al nuovo nodo
        new_node->value = value;
        new_node->key = key;
        new_node->left = NULL;
        new_node->right = NULL;

        return new_node;

    }else{

        cmp = key_cmp(key, node->key);

        // Controllo se la chiave da cercare e' piu' piccola di quella da cercare
        if(cmp < 0){
            
            node->left = upo_bst_insert_impl(node->left, key, value, key_cmp);

        // Controllo se la chiave da cercare e' piu' grande di quella da cercare
        }else if(cmp > 0){

            node->right = upo_bst_insert_impl(node->right, key, value, key_cmp);

        }else{

            return node;
        }
    }

    return node;
}

// Inserimento nuovo nodo
void upo_bst_insert(upo_bst_t tree, void *key, void *value)
{
   // Controllo che l'albero non sia vuoto
   if(tree == NULL){

    exit(1);
   }
   // Richiamo funzione helper
   tree->root = upo_bst_insert_impl(tree->root, key, value, tree->key_cmp);
}

// Implementazione restituzione valore di un nodo (se presente) dell'albero (associato alla chiave)
static void *upo_bst_get_impl(const upo_bst_node_t *node, const void *key, upo_bst_comparator_t key_cmp){

    // Comparatore per la ricerca
    int cmp;

    // Controllo che il nodo non sia vuoto
    if(node == NULL){

        return NULL;

    }else{

        // Salvataggio risultato della cmp tra chiave e nodo
        cmp = key_cmp(key, node->key);

        // Se la chiave e' uguale a quella del nodo corrente
        if(cmp == 0){

            // Si ritorna il nodo corrente
            return node->value;
        
        }else if(cmp < 0){

            // Ci si sposta nel sottoalbero sinistro
            return upo_bst_get_impl(node->left, key, key_cmp);
        
        }else{ // if(cmp > 0)

            // Ci si sposta nel sottoalbero destro
            return upo_bst_get_impl(node->right, key, key_cmp);
        }
    }
}

// Restituzione valore di un nodo (se presente) associato alla chiave 
void* upo_bst_get(const upo_bst_t tree, const void *key)
{
    // Controllo che l'albero non sia vuoto
    if(tree == NULL){

       return NULL;

    }else{

        return upo_bst_get_impl(tree->root, key, tree->key_cmp);
    }
}

// Funzione che restituisce True(1) o False(0) a seconda se un nodo e' presente o meno nell'albero
int upo_bst_contains(const upo_bst_t tree, const void *key)
{
    // Controllo che l'albero non sia vuoto
    if(tree == NULL){

        // Ritorno 0 perche' non ha nodi
        return 0;
    
    }else{

        void *ris  = upo_bst_get(tree, key);

        // Se il valore di ritorno é diverso da null allora un nodo nell'albero contiene la chiave
        if(ris != NULL){

            return 1;
        
        }else{

            return 0;
        }
    }
}

// Ricerca del nodo minimo in un albero (visita sempre il sottoalbero sinistro)
static upo_bst_node_t *upo_bst_node_min(upo_bst_node_t *node) {

    if (node == NULL){

        return NULL;
    } 

    while (node->left != NULL){

        node = node->left;
    }

    return node;
}

// Implementazione cancellazione nodo dall'albero
static upo_bst_node_t *upo_bst_delete_impl(upo_bst_node_t *node, const void *key, upo_bst_comparator_t key_cmp, int destroy_data){

    // Controllo che il nodo non sia vuoto
    if(node == NULL){

        return NULL;
    }

    // Comparatore per i due nodi
    int cmp = key_cmp(key, node->key);

    // Se la key e' presente nel nodo corrente
    if(cmp == 0){

        // Controllo che abbia solo figli destri
        if(node->left == NULL){

            // Salvo il contenuto del figlio in una variabile tmp
            upo_bst_node_t *tmp = node->right;

            // Se destroy_data == 1
            if(destroy_data){

                // Deallocazione key e value
                free((void*)node->key);
                free(node->value);
            }
            // Deallocazione del nodo corrente
            free(node);
            return tmp;
        
        // Se ha solo figli sinistri
        }else if(node->right == NULL){

            // Salvo il contenuto del figlio in una variabile tmp
            upo_bst_node_t *tmp = node->right;

            // Se destroy_data == 1
            if(destroy_data){

                // Deallocazione key e value
                free((void*)node->key);
                free(node->value);
            }
            // Deallocazione del nodo corrente
            free(node);
            return tmp;
        
        }else{// Se ha entrambi i figli

            // Trovo il valore minimo -> SUCCESSORE
            upo_bst_node_t *succ = upo_bst_node_min(node->right);
            if(destroy_data){

                // Deallocazione di key e value
                free((void*)node->value);
                free(node->value);
            }
            //Sovrascrivo i campi del successore nel nodo corrente
            node->key = succ->key;
            node->value = succ->value;
            node->right = upo_bst_delete_impl(node->right, succ->key, key_cmp, 0); //Rimuovo il successore dal sottoalbero destro, passo 0 per non distruggere i campi appena trasferiti
            return node; // Ritorno il nodo aggiornato
        }
    }else if(cmp < 0){

        // Continua l'operazione nel sottoalbero sinitro
        node->left = upo_bst_delete_impl(node->left, key, key_cmp, destroy_data);
        return node;
    
    }else{

        // Continua l'operazione nel sottoalbero destro
        node->right = upo_bst_delete_impl(node->right, key, key_cmp, destroy_data);

        // (Nuovo nodo da collegare alla radice)
        return node;
    }
}

// Cancellazione nodo dall'albero (Elimina ricorsivamente il nodo associato ad una chiave)
void upo_bst_delete(upo_bst_t tree, const void *key, int destroy_data)
{
    // Controllo che la radice non sia vuota
    if(tree == NULL){
        return;
    }

    tree->root = upo_bst_delete_impl(tree->root, key, tree->key_cmp, destroy_data);
}

// Implemtentazione calcolo nodi complessivi dell'albero
static size_t upo_bst_size_impl(const upo_bst_node_t *node){

    if(node == NULL){
        
        return 0;
    }

    size_t sizeLeft = upo_bst_size_impl(node->left);

    size_t sizeRight = upo_bst_size_impl(node->right);

    return 1 + sizeLeft + sizeRight;
}

// Calcolo numero di nodi che compongono l'albero
size_t upo_bst_size(const upo_bst_t tree)
{
    // Controllo che l'albero non sia vuoto
    if(tree == NULL){

        return 0;
    }

    // Chiamata ricorsiva 
    return upo_bst_size_impl(tree->root);
}

// Implemntazione calcolo dell'altezza di un albero
static size_t upo_bst_height_impl(const upo_bst_node_t *node){

    // Controllo se un nodo e' NULLO o foglia
    if(node == NULL || (node->left == NULL && node->right == NULL)){

        return 0;
    }

    // Altrimenti, si controllano i due sottoalberi (sx e dx)
    size_t sizeLeft = upo_bst_height_impl(node->left);
    size_t sizeRight = upo_bst_height_impl(node->right);

    // Bisogna controllare anche se e' presente un ramo più lungo
    if(sizeLeft > sizeRight){

        // Root + altezza sottoalbero sinistro
        return 1 + sizeLeft;

    }else{

        // Root + altezza sottoalbero destro
        return 1 + sizeRight;
    }
}

// Calcolo altezza di un albero
size_t upo_bst_height(const upo_bst_t tree)
{
    // Controllo che l'albero non sia vuoto
    if(tree == NULL) {

        return 0;
    }

    return upo_bst_height_impl(tree->root);
}

// Implementazione della visita in ordine dei nodi dell'albero
static void upo_bst_traverse_in_order_impl(upo_bst_node_t *node, upo_bst_visitor_t visit, void *visit_context){

    // Se il nodo non e' nullo
    if(node != NULL){

        // Ricorsione sul nodo sinistro
        upo_bst_traverse_in_order_impl(node->left, visit, visit_context);

        // Si visita il nodo
        visit(node->key, node->value, visit_context);

        // Ricorsione sul nodo destro
        upo_bst_traverse_in_order_impl(node->right, visit, visit_context);
    }
}

// Visita in ordine dell'albero
void upo_bst_traverse_in_order(const upo_bst_t tree, upo_bst_visitor_t visit, void *visit_context)
{
   // Controllo che l'albero non sia vuoto
   if(tree == NULL || visit == NULL){

    return;
   }

   // Chiamata ricorsiva
   upo_bst_traverse_in_order_impl(tree->root, visit, visit_context);
}

// Controllo se l'albero e' vuoto (True(1) se e' vuoto, False(0) se NON e' vuoto)
int upo_bst_is_empty(const upo_bst_t tree)
{
    // Controllo sulla radice dell'albero
    if(tree == NULL || tree->root == NULL){

        return 1;

    }else{

        return 0;
    }
}

/**** END of FUNDAMENTAL OPERATIONS ****/


/**** EXTRA OPERATIONS ****/

// Restituzione del ptr alla chiave piu' piccola dell'albero
void* upo_bst_min(const upo_bst_t tree) {
    if (upo_bst_is_empty(tree)) {
        return NULL;
    }

    const upo_bst_node_t *min = upo_bst_node_min(tree->root);

    return (min != NULL) ? min->key : NULL;
}

// Ricerca del nodo con valore massimo nell'albero
static const upo_bst_node_t *upo_bst_max_impl(const upo_bst_node_t *node) {
    if (node == NULL) {
        return NULL;
    }

    // Se non c'è un sottoalbero destro, il nodo corrente è il massimo
    if (node->right == NULL) {

        return node; 
    }

    return upo_bst_max_impl(node->right);
}

// Restituzione del ptr alla chiave piu' grande dell'albero
void* upo_bst_max(const upo_bst_t tree) {
    if (upo_bst_is_empty(tree)) {
        return NULL;
    }

    const upo_bst_node_t *max = upo_bst_max_impl(tree->root);

    return (max != NULL) ? max->key : NULL;
}

// Implementazione della cancellazione della chiave minima dell'albero
static upo_bst_node_t * upo_bst_delete_min_impl(upo_bst_node_t *node, int destroy_data){

    // Controllo del nodo
    if(node == NULL){

        return NULL;
    }

    // Controllo sul sottoalbero sinistro 
    if(node->left == NULL){

        // Salvataggio dati in un puntatore tmp
        upo_bst_node_t *tmp = node->right;

        if(destroy_data){

            free((void*)node->key);
            free(node->value);
        }

        free(node);
        return tmp;
    }

    // Se non sono ancora all'ultimo nodo devo continuare a scendere ricorsivamente
    node->left = upo_bst_delete_min_impl(node->left, destroy_data);

    return node;
}

// Cancellazione chiave piu' piccola dell'albero
void upo_bst_delete_min(upo_bst_t tree, int destroy_data)
{
    if(upo_bst_is_empty(tree)){

        return;
    }

    // Chiamata sulla radice dell'albero
    tree->root = upo_bst_delete_min_impl(tree->root, destroy_data);
}

// Implementazione della cancellazione della chiave massima dell'albero
static upo_bst_node_t *upo_bst_delete_max_impl(upo_bst_node_t *node, int destroy_data){

    // Se il nodo e' nullo, non c'e' nulla da cancellare
    if(node == NULL){

        return NULL;
    }

    // In un BST, il massimo e' nel sottoalbero destro
    if(node->right == NULL){

        upo_bst_node_t *tmp = node->left;
        if(destroy_data){

            free((void*)node->key);
            free(node->value);
        }

        free(node);
        return tmp;
    }

    // Se non sono ancora all'ultimo nodo devo continuare a scendere ricorsivamente
    node->right = upo_bst_delete_max_impl(node->right, destroy_data);

    return node;

}

// Cancellazione chiave piu' grande dell'albero
void upo_bst_delete_max(upo_bst_t tree, int destroy_data)
{
    if(upo_bst_is_empty(tree)){

        return;
    }

    // Chiamata sulla radice dell'albero
    tree->root = upo_bst_delete_max_impl(tree->root, destroy_data);
}

// Implementazione ricerca massimo tra le chiavi che sono minori o uguali alla chiave cercata
static const upo_bst_node_t * upo_bst_floor_impl(const upo_bst_node_t *node, const void *key, upo_bst_comparator_t key_cmp){

    // Controllo che il nodo non sia NULL
    if(node == NULL){

        return NULL;
    }

    int cmp = key_cmp(key, node->key);

    // Se la chiave e' contenuta nel nodo corrente
    if(cmp == 0){

        // Si ritorna il nodo poiche' la chiave e' uguale a quella data in input
        return node;

    }else if(cmp < 0){

        // Se e' minore, ci si sposta nel sottoalbero sx
        return upo_bst_floor_impl(node->left, key, key_cmp);

    }else{

        // Chiamata ricorsiva per capire se la chiave del nodo attuale e' l'unica candidata
        const upo_bst_node_t *floor_node = upo_bst_floor_impl(node->right, key, key_cmp);

        // Il sottoalbero aveva un candidato migliore
        if(floor_node != NULL){

            return floor_node;

        }else{

            return node;
        }    
    }
}

// Ricerca massimo tra le chiavi che sono minori o uguali alla chiave cercata
void* upo_bst_floor(const upo_bst_t tree, const void *key)
{
    if(upo_bst_is_empty(tree)){

        return NULL;
    }

    const upo_bst_node_t *ris = upo_bst_floor_impl(tree->root, key, tree->key_cmp);

    if(ris != NULL){

        return ris->key;

    }else{

        return NULL;
    }
}

// Implementazione ricerca minimoo tra le chiavi che sono maggiori o uguali alla chiave cercata
static const upo_bst_node_t* upo_bst_ceiling_impl(const upo_bst_node_t *node, const void *key, upo_bst_comparator_t key_cmp){

    if(node == NULL){

        return NULL;
    }

    int cmp = key_cmp(key, node->key);

    if(cmp > 0){

        return upo_bst_ceiling_impl(node->right, key, key_cmp);

    }else{

        // La chiave cercata e' minore di quella del nodo corrente
        const upo_bst_node_t *ceiling_node = upo_bst_ceiling_impl(node->left, key, key_cmp);

        if(ceiling_node != NULL){

            return ceiling_node;

        }else{

            return node;
        }
    }
}

// Ricerca minimoo tra le chiavi che sono maggiori o uguali alla chiave cercata
void* upo_bst_ceiling(const upo_bst_t tree, const void *key)
{
    if(upo_bst_is_empty(tree)){

        return NULL;
    }

    const upo_bst_node_t *ris = upo_bst_ceiling_impl(tree->root, key, tree->key_cmp);

    if(ris != NULL){

        return ris->key;

    }else{

        return NULL;
    }
}

// Implementazione lista con valori dell'albero compresi in un certo intervallo (estremi esclusi)
static void upo_bst_keys_range_impl(const upo_bst_node_t *node, const void *low, const void* high, upo_bst_comparator_t key_cmp, upo_bst_key_list_t *list_head){

    if(node == NULL){

        return;
    }

    int cmp_low = key_cmp(low, node->key);
    int cmp_high = key_cmp(high, node->key);

    if(cmp_low < 0){

        // Controllo se nel ramo sinistro sono presenti chiavi nel range target
        upo_bst_keys_range_impl(node->left, low, high, key_cmp, list_head);
    }

    // Se il nodo da cercare è compreso nel range, lo si inserisce nella lista nuova
    if(cmp_low <= 0 && cmp_high >= 0){

        upo_bst_key_list_node_t *new_node = malloc(sizeof(struct upo_bst_key_list_node_s));

        // Controllo l'allocazione
        if(new_node == NULL){

            abort();
        }
        
        // Assegnamento valori
        new_node->key = node->key;
        // Inserimento in testa
        new_node->next = *list_head;
        *list_head = new_node;
    }

    if(cmp_high > 0){

        // Controllo se nel ramo destro sono presenti chiavi nel range target
        upo_bst_keys_range_impl(node->right, low, high, key_cmp, list_head);
    }
}

// Creazione lista con valori dell'albero compresi in un certo intervallo (estremi esclusi)
upo_bst_key_list_t upo_bst_keys_range(const upo_bst_t tree, const void *low_key, const void *high_key)
{
    upo_bst_key_list_t list = NULL; //Creo una nuova lista e inizializzo la sua testa a NULL
    if(upo_bst_is_empty(tree)){
        return NULL;
    }
    //Richiamo la funzione ricorsiva partenedo dalla radice e la testa della lista
    upo_bst_keys_range_impl(tree->root, low_key, high_key, tree->key_cmp, &list); // Do l'inidirizzo di memoria di list da modificare
    //Ritorno la lista aggiornata
    return list;

}

// Implementazione costruzione lista con tutte le chiavi dell'albero in ordine crescente
static void upo_bst_keys_impl(const upo_bst_node_t *node, upo_bst_key_list_t *list_head)
{
    if(node == NULL){
        return;
    }

    //Scendo ricorsivamente sul ramo destro dell'albero
    upo_bst_keys_impl(node->right, list_head);

    //Salvo il nodo corrente in testa alla lista
    upo_bst_key_list_node_t *newnode = malloc(sizeof(struct upo_bst_key_list_node_s));

    if(newnode == NULL){ //controllo che il nodo sia stato creato correttamente
        abort();
    }

    newnode->key = node->key; //Salvo la chiave corrente nel nodo di lista

    //Inserisco il nuovo nodo in testa
    newnode->next = *list_head;
    *list_head = newnode;

    //Scendo ricorsivamente sul ramo sinistro dell'albero
    upo_bst_keys_impl(node->left, list_head);
}

// Costruzione lista con chiavi dell'albero in ordine crescente
upo_bst_key_list_t upo_bst_keys(const upo_bst_t tree)
{
   upo_bst_key_list_t list = NULL; //Creo una nuova lista e inizializzo la sua testa a NULL
    if(upo_bst_is_empty(tree)){
        return NULL;
    }
    upo_bst_keys_impl(tree->root, &list); //Avvio la discesa tramite la radice
    return list; //Ritorno la lista aggiornata;
}

// Implementazione controllo che albero sia effettivamente un BST
static int upo_bst_is_bst_impl(const upo_bst_node_t *node, const void *min, const void *max, upo_bst_comparator_t key_cmp)
{
    if(node == NULL){
        return 1;
    }

    int cmp_low = key_cmp(node->key, min); // Comparo il valore minimo di tutto l'albero con il nodo corrente
    int cmp_high = key_cmp(node->key, max); // Comparo il valore massimo di tutto l'albero con il nodo corrente

    if(cmp_low < 0){
        return 0; //l'albero non é un bst non puó esistere una chiave minore del nodo con chiave minore
    }
    if(cmp_high > 0){
        return 0; //l'albero non é un bst non puó esistere una chiave maggiore del nodo con chiave maggiore
    }
    //Continuo l'esplorazione nel ramo sinistro e nel ramo destro
    //Durante l'esplorazione del ramo sinistro Essendo il figlio il tetto massimo sará il padre
    //Durante l'esplorazione del ramo destro Essendo il figlio il tetto minimo sará il padre
    return upo_bst_is_bst_impl(node->left, min, node->key, key_cmp) && upo_bst_is_bst_impl(node->right, node->key, max, key_cmp);
}

// Controllo che albero sia effettivamente un BST
int upo_bst_is_bst(const upo_bst_t tree, const void *min_key, const void *max_key){

    if(upo_bst_is_empty(tree)){
        return 1; //Un albero vuoto é un bst
    }

    return upo_bst_is_bst_impl(tree->root, min_key, max_key, tree->key_cmp); //Restituisco il risultato dell'esplorazione che parte dalla radice
}

// Comparatore chiavi
upo_bst_comparator_t upo_bst_get_comparator(const upo_bst_t tree)
{
    if (tree == NULL)
    {
        return NULL;
    }

    return tree->key_cmp;
}
