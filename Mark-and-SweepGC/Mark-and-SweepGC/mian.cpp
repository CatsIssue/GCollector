#include<stdlib.h> 
#include<assert.h>
#include<stdio.h>


#define STACK_MAX 256

typedef enum {
	OBJ_INT,
	OBJ_PAIR,
} ObjectType;

typedef struct sObject {
	unsigned char marked;
	ObjectType type;
	
	// next node in the list of all objects 
	struct sObject* next;
	
	union {
		// OBJ_INT
		int value;
	};

	//OBJ_Pair
	struct {
		struct sObject* head;
		struct sObject* tail;			// union - no need have 3 fields either int or pair
	};
} Object;

typedef struct {
	// Total number of currently allocated objects
	int numObjects;

	// Number of objects required for trigger a GC
	int maxObjects;

	// the first object in the list of all objects
	Object* firstObject;


	Object* stack[STACK_MAX];
	int stackSize;
} VM; // role for to have a stack 

VM* newVm() {
	VM* vm = (VM *)malloc(sizeof(VM)); // or (VM *)malloc(sizeof(Object *) * STACK_MAX)
	vm->stackSize = 0;
	
	vm->numObjects++;
	vm->maxObjects = 8;

	return vm; 

}

// once got a VM -> need to be able to manipulate the stack 

void push(VM* vm, Object* value) {
	assert(vm->stackSize < STACK_MAX, "Stack overflow");
	vm->stack[vm->stackSize++] = value;
}

Object* pop(VM* vm) {
	assert(vm->stackSize > 0, "Stack underflow! ");
	return vm->stack[--vm->stackSize];
}

// Create Objects 

Object* newObject(VM* vm, ObjectType type) {
	if (vm->numObjects == vm->maxObjects) garbageCollector(vm);

	Object* object = (Object *)malloc(sizeof(Object*));
	object->type = type;
	// Once created mark as still unreacheble
	object->marked = 0; 
	vm->numObjects++;
	//Insert it into the list of allocated objects
	object->next = vm->firstObject;
	// !!!!!
	vm->firstObject = object;

	return object;
}

void pushInt(VM* vm, int intValue) {
	Object* object = newObject(vm, OBJ_INT);
	object->value = intValue;
	push(vm, object);
}

Object* PushPair(VM* vm) {
	Object* object = newObject(vm, OBJ_PAIR);
	object->tail = pop(vm);
	object->head = pop(vm);

	push(vm, object);
	return object;
}

// Code for mark every reacheble object in memory 
// next phase is to free all space of all objects that aren't marked

// as so as no marked objects are object that are unreacheble -> cannot find them
// can keep all pointers on objects ever allocated

void mark(Object* object) {
	// to avoid recursing on cycles - Check
	// already marked.
	if (object->marked) return;

	object->marked = 1;

	if (object->type == OBJ_PAIR) {
		mark(object->head);
		mark(object->tail);
	}

}

void markAll(VM * vm) {
	for (int i = 0; i < vm->stackSize; i++) {
		mark(vm->stack[i]);
	}
}


// Traverse trhough list and delete all objects
void sweep(VM* vm) {
	Object** object = &vm->firstObject;
	while (*object) {
		if (!(*object)->marked) {
			// cannot reaching -> Thus remove from the list and free it
			Object* unreached = *object;

			
			*object = unreached->next;
			free(unreached);
			vm->numObjects--;
		} else {
			// this object was reached => mark as unreached for next garbage collector traverse 
			(*object)->marked = 0;
			object = &(*object)->next;
		

		}
	}
}

void garbageCollector(VM* vm) {
	int numObjects = vm->numObjects;
	 markAll(vm);
	 sweep(vm);

	 printf("Collected %d objectes, remaining %d", numObjects - vm->numObjects, vm->numObjects);
}