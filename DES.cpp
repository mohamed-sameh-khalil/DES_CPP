// DES.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include<vector>
#include<algorithm>
#include"TYPES.h"
#include"DEVICES.h"
using namespace std;

vbb ENCKEY , DECKEY;//a list with 16 encryption or decryption keys

/*
This function takes in a vector of bools and left-shift it in a circle
according to the amount
Left shift means bit at index 0(msb) becomes at index n(lsb)
*/
void circularSL(vb &x, int shiftAmount) {
	for (int i = 0; i < shiftAmount; i++)
		x.push_back(x[i]);
	x.erase(x.begin(), x.begin() + shiftAmount);
}
/*
This function takes in a unsigned long long and gives a vector
of booleans with every bit
the msb is in index 0
*/
void LLtoVB(ll n, vb& ans, int size) {
	ans.clear();
	ans.resize(size);
	for (int i = 0; i < size; i++) {//MSB at index 0
		ans[size - i - 1] = (n & 1);
		n >>= 1;
	}
}
/*
This function takes in a vector of bits and gives an unsigned long long represting them
*/
ll VBtoLL(vb& n) {
	ll ans = 0;
	for (int i = 0; i < n.size(); i++) {//msb at index 0
		ans <<= 1;//shift the number first to make room for new bit
		ans |= n[i];
	}
	return ans;
}

/*
This function takes in a vector of bits and returns 2 vectors
each carrying one half of the original
*/
void splitvb(const vb& orig, vb& left, vb& right) {
	right.clear(), left.clear();
	left.insert(left.begin(), orig.begin(), orig.begin() + orig.size() / 2);
	right.insert(right.begin(), orig.begin() + orig.size() / 2, orig.end());
}

void mixvb(const vb& left, const vb& right, vb& mix) {
	mix.clear();
	mix.insert(mix.end(), left.begin(), left.end());
	mix.insert(mix.end(), right.begin(), right.end());

}

/*
This function takes ina  bool vector and a permutation vector and an output vector
it outputs the result of the permutation in the output
input and output can be the same vector and the function will work
*/
void permute(const vb& input, const vi& perm, vb& output) {
	vb ans;
	ans.reserve(input.size());
	for (auto i : perm) {//push in answer according to permuter vector
		ans.push_back(input[i - 1]);
	}
	output = ans;
}
/*
unit test for the permute function
*/
int permuteTest(const vb& input = vb(), const vb& output = vb(), const vi& perm = permc1) {

	for (int i = 0; i < output.size(); i++) {
		if (input[perm[i] - 1] != output[i]) {
			cout << "permute not working";
			return -1;
		}
	}
	return 0;
}
void printkey(const vb& key) {
	for (int i = 0; i < key.size(); i++)
		cout << key[i];
	cout << endl;
}

void makekeys(ll mainkey, int rounds = 16) {
	ENCKEY.clear();
	DECKEY.clear();
	vb init; LLtoVB(mainkey, init,64);//init size now is 64
	permute(init, permc1, init);//init size now is 56
	vb left, right;
	splitvb(init, left, right);// init is now split into left and right
	for(int i = 0; i < rounds; i++){
		circularSL(left, shiftTable[i]);//shift left half according to table
		circularSL(right, shiftTable[i]);//shift right half according to table
		vb curKey;
		mixvb(left, right, curKey);// left and right half are mixed to make current round
		permute(curKey, permc2, curKey);//pc 2 result is not saved in init
		//permuteTest(init, curKey, permc2);//test that permutation was valid
		ENCKEY.push_back(curKey);
	}
	for (auto i = ENCKEY.rbegin(); i != ENCKEY.rend(); i++)
		DECKEY.push_back(*i);
}

void VBxor(const vb& a, const vb& b, vb& output) {
	vb ans;
	int i = 0;
	auto mn = min(a.size(), b.size());
	for (; i < mn; i++) {
		ans.push_back(a[i] ^ b[i]);
	}
	for (; i < a.size(); i++)
		ans.push_back(a[i]);
	for (; i < b.size(); i++)
		ans.push_back(b[i]);
	output = ans;
}
/*
This function does the work of the 8 S-boxes
input and output can be the same vector and it will still work
*/
void sboxer(const vb& input, vb& output) {
	_ASSERT(input.size() == 48);
	vb ans;
	ans.reserve(32);
	vb tmp;
	for (int i = 0; i < 48; i+=6) {// this loop loops 8 times (48 / 6)
		int ctrl = input[i] * 2 + input[i + 5];
		int val = input[i + 1] * 8 + input[i + 2] * 4 + input[i + 3] * 2 + input[i + 4];
		LLtoVB(SBOX[i / 6][ctrl][val], tmp, 4);

		for (int j = 0; j < 4; j++)//loop happens 8 times so ans has 8 * 4 32 elements
			ans.push_back(tmp[j]);
	}
	output = ans;
}
/*
This is the F function.
it takes in the right part of the plain text and the round key and outpute a 32 bit answer
*/
void F(const vb& right, const vb& key, vb& output) {
	_ASSERT(right.size() == 32);
	_ASSERT(key.size() == 48);
	vb ans;
	// Expansion permutation
	permute(right, perme, ans);//ans has the permutation of right
	_ASSERT(ans.size() == 48);

	// XOR between KEY and right after expansion
	VBxor(ans, key, ans);// ans has the xor between itself and key
	_ASSERT(ans.size() == 48);

	// 8 SBOX for key xor right
	sboxer(ans, ans);//ans is now 32 bits due to sbox
	_ASSERT(ans.size() == 32);

	// P permutation
	permute(ans, permp, ans);

	output = ans;
}

/*
This function simulates a round the plain text takes
righto refers to right output
lefto refers to left output
right and righto can be the same vector
left and lefto and be the same vector
*/
void passround(const vb& left, const vb& right, const vb& key, vb& righto, vb& lefto) {
	_ASSERT(left.size() == 32 && right.size() == 32 && key.size() == 48);
	vb lefttmp = right, righttmp;
	F(right, key, righttmp);
	VBxor(left, righttmp, righttmp);
	righto = righttmp, lefto = lefttmp;
}

/*
This function does the encryption and decryption in des
*/
void DES(const vb& input, vb& output, bool encrypt, int rounds = 16) {
	vbb keys = encrypt ? ENCKEY : DECKEY;
	_ASSERT(input.size() == 64);
	vb tmpinput = input;

	permute(tmpinput, IP, tmpinput);
	_ASSERT(tmpinput.size() == 64);

	vb left, right;

	for (int i = 0; i < tmpinput.size() / 2; i++)left.push_back(tmpinput[i]);
	for (int i = tmpinput.size() / 2; i < tmpinput.size(); i++)right.push_back(tmpinput[i]);
	_ASSERT(left.size() == right.size());

	for (int i = 0; i < rounds; i++) //16 rounds
		passround(left, right, keys[i], right, left);
	output.clear();

	//push right part first to apply the (round+1)th swap
	for (auto i : right) output.push_back(i);
	for (auto i : left) output.push_back(i);
	permute(output, IIP, output);
}

//a wrapper for the real function to take ll directly
ll DES(ll p, bool encrypt) {
	vb plain;
	LLtoVB(p, plain, 64);
	vb ans;
	DES(plain, ans, encrypt);
	return VBtoLL(ans);
}
int main(){
	int choice;
	while (1) {
		cout << "enter 0 for decryption and any other number for encryption" << endl;
		cin >> choice;
		ll key, plain, count;
		cout << "please enter the key in HEX:\n";
		cin >> hex >> key;
		makekeys(key);
		cout << "please enter the " << ((choice)? "plain":"cipher")<< " text in HEX:\n";
		cin >> hex >> plain;
		cout << "please enter the number " << ((choice) ? "en" : "de") << "cryptions required:\n";
		cin >> count;
		while (count-- > 0) {
			plain = DES(plain, choice);
		}
		cout << hex << plain << endl << endl;
	}
}
