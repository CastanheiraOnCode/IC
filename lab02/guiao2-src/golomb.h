#include <string>
#include <math.h>
using namespace std;


string unaryEnc(int q){
	string res = "";
	if(q == 0){
      	res = "0";
	}else{
		for(int i = 0; i < q; i++){
			res.push_back('1');
		}
		res.push_back('0');
    }
	return res;
}

int toDec(long n) {
  int dec = 0, i = 0, rem;

  while (n!=0) {
    rem = n % 10;
    n /= 10;
    dec += rem * pow(2, i);
    ++i;
  }

  return dec;
}

string toBin(int decimal, int val){
	long binary = 0, remainder, product = 1;
	while (decimal != 0) {
			remainder = decimal % 2;
			binary = binary + (remainder * product);
			decimal = decimal / 2;
			product *= 10;
		}

	string res  = to_string(binary);
	int resLength = res.length();

	uint32_t val_lui = val;

	if (val > resLength){

		while(res.length() < val_lui){
	 		res.insert(0,"0");
		}
	}

	return res;
}

string truncBinEnc(int r, int b, int M){
	string remainderCode;


	if(r < pow(2,b+1) - M){

		remainderCode = toBin(r,b);	
			
	}else{
		remainderCode = toBin(r + pow(2,b+1) - M,b+1);
	}	

	return remainderCode;

}

int decodeUnary(string code){
	int ret = 0;

	for(uint32_t i = 0; i < code.length(); i++){
		if(code[i] == '1'){
			ret++;
		}else{
			break;
		}
	}
	return ret;
}

int decodeRemainder(string code, int q, int b, int M){

	string r2 = code.substr(q+1,b);

	int decimal = toDec(stol(r2));
	int r;
	if (decimal < (pow(2,b+1) - M)){
		r = decimal;
	}else{
		r2  = code.substr(q+1,b+1);
		decimal = toDec(stol(r2));
		r = decimal - pow(2,b+1) + M;
	}

	return r;
}


class Golomb 
{      
	public:             // Access specifier
		int M;


	Golomb(int input){
    
    	M = input; 
	}

	string encode(int N){
		if (N<0){
			N =(-2*N)-1;
		}else{
			N = 2 * N;
		}
		string code;
		int quotient = floor(N/M);
		int remainder = N % M;

    	string quotientCode = unaryEnc(quotient); 

		int b = floor(log2(M));

		string remainderCode = truncBinEnc(remainder,b,M);
		code = quotientCode + remainderCode;

		return code;
    }

	int decode(string code){
		
		//string m = code.substr(0,17);

		
		int q  = decodeUnary(code);
		int b  = floor(log2(M));
		int N;

		//to skip the 0 delimiter of the golomb coding and have the string ready for the next computation
		int r = decodeRemainder(code, q, b, M);

		N = q * M + r;
		
		if(N % 2 != 0){
			N = floor((N/-2) -1);
		}else{
			N  = N/2;
		}

		return N;
	}

	int valid(int x, int y){
		if(x == y){
			return 1;
		}else{
			return 0;
		}

	}


};

