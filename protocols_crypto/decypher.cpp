#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
using namespace std;
using namespace helib;

// using skey json file, decypher a given data 

int main(int argc, char* argv[]){
    if (argc<5 ){
        cout << "[skey path] [context path] [cypher data path] [decyphered path]" << endl;
    }
    else {
        ifstream skeyfile,contextfile,ctxtfile ; 
        skeyfile.open(argv[1]);
        contextfile.open(argv[2]);
        ctxtfile.open(argv[3]);

        Context context = Context::readFromJSON(contextfile);
        SecKey sKey = SecKey::readFromJSON(skeyfile,context);

        skeyfile.close();
        contextfile.close();

        //we also need pubkey to decipher, it might be faster to just get it from a json file, 
        //but for now, we juste regenerate it from sKey
        const PubKey& pKey = sKey;

        Ctxt ctxt = Ctxt::readFromJSON(ctxtfile,pKey);
        
        ctxtfile.close();

        PtxtArray ptxt(context);
        ptxt.decrypt(ctxt,sKey);

        ofstream ptxtfile;
        ptxtfile.open(argv[4]);
        ptxtfile << ptxt.writeToJSON();

    } ;

    return 0;

}
