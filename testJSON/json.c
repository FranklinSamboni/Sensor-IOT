#include <stdio.h>
#include <json/json.h>

int main(){
    // Creando JSON object
    json_object * jobj = json_object_new_object();

    // Creando Json string
    json_object * jstring = json_object_new_string("Hola");

    //añadiendo el string al objeto JSON
    json_object_object_add(jobj,"msg",jstring);

    printf("Json: %s\n", json_object_to_json_string(jobj));


}
