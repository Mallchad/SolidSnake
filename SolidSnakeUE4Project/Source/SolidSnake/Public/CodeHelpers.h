
#pragma once

/**
   These are a collection of macros that provide code search hints, build system
   hints, and stub helper macros like stubs.
*/

// States the declaration origin, is distinct from things like  forward declarations
#define DECLARE
// States a function definition or a declaration of any kind
#define FUNCTION
// States a variable declaration, primarily intended for classes and structs
#define DATA
// States a variable declaration
#define VARIABLE
#define VAR
// States a forward declaration
#define FWD
#define FWD_FUNCTION
#define FWD_CLASS class
#define FWD_STRUCT struct
#define FORWARD
#define FORWARD_FUNCTION
#define FORWARD_CLASS class
#define FORWARD_STRUCT struct
// States a function should be pure
#define PURE
#define PURE_FUNCTION
// States a class is intended to be inheritable
#define BASE
#define BASE_FUNCTION
// States a constructor declaration or definition
#define CTOR
#define CONSTRUCTOR
// States a destructor declaration or definition
#define DTOR
#define DESTRUCTOR
// States a move constructor declaration or definition
#define MOVE_
#define MOVE_CONSTRUCTOR(ARG_CLASS, ARG_PARAM_NAME)
// States a move constructor declaration or definition
#define COPY_CTOR
#define COPY_CONSTRUCTOR(ARG_CLASS, ARG_PARAM_NAME)

// States a class or structis being inherited
#define INHERIT
// States the following code containers members
#define MEMBERS
// States the following code containers functions
#define FUNCTIONS
// States the following code contains friend declarations
#define FRIENDS

// States following hints are public
#define PUBLIC_DATA public
#define PUBLIC_MEMBERS public
#define PUBLIC_VARIABLES public
#define PUBLIC_FUNCTIONS public
// States the following hints are protected (internal to the class)
#define INTERNAL_DATA protected
#define INTERNAL_MEMBERS protected
#define INTERNAL_VARIABLES protected
#define INTERNAL_FUNCTIONS protected
// States the following hints are private
#define PRIVATE_DATA private
#define PRIVATE_MEMBERS private
#define PRIVATE_VARIABLES private
#define PRIVATE_FUNCTIONS private
#define FRIEND_CLASSES friend

#if  !(__UNREAL__)
#define ABSTRACT
#define ABSTRACT_CLASS class
#define DECLARE_CLASS class
#define PURE_VIRTUAL
#endif

// No Operation empty statements, can silence warnings
#define NOOP (void)(0)
#define NOP (void)(0)
// Empty macros, can pad declarations for tools
#define NODECLARE
#define NODEC
