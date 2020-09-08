
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h> 
#include <gmp.h>
#include <unistd.h>
#include <algorithm>
#include <gmpxx.h>
#include <bitset>
#include <sstream>
#include <random>
#include <chrono>

gmp_randclass rr(gmp_randinit_default);


mpz_class generateRandomNumber(mpz_class min, mpz_class max);

mpz_class generateRandomNumber(int bits)
{
	sleep(1);
	rr.seed(time(NULL));

	mpz_class ran;
	ran =rr.get_z_bits(bits);
	return ran;
}

mpz_class power(mpz_class x, mpz_class y, mpz_class p)
{
	mpz_class ret;
	mpz_powm(ret.get_mpz_t(), x.get_mpz_t(), y.get_mpz_t(), p.get_mpz_t());
	return ret;
}


uintmax_t bitCount(mpz_class n)
{
	uintmax_t count = 0;
	while(n)
	{
		count++;
		n >>= 1;
	}
	return count;
}

bool millerTest(mpz_class n, mpz_class d)
{
	mpz_class a = generateRandomNumber(2, n-2);
	mpz_class x = power(a,d,n);
	if(x == 1 || x == n-1) return true;

	while(d != n-1)
	{
		x = (x*x) % n;
		d *= 2;
		if (x == 1) return false;
		if (x == n-1) return true;
	}
	return false;
}


bool isPrime(mpz_class n, uintmax_t k)
{
	if(n <= 3)
	{
		return n > 1;
	}
	else if(n % 2 == 0 || n % 3 == 0)
	{
		return false;
	}

	mpz_class d = n - 1;
	while(d % 2 == 0)
	{
		d /= 2;
	}

	for(int i = 0; i < k; i++)
	{
		if(millerTest(n, d) == false) return false;
	}
	return true;
}

mpz_class generateRandomNumber(mpz_class min, mpz_class max)
{


	return rand() % (max - min + 1) + min;
}


mpz_class generateRandomPrime(int bits)
{
	mpz_class num = generateRandomNumber(bits);//pow(2, bits-1), pow(2, bits));
	mpz_setbit(num.get_mpz_t(), bits-1);
	mpz_setbit(num.get_mpz_t(), bits-2);
	mpz_setbit(num.get_mpz_t(), 0);
	while(true)
	{
		if(isPrime(num, 4)) return num;
		else num +=2;
	}
}

mpz_class modInv(mpz_class a, mpz_class m) 
{ 
    mpz_class m0 = m; 
    mpz_class y = 0, x = 1; 
  
    if (m == 1) 
      return 0; 
  
    while (a > 1) 
    { 
        // q is quotient 
        mpz_class q = a / m; 
        mpz_class t = m; 
  
        // m is remainder now, process same as 
        // Euclid's algo 
        m = a % m, a = t; 
        t = y; 
  
        // Update y and x 
        y = x - q * y; 
        x = t; 
    } 
  
    // Make x positive 
    if (x < 0) 
       x += m0; 
  
    return x; 
} 

void swap(uintmax_t &a, uintmax_t &b)
{
	uintmax_t temp = a;
	a = b;
	b = temp;
}

bool generateKeys(mpz_class& N, int& e, mpz_class& d)
{
	int k = 4096;
	e = 65537;
	mpz_class p;
	mpz_class q;
	std::cout << "Generating P..\n";
	while(true)
	{
		p = generateRandomPrime(k/2);
		if(p % e != 1) break;
		else std::cout << "P mod e is equal to one, getting new one\n";
	}
	std::cout << "P: " << p << " Num of bits: " << bitCount(p) << "\n";
 
 	std::cout << "Generating Q..\n";
	while(true)
	{
		q = generateRandomPrime(k - k/2);
		if(q % e != 1) break;
		else std::cout << "Q mod e is equal to one, getting new one\n";
	}

	std::cout << "Q: " << q << " Num of bits: " << bitCount(q) << "\n";

	if(p < q)
	{
		swap(p, q);
		std::cout << "SWAPPING P AND Q";
	} 
	else if(p == q) 
	{
		std::cout << "P equals Q, returning false";
		return false;
	}

	N = p * q;
	mpz_class L = (p-1)*(q-1);
	d = modInv(e, L);

	return true;
}

mpz_class encrypt(mpz_class m, mpz_class n, mpz_class e)
{
	return power(m, e, n);
}

mpz_class decrypt(mpz_class m, mpz_class n, mpz_class d)
{
	return power(m, d, n);
}

//PKCS#1V1.5 padding scheme
mpz_class Pad(std::string message, int nBitLen)
{
	std::string binaryMessage = "";
	int keyLen = (nBitLen);
	if(message.size()*8 > keyLen - 11*3) std::cout << "To long message";

	int pLength = keyLen - message.size()*8 - 3*8;
	binaryMessage += "00000000";
	binaryMessage += "00000010";
	int counter = 0;
	while(binaryMessage.size() < (pLength + 2*8))
	{
		std::string pString = generateRandomNumber(1, pow(2, 8)-1).get_str(2);
		while (pString.size() < 8) pString.insert(0, "0");

		binaryMessage += pString;
	}

	binaryMessage += "00000000";

	for(char c : message)
	{
		binaryMessage += std::bitset<8>(c).to_string();
	}

	mpz_class padded(binaryMessage, 2);
	return padded;
}

std::string DePad(mpz_class input, int nBitLen)
{
	std::string data = input.get_str(2);
	data.insert(0, "00000000000000");
	data.erase(0, 16);

	std::stringstream sstream(data);
	std::string output;
	bool paddingOver = false;
	while(sstream.good())
	{
		std::bitset<8> bits;
		sstream >> bits;
		unsigned long i = bits.to_ulong();
		if(i == 0 && !paddingOver)
		{
			paddingOver = true;
			continue;
		}

		unsigned char c = static_cast<unsigned char>(i);
		if(paddingOver && c != 0x00)
		{
			output += c;
		}
	}
	return output;
}

void getReadyKeys(mpz_class& N, int& e, mpz_class& d)
{
	N = "101120238836005269809773416981882641266011205065504744716304420651224630140594291762148518493854239011880064057427658763683069244958109350152226702958002371114234952921016860995990043774353171458935500523070383252955438143535556571717694424352773127556622187567084712761119239689462186905646479992413085842411";
	e = 65537;
	d = "74002912782893399920905025028381757486617169192238560905799664666891003720084584608635843020007583677736112918234666976269395412159251120008561532070001812087569020677442476043083968347381461773737724418404030517296607695887555864802079862117049479261141264570098470834533841988043307879465566277369962213265";
}

int main()
{



	std::cout << "\nGenerating Keys\n";

	mpz_class N, d;
	int e;

	while(!generateKeys(N, e, d));

	//getReadyKeys(N, e, d);







	//std::cout << "N: " << N << " Bits: " << bitCount(N) << "\n";
	//std::cout << "e: " << e << " Bits: " << bitCount(e) << "\n";
	//std::cout << "d: " << d << " Bits: " << bitCount(d) << "\n";

	std::string message = "This is a RSA encrypted message, it probably won't be readable in the encrypted format";
	mpz_class intMessage = Pad(message, bitCount(N));
	//mpz_class intMessage = stringToInt(message);
	
	std::cout << "Original message: " << message << "\n\n";
	std::cout << "Converted to int is: " << intMessage << "\n\n";
	mpz_class encrypted = encrypt(intMessage, N, e);
	std::cout << intMessage << " encrypted with Public is: " << encrypted << "\n\n";

	mpz_class decrypted = decrypt(encrypted, N, d);
	std::cout << encrypted << " decrypted with Private is: " << decrypted << "\n\n";
	std::cout << "Decrypted to String is: \n" << DePad(decrypted, bitCount(N)) << "\n\n";

	/*mpz_class encrypted2 = encrypt(intMessage, N, d);
	std::cout << intMessage << " encrypted with Private is: " << encrypted2 << "\n";

	mpz_class decrypted2 = decrypt(encrypted2, N, e);
	std::cout << encrypted2 << " decrypted with Public is: " << decrypted2 << "\n";
*/
}