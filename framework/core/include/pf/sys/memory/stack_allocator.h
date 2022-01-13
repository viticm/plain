/**
 * PLAIN FREAMEWORK ( https://github.com/viticm/plain )
 * $Id stack_alloc.h
 * @link https://github.com/viticm/plain for the canonical source repository
 * @copyright Copyright (c) 2022 viticm( viticm.ti@gmail.com )
 * @license
 * @user viticm( viticm.ti@gmail.com )
 * @date 2022/01/13 15:39
 * @uses The simple stack allocator for memory pool.
 */
#ifndef PF_SYS_MEMORY_STACK_ALLOCATOR_H_
#define PF_SYS_MEMORY_STACK_ALLOCATOR_H_

#include "pf/sys/memory/config.h"

namespace pf_sys {

namespace memory {

template <typename T>
struct stack_node_t {
  T data;
  stack_node_t* prev;
};

/** T is the object to store in the stack, Alloc is the allocator to use */
template <class T, class Alloc = std::allocator<T> >
class StackAllocator {
 public:
   typedef stack_node_t<T> Node;
   typedef typename Alloc::template rebind<Node>::other allocator;
 
   /** Default constructor */
   StackAllocator() { head_ = 0; }
   /** Default destructor */
   ~StackAllocator() { clear(); }
 
   /** Returns true if the stack is empty */
   bool empty() {return (head_ == 0);}
 
   /** Deallocate all elements and empty the stack */
   void clear() {
     Node* curr = head_;
     while (curr != 0) {
       Node* tmp = curr->prev;
       allocator_.destroy(curr);
       allocator_.deallocate(curr, 1);
       curr = tmp;
     }
     head_ = 0;
   }
 
   /** Put an element on the top of the stack */
   void push(T element) {
     Node *newNode = allocator_.allocate(1);
     allocator_.construct(newNode, Node());
     newNode->data = element;
     newNode->prev = head_;
     head_ = newNode;
   }
 
   /** Remove and return the topmost element on the stack */
   T pop() {
     T result = head_->data;
     Node *tmp = head_->prev;
     allocator_.destroy(head_);
     allocator_.deallocate(head_, 1);
     head_ = tmp;
     return result;
   }
 
   /** Return the topmost element */
   T top() { return (head_->data); }
 
 private:
   allocator allocator_;
   Node *head_;
};

} // namespace memory

} // namespace pf_sys

#endif // PF_SYS_MEMORY_STACK_ALLOCATOR_H_
