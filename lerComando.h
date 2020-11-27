#include <string>
#include <cstdio>


using namespace std;

class lerComando{
private:
    char c;
    string entrada;
    
public:
    lerComando();
    ~lerComando();
    void ler();
    void deleteEntrada();
    string getEntrada();
};
