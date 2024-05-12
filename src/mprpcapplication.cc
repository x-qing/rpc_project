#include "mprpcapplication.h"

void MprpcApplication::Init(int argc,char **argv){

}


MprpcApplication& MprpcApplication::getInstance(){
    static MprpcApplication app;
    return app;
}