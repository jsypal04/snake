#ifndef CJSON_H
#define CJSON_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_FILE_SIZE_B 1048576 // Maximum allowed json file measured in bytes (equivalent to 1MB)

/*
 * Data Structure to store the JSON data (An ordered map)
 */
#define STRING    's'
#define INT       'i'
#define FLOAT     'f'
#define BOOLEAN   'b'
#define ARRAY     'a'
#define MAP       'm'

typedef struct {
    char* key;
    void* value;
    char type; // can be either s (string), i (int), f (float), b (bool), a (), m (map)
} KeyValuePair;

typedef struct {
    KeyValuePair* pairs;
    int size;
    int mapCap;
} Map;

typedef struct {
    void* value;
    char type;
} Element;

typedef struct {
    Element* array;
    int size;
    int cap;
} MapArray;

// Function to allocate the necessary heap memory for a map with a given initial capacity
Map* initMap(int initMapCap);

/* Function to free all allocated memory used to create the map
 *
 * PLEASE NOTE: This function does NOT free memory of the values stored in the in the map.
 *  you must go through the map destroying any values that you allocated and then stored in the map.
 * */
void destroyMap(Map* map);

/*
 * Duplicates `source` and returns a pointer to the new map.
 * - `source` is not destroyed, the caller still needs to destroy it.
 * - NOTE: If a nested map is encountered, need to use recursion.
 * */
Map* mapdup(Map* source);
MapArray* arrdup(MapArray* array);

// Function to allocate the necessary heap memory for an array with a given initial capacity
MapArray* initMapArray(int arr_cap);
void destroyMapArray(MapArray* array);

// Functions to insert values into an array
void appendInt(     MapArray** arr_ref, int       value);
void appendFloat(   MapArray** arr_ref, float     value);
void appendString(  MapArray** arr_ref, char*     value);
void appendMap(     MapArray** arr_ref, Map*      value);
void appendMapArray(MapArray** arr_ref, MapArray* value);


// Functions to insert a value with a given key and type into the map
void insertInt(     Map** map_ref, char* key, int       value);
void insertFloat(   Map** map_ref, char* key, float     value);
void insertString(  Map** map_ref, char* key, char*     value);
void insertMap(     Map** map_ref, char* key, Map*      value);
void insertMapArray(Map** map_ref, char* key, MapArray* value);


/* Function to retrieve the value mapped to a given key
 *
 * The `type` parameter is the address where the type of the retrieved value will be stored.
 * You must use it to cast the void* to the correct type after the value has been retrieved.
 * */
void* get(Map* map, char* key, char* type);

/* Retrieves the value at a given index of the MapArray.
 *
 * The `type` parameter is the address where the type of the retrived value will be stored.
 * You must use it to cast the void* to the correct typr after the value has been retrieved
*/
void* read_arr(MapArray* array, int index, char* type);

bool map_cmp(Map* map1, Map* map2);
bool arr_cmp(MapArray* arr1, MapArray* arr2);

void printMap(Map* map);
void printMapArray(MapArray* array);

/*
 * Functions to load json parsed by the parser into a Map
 * */
Map* load_file(const char* path);

bool is_float(char* value);

/* 
 * Serialize a C Map into a json string
 * */
char* dump(Map* map);

/*
 * Serialize a C Map into a .json file
 *
 * The `path` param sepecifies where the output directory of the file (must include the filename)
 *
 * A non zero return value indicates failure
 * 
 * NOTE: if a file already exists at the path, it will be overwritten!
 * */
int dumpf(Map* map, const char* output_path);

#endif
