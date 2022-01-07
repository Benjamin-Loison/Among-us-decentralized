#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
using namespace std;
using namespace helib;

//init should be a vector full of zeros 
long compute_distance(Ctxt c1,Ctxt c2,PtxtArray p,PtxtArray init, Context context,SecKey skey){
    c1-=c2;
    c1*=c1;
    c2=c1;
    rotate(c1,1);
    c1+=c2;

    p.decrypt(c1,skey);
    return Distance(p,init);
}