* If `main` is entered for the first time, set offset for the stack (because getting the number of functions above `main` in the stack is complicated)
    * Currently, offset is set for each entry in function `main`. This should not create bugs
    * `number_of_functions_on_stack_in_last_check` is set to 0 because we want to offset and this value be different if we enter the `main` function for the first time

* If we trace execution check if the number of functions on the stack is same as before in `IMark`
	* if it is not, then a new function is called, or left, and we should:
        * dump table
            * CHANGE THIS IF we dump when function exit, not if it just calls another function
        * create new table
    * If it is, we just ignore this machine instruction
        * or do this
        ```c
        else{
            // if number of hash tables is 0,
            // we are first time in main and hash table should be created
            if (table == NULL){
                table = VG_(HT_contruct)("hash_table_name");
            }
        }
        ```

* If we trace execution and LOAD/STORE/MODIFY happen, in `trace_load/store/modify` we should create node for hash table, hash it, and check does that node exist in table
    * if it does exist, access counter should be updated
    * if it does not exist, new node in table should be created
        * to existing data, header for hash table should be added

* __dump table__ needs to:
    * go through all nodes and call print function
    * free table

* Functions for __node__ needs to:
    * create new node
    * for every data in node should exist get/set functions
    * add __hash_table__ data in node


**Stack**



b()
^
|
a()
^
|
main()

```c

struct stack{
    stack *pointer;
    char *function_name;
    table *table;
};

if ( unlikely( nr_of_fn_on_stack != nr_of_fn_on_stack_in_last_check)){
    if (nr_of_fn_on_stack > nr_of_fn_on_stack_in_last_check){
        create_table();
        stack_push();
    }
    else{
        print_table(table);
        stack_pop();
    }
```

* crete stack for function call trace