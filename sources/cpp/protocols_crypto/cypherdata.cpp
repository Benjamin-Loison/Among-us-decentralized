#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
using namespace std;
using namespace helib;

// using pkey json file, cypher a given data 

int main(int argc, char* argv[]){
    if (argc<6 ){
        cout << "[pkey path] [context path] [x] [y] [cypher path]" << endl;
    }
    else {
        ifstream pkeyfile,contextfile ; 
        pkeyfile.open(argv[1]);
        contextfile.open(argv[2]);
        Context context = Context::readFromJSON(contextfile);
        PubKey pKey = PubKey::readFromJSON(pkeyfile,context);

        int x,y;
        x=atoi(argv[3]);
        y=atoi(argv[4]);

        long n = context.getNSlots();
        vector<long> pos(n);
        for (long i = 0; i<n ;i+=2){
            pos[i]=x;
            pos[i+1]=y;
        }

        PtxtArray ptxt(context,pos);
        cout << ptxt.writeToJSON();
        Ctxt ctxt(pKey);
        ptxt.encrypt(ctxt);

        ofstream ctxtfile;
        ctxtfile.open(argv[5]);
        ctxtfile<< ctxt.writeToJSON();
        ctxtfile.close();

    } ;

    return 0;

}
