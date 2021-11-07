
#pragma once

/**
   These are a collection of macros that provide code search hints, build system
   hints, and stub helper macros like stubs.
*/

#define DECLARE
#define FUNCTION
#define FWD
#define FWD_FUNCTION
#define FWD_CLASS class
#define FWD_STRUCT struct
#define FORWARD
#define FORWARD_FUNCTION
#define FORWARD_CLASS class
#define FORWARD_STRUCT struct
#define PURE
#define PURE_FUNCTION
#define BASE
#define BASE_FUNCTION
#define CTOR
#define CONSTRUCTOR
#define DTOR
#define DESTRUCTOR
#define MOVE_CTOR
#define COPY_CTOR
#define COPY_CONSTRUCTOR(ARG_CLASS, ARG_PARAM_NAME)
#define MOVE_CONSTRUCTOR(ARG_CLASS, ARG_PARAM_NAME)

#define INHERIT
#define DATA
#define MEMBERS
#define FUNCTIONS
#define MEMBERS
#define FRIENDS
#define PUBLIC_DATA public
#define PUBLIC_MEMBERS public
#define PUBLIC_VARIABLES public
#define PUBLIC_FUNCTIONS public
#define INTERNAL_DATA protected
#define INTERNAL_MEMBERS    protected
#define INTERNAL_VARIABLES  protected
#define INTERNAL_FUNCTIONS  protected
#define PRIVATE_DATA        private
#define PRIVATE_MEMBERS     private
#define PRIVATE_VARIABLES   private
#define PRIVATE_FUNCTIONS   private
#define FRIEND_CLASSES      friend

#if  !(__UNREAL__)
#define ABSTRACT
#define ABSTRACT_CLASS class
#define DECLARE_CLASS       class
#define PURE_VIRTUAL
#endif

// Empty statements, can silence warnings
#define NOOP                (void)(0)
#define NOP                 (void)(0)
// Empty macros, can pad declarations for tools
#define NODECLARE
#define NODEC
