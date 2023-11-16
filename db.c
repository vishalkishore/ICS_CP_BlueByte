#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Enum to represent data types
typedef enum {
    INT,
    DOUBLE,
    STRING,
    DICTIONARY
} DataType;

// Union to store different data types
typedef union {
    int int_val;
    double double_val;
    char* string_val;
    struct Dictionary* dictionary_val;
} DataValue;

// Define a structure for key-value pairs with data type
typedef struct KeyValuePair {
    char* key;
    DataType type;
    DataValue value;
} KeyValuePair;

// Define a structure for nested dictionaries
typedef struct Dictionary {
    KeyValuePair* data;
    size_t size;
} Dictionary;

// Define a structure for the in-memory database collection
typedef struct {
    char* name;
    KeyValuePair* data;
    size_t size;
} Collection;

// Define a structure for the in-memory database
typedef struct {
    Collection* collections;
    size_t num_collections;
} InMemoryDatabase;

// Function to initialize a nested dictionary
void initializeDictionary(Dictionary* dictionary) {
    dictionary->data = NULL;
    dictionary->size = 0;
}

// Function to initialize the in-memory database
void initializeDatabase(InMemoryDatabase* db) {
    db->collections = NULL;
    db->num_collections = 0;
}

// Function to add a collection to the database
void addCollection(InMemoryDatabase* db, const char* name) {
    db->collections = realloc(db->collections, (db->num_collections + 1) * sizeof(Collection));
    db->collections[db->num_collections].name = strdup(name);
    db->collections[db->num_collections].data = NULL;
    db->collections[db->num_collections].size = 0;
    db->num_collections++;
}

// Function to add a key-value pair to a collection in the database
void addToCollection(InMemoryDatabase* db, const char* collectionName, const char* key, DataType type, DataValue value) {
    // Find the collection
    for (size_t i = 0; i < db->num_collections; ++i) {
        if (strcmp(db->collections[i].name, collectionName) == 0) {
            // Check if the key already exists
            for (size_t j = 0; j < db->collections[i].size; ++j) {
                if (strcmp(db->collections[i].data[j].key, key) == 0) {
                    // Update the value if the key already exists
                    db->collections[i].data[j].type = type;
                    if (type == STRING) {
                        free(db->collections[i].data[j].value.string_val);
                        db->collections[i].data[j].value.string_val = strdup(value.string_val);
                    } else if (type == DICTIONARY) {
                        // Free existing dictionary and copy the new one
                        free(db->collections[i].data[j].value.dictionary_val);
                        db->collections[i].data[j].value.dictionary_val = malloc(sizeof(Dictionary));
                        memcpy(db->collections[i].data[j].value.dictionary_val, value.dictionary_val, sizeof(Dictionary));
                    } else {
                        db->collections[i].data[j].value = value;
                    }
                    return;
                }
            }

            // Allocate memory for a new key-value pair
            db->collections[i].data = realloc(db->collections[i].data, (db->collections[i].size + 1) * sizeof(KeyValuePair));
            db->collections[i].data[db->collections[i].size].key = strdup(key);
            db->collections[i].data[db->collections[i].size].type = type;
            if (type == STRING) {
                db->collections[i].data[db->collections[i].size].value.string_val = strdup(value.string_val);
            } else if (type == DICTIONARY) {
                db->collections[i].data[db->collections[i].size].value.dictionary_val = malloc(sizeof(Dictionary));
                memcpy(db->collections[i].data[db->collections[i].size].value.dictionary_val, value.dictionary_val, sizeof(Dictionary));
            } else {
                db->collections[i].data[db->collections[i].size].value = value;
            }
            db->collections[i].size++;
            return;
        }
    }

    // Collection not found
    fprintf(stderr, "Collection %s not found\n", collectionName);
}

// Function to get the value associated with a key from a collection in the database
DataValue getFromCollection(const InMemoryDatabase* db, const char* collectionName, const char* key) {
    DataValue notFound;
    notFound.string_val = NULL;

    for (size_t i = 0; i < db->num_collections; ++i) {
        if (strcmp(db->collections[i].name, collectionName) == 0) {
            for (size_t j = 0; j < db->collections[i].size; ++j) {
                if (strcmp(db->collections[i].data[j].key, key) == 0) {
                    return db->collections[i].data[j].value;
                }
            }
            // Key not found in the collection
            fprintf(stderr, "Key %s not found in collection %s\n", key, collectionName);
            return notFound;
        }
    }

    // Collection not found
    fprintf(stderr, "Collection %s not found\n", collectionName);
    return notFound;
}

// Function to free the memory allocated for a nested dictionary
void freeDictionary(Dictionary* dictionary) {
    for (size_t i = 0; i < dictionary->size; ++i) {
        free(dictionary->data[i].key);
        if (dictionary->data[i].type == STRING) {
            free(dictionary->data[i].value.string_val);
        } else if (dictionary->data[i].type == DICTIONARY) {
            freeDictionary(dictionary->data[i].value.dictionary_val);
            free(dictionary->data[i].value.dictionary_val);
        }
    }
    free(dictionary->data);
}

// Function to free the memory allocated for the in-memory database
void freeDatabase(InMemoryDatabase* db) {
    for (size_t i = 0; i < db->num_collections; ++i) {
        free(db->collections[i].name);
        for (size_t j = 0; j < db->collections[i].size; ++j) {
            free(db->collections[i].data[j].key);
            if (db->collections[i].data[j].type == STRING) {
                free(db->collections[i].data[j].value.string_val);
            } else if (db->collections[i].data[j].type == DICTIONARY) {
                freeDictionary(db->collections[i].data[j].value.dictionary_val);
                free(db->collections[i].data[j].value.dictionary_val);
            }
        }
        free(db->collections[i].data);
    }
    free(db->collections);
}

#include <stdio.h>
#include <string.h>

// Function to check if a value matches the filter condition
int matchesFilter(const DataValue* value, const char* condition, const char* filter) {
    if (value->string_val != NULL && strcmp(condition, "=") == 0) {
        return strcmp(value->string_val, filter) == 0;
    } else if (value->int_val && strcmp(condition, "=") == 0) {
        int filterValue;
        sscanf(filter, "%d", &filterValue);
        return value->int_val == filterValue;
    } else if (value->double_val && strcmp(condition, "=") == 0) {
        double filterValue;
        sscanf(filter, "%lf", &filterValue);
        return value->double_val == filterValue;
    }

    return 0; // No match
}

// Function to parse and execute a query with filtering
void executeQueryWithFilter(InMemoryDatabase* db, const char* query) {
    char command[20], collectionName[50], key[50], condition[3], filter[50];
    int parsed = sscanf(query, "%s %s %s %s %s", command, collectionName, key, condition, filter);

    if ((parsed == 5) && ((strcmp(command, "GET") == 0) || (strcmp(command, "FILTER") == 0))) {
        // Execute GET or FILTER query
        DataValue result = getFromCollection(db, collectionName, key);

        if (result.string_val != NULL) {
            if ((strcmp(command, "GET") == 0) || (matchesFilter(&result, condition, filter))) {
                printf("%s: %s\n", key, result.string_val);
            } else {
                printf("Filter condition not met for %s in collection %s\n", key, collectionName);
            }
        } else {
            printf("Key not found in collection %s\n", collectionName);
        }
    } else {
        printf("Invalid query\n");
    }
}

int main() {
    InMemoryDatabase db;
    initializeDatabase(&db);

    // Example queries with filtering
    executeQueryWithFilter(&db, "SET employees employee1_name STRING John Doe");
    executeQueryWithFilter(&db, "SET employees employee1_age INT 30");
    executeQueryWithFilter(&db, "SET employees employee1_salary DOUBLE 50000.00");
    executeQueryWithFilter(&db, "GET employees employee1_name");
    executeQueryWithFilter(&db, "FILTER employees employee1_age = 30");

    // Free allocated memory
    freeDatabase(&db);

    return 0;
}

