#include "pub_tool_basics.h"

#include "vr_hash.h"

unsigned long hash_djb2(char *str){
    unsigned long hash = 5381;
    int c;
    //VG_(printf)("STRING: %s\n", str);

    while (c=*str++){
        hash = ((hash << 5) + hash) + c;
    }
    //VG_(printf)("HASH: %lu\n", hash);
    return hash;
}


unsigned long xor (unsigned long *data, int n){
    int i;
    unsigned long result = data[0];
    for (i = 1;i < n; i++){
        result = result ^ data[i];
    }
    
    return result;
}

unsigned long hash_then_xor(var_info *data){
    
    unsigned long hashed_variable_data[4];
    unsigned long xored_value;
    char str[256];

    
    hashed_variable_data[0] = hash_djb2(data->variable_name);
        
    // convert unsigned long into string
    VG_(sprintf)(str, "%lu", data->address);
    
    hashed_variable_data[1] = hash_djb2(str);
    hashed_variable_data[2] = hash_djb2(data->declared_at);
    
    VG_(sprintf)(str, "%d", data->type);
    hashed_variable_data[3] = hash_djb2(str);

    xored_value = xor(hashed_variable_data, 4);

    VG_(sprintf)(str, "%lu", xored_value);
   // VG_(printf)("xored: %lu\n\n",xored_value);
    return hash_djb2(str);
}
