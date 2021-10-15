#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/time.h>
#include <helib/helib.h>
using namespace std;

int main(int argc, char* argv[])
{
        // Plaintext prime modulus
    unsigned long p = 4999;
    // Cyclotomic polynomial - defines phi(m)
    unsigned long m = 231;
    // Hensel lifting (default = 1)
    unsigned long r = 1;
    // Number of bits of the modulus chain
    unsigned long bits = 10;
    // Number of columns of Key-Switching matrix (default = 2 or 3)
    unsigned long c = 2;

    int x=10;
    int y=5;
    printf("x=%d  y=%d\n",x,y);
    std::cout << "Initialising context object..." << std::endl;
    // Initialize context
    // This object will hold information about the algebra created from the
    // previously set parameters
    helib::Context context = helib::ContextBuilder<helib::BGV>()
                                .m(m)
                                .p(p)
                                .r(r)
                                .bits(bits)
                                .c(c)
                                .build();

    // Print the context
    context.printout();
    std::cout << std::endl;

    // Print the security level
    std::cout << "Security: " << context.securityLevel() << std::endl;
    // Secret key management
    std::cout << "Creating secret key..." << std::endl;
    // Create a secret key associated with the context
    helib::SecKey secret_key(context);
    // Generate the secret key
    secret_key.GenSecKey();
    std::cout << "Generating key-switching matrices..." << std::endl;

    // Public key management
    // Set the secret key (upcast: SecKey is a subclass of PubKey)
    const helib::PubKey& public_key = secret_key;

    // Get the EncryptedArray of the context
    const helib::EncryptedArray& ea = context.getEA();

    // Get the number of slot (phi(m))
    long nslots = ea.size();
    std::cout << "Number of slots: " << nslots << std::endl;
    timeval a,b;
    // Create a vector of long with nslots elements
    helib::Ptxt<helib::BGV> p1(context);
    helib::Ptxt<helib::BGV> p2(context);
    
    gettimeofday(&a,0);
    // Set it with numbers 0..nslots - 1
    // ptxt = [0] [1] [2] ... [nslots-2] [nslots-1]
    p1[0]=x;
    p2[0]=y;

    // Print the plaintext
    std::cout << "Initial P1: " << p1 << std::endl;
    std::cout << "Initial P2: " << p2 << std::endl;

    helib::Ctxt c1(public_key);
    helib::Ctxt c2(public_key);

    public_key.Encrypt(c1,p1);
    public_key.Encrypt(c2,p2);
    c1+=c2;

    //plaintext for decryption
    helib::Ptxt<helib::BGV> p3(context);
    secret_key.Decrypt(p3,c1);
    std::cout << p3[0] <<std::endl;
    gettimeofday(&b,0);
    cout << (b.tv_usec - a.tv_usec) << endl;
    return 0;

}