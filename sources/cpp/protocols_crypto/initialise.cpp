#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
using namespace std;
using namespace helib;

//very simple program, generating a secret and a public key and storing both to a json file

int main(int argc, char* argv[])
{

        Context context = ContextBuilder<CKKS>()
        .m(16384)
        .bits(119)
        .precision(20)
        .c(2)
        .build();

    if (argc<4){
        cout << "[skey path] [pkey path] [context path]" << endl;
    }
    else {
        
        //key generations
        SecKey sKey(context);
        sKey.GenSecKey();
        const PubKey& pKey = sKey; 

        ofstream skeyfile, pkeyfile, contextfile;
        skeyfile.open(argv[1]);
        pkeyfile.open(argv[2]);
        contextfile.open(argv[3]);

        skeyfile << sKey.writeToJSON() ;
        pkeyfile << pKey.writeToJSON() ;
        //we also need to specify the context, but all players should have the same, so no real need to share it
        contextfile << context.writeToJSON();

        skeyfile.close();
        pkeyfile.close();
        contextfile.close();
    };

    return 0;




}