#include <iostream>
#include "NavMeshEditorApp.h"

using namespace std;

int main (int argc, char *argv[])
{
    cout<<"Instrucciones:"<<endl;
    cout<<"--------------"<<endl;
    cout<<"- Presione A para entrar en el modo de edicion de regiones caminables"<<endl<<endl;
    cout<<"- Presione S para entrar en el modo de edicion de regiones de obstaculos"<<endl<<endl;
    cout<<"- Presione Q para ver el Navmesh resultado"<<endl<<endl;
    cout<<"- Para crear poligonos haga click y arrastre el mouse"<<endl<<endl;
    cout<<"- Para mover poligonos haga click izquierdo sobre el poligono con el modo de"<<endl << "edicion correcto y arrastre"<<endl<<endl;
    cout<<"- Para rotar poligonos haga click derecho sobre el poligono con el modo de"<<endl<< "edicion correcto y arrastre"<<endl<<endl;

    NavMeshEditorApp& app = NavMeshEditorApp::getInstance();
    return app.run();
}