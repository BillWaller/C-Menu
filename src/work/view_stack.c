/** @brief Initialize View Stack
    @ingroup view_engine
    @param s pointer to ViewStack structure
    @param initial_capacity initial capacity of the stack
    @returns true if successful, false if memory allocation fails
    @details This function initializes a ViewStack structure by allocating
   memory for the items array with the specified initial capacity. It sets
   the capacity and top index accordingly. If memory allocation fails, it
   returns false.
 */
bool view_stack_init(ViewStack *s, size_t initial_capacity) {
    s->items = malloc(initial_capacity * sizeof(View));
    if (!s->items)
        return false;
    s->capacity = initial_capacity;
    s->top = 0;
    return true;
}
/** @brief Push Item onto View Stack
    @ingroup view_engine
    @param s pointer to ViewStack structure
    @param item View item to push onto the stack
    @returns true if successful, false if memory allocation fails during
   resizing
    @details This function pushes a View item onto the stack. If the stack is
   full, it reallocates memory to double the capacity. If memory allocation
   fails during resizing, it returns false.
 */
bool view_stack_push(ViewStack *s, View item) {
    if (s->top >= s->capacity) {
        size_t new_capacity = s->capacity * 2;
        View *new_items = realloc(s->items, new_capacity * sizeof(View));
        if (!new_items)
            return false; // Out of memory
        s->items = new_items;
        s->capacity = new_capacity;
    }
    s->items[s->top++] = item; // Structure copy
    return true;
}
/** @brief Pop Item from View Stack
    @ingroup view_engine
    @param s pointer to ViewStack structure
    @param out_item pointer to View structure where the popped item will be
   stored
    @returns true if successful, false if the stack is empty (underflow)
    @details This function pops a View item from the stack and stores it in
   the provided out_item pointer. If the stack is empty, it returns false.
 */
bool view_stack_pop(ViewStack *s, View *out_item) {
    if (s->top == 0)
        return false; // Stack underflow
    *out_item = s->items[--s->top];
    return true;
}
/** @brief Peek at Top Item of View Stack
    @ingroup view_engine
    @param s pointer to ViewStack structure
    @param out_item pointer to View structure where the top item will be
   stored
    @returns true if successful, false if the stack is empty
    @details This function retrieves the top item of the stack without
   removing it. If the stack is empty, it returns false.
 */
bool view_stack_peek(const ViewStack *s, View *out_item) {
    if (s->top == 0)
        return false;
    *out_item = s->items[s->top - 1];
    return true;
}
/** @brief Free View Stack
    @ingroup view_engine
    @param s pointer to ViewStack structure
    @details This function frees the memory allocated for the items array in
   the ViewStack structure and resets the capacity and top index to zero.
 */
void view_stack_free(ViewStack *s) {
    free(s->items);
    s->items = nullptr;
    s->capacity = 0;
    s->top = 0;
}
