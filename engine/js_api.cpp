#include <string>

extern  "C" {
const char* getLegalMoves(const char* fen){
    char* str = (char*) malloc(80);
    strcpy(str, "e2e3 e2e4");
    strcat(str, fen);
    return str;
}
}