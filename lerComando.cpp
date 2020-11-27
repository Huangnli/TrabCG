#include <cstdio>
#include "lerComando.h"

#define MAX 50

using namespace std;

lerComando::lerComando(){
    do{
        scanf("%c", &c);
        entrada.push_back(c);
    }while (c != '\n');
    entrada.pop_back(); //pra tirar o \n

    //string addcubo("add_shape cube");
    //if( entrada.compare(addcubo) == 0 )
      //  cube = 1;
}

lerComando::~lerComando(){
}

string lerComando::getEntrada(){
    return entrada;
}