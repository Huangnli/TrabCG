#include <cstdio>
#include "lerComando.h"

#define MAX 50

using namespace std;

lerComando::lerComando(){
     
}

lerComando::~lerComando(){
}

void lerComando::ler(){
    do{
        scanf("%c", &c);
        entrada.push_back(c);
    }while (c != '\n');

    //pra tirar o \n
    entrada.pop_back();
    
}

void lerComando::deleteEntrada(){

    int i = entrada.length();
    for(i; i > 0; i--){
        entrada.pop_back();
    }
}

string lerComando::getEntrada(){
    return entrada;
}