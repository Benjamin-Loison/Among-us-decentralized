#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
#include <chrono>
using namespace std;
using namespace helib;

int main(int argc, char* argv[])
{

    double x1,y1,x2,y2,borne;
    x1=atoi(argv[1]);
    y1=atoi(argv[2]);
    x2=atoi(argv[3]);
    y2=atoi(argv[4]);
    borne=atoi(argv[5]);
    
    printf("x1=%f  y1=%f  x2=%f  y2=%f\n",x1,y1,x2,y2);
    std::cout << "Initialising context object..." << std::endl;

    
    //context initialisation, very specific parmaters to use
    Context context = ContextBuilder<CKKS>()
        .m(16384)
        .bits(119)
        .precision(20)
        .c(2)
        .build();

    cout << "SecurityLevel=" << context.securityLevel() << endl;
    
    long n = context.getNSlots();
    //key genereations
    SecKey sKey(context);
    sKey.GenSecKey();
    const PubKey& pKey = sKey; 

    vector<complex<double>> pos1(n);
    vector<complex<double>> pos2(n);
    vector<complex<double>> r(n);

    for (long i = 0; i < n; i++) {
        pos1[i]=complex<double>(x1,y1);
        pos2[i]=complex<double>(x2,y2);
        r[i]=complex<double>(borne,0.);
    }
    
    addSomeFrbMatrices(sKey);
    //inisialisation as plaintexts
    PtxtArray p1(context,pos1);
    PtxtArray p2(context,pos2);
    PtxtArray pr(context,r);

    Ctxt c1(pKey);
    Ctxt c2(pKey);
    Ctxt cr(pKey);
    cout << "START" << endl;
    auto a = chrono::steady_clock::now();
    p1.encrypt(c1);
    p2.encrypt(c2);
    pr.encrypt(cr);

    c1-=c2;
    c2=c1;
    conjugate(c1);
    c1*=c2;

    PtxtArray result(context);
    result.decrypt(c1, sKey);
    vector<double> final;
    result.store(final);
    auto b = chrono::steady_clock::now();
    cout << "result=" << final[0] << endl;
    cout << "computation time: " << chrono::duration_cast<chrono::microseconds>(b - a).count() << endl;
    return 0;


}