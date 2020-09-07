#pragma once
#include <vector>
#include <iostream>

// A BigInt for use in project euler challenges
struct BigInt {
	//// Constructors
	BigInt() { segments = std::vector<unsigned>(0); sign = 0; };
	BigInt(std::vector<unsigned> segments, bool sign) : segments(segments), sign(sign) {};
	//// Base 10 string constructor
	BigInt(std::string str);

	// The segments of this BigInt. segments[N] = bits[(N-1)*32:N*32-1]
	std::vector<unsigned> segments;

	// The sign of the bigint, 0 = positive, 1 = -'ve.
	bool sign;

	//// Functions
	// Simple printing:
	void simplePrint();
	
	// Base 10 printing
	void base10Print();

	//// OPERATORS
	// Equality
	bool operator==(const BigInt& b) const;
	bool operator!=(const BigInt& b) const;

	// Comparison
	bool operator>(const BigInt& b) const;
	bool operator<(const BigInt& b) const;		

	// Add together two BigInts
	BigInt operator+(const BigInt& b) const;
	
	// Subtract two BigInts from eachother
	BigInt operator-(BigInt& b) const;

	// Multiply together two BigInts
	BigInt operator*(const BigInt& b) const;

	// Integer division of BigInts
	BigInt operator/(const BigInt& b) const;

	// Modulus of Bigints
	BigInt operator%(const BigInt& b) const;
};

////// Utility functions for implementation
//// Overflow resolution
void OverflowResolution(std::vector<unsigned> &segments, std::vector<int> &overflows) {
	// Dealing with the overflow:
	int end = overflows.size() - 1;

	// OPTIMIZE THIS
	while (end != -1) {
		int temp_end = -1;

		// Looping through the overflow additions
		for (int i = 0; i <= end; i += 1) {
			#if __has_builtin(__builtin_uadd_overflow)
			if (__builtin_uadd_overflow(segments[overflows[i] + 1], 1, &segments[overflows[i] + 1])) {
				temp_end += 1;
				overflows.insert(overflows.begin() + temp_end, overflows[i] + 1);
			}
			#else
			// TODO
			#endif
		}

		end = temp_end;
	}
	return;
}

// Useful for both DSA entry_points
inline void RemoveZeroes(std::vector<unsigned> &segments) {
	// Removing any trailing zeros from the ret value
	int i = segments.size() - 1;
	while (segments[i] == 0) {
		segments.pop_back();
		i -= 1;
	}
}

//// Base 10 string constructor
BigInt::BigInt(std::string str) {
	// Is this int negative?
	sign = str[0] == '-';

	//// Constructing the actual number
	int num_digits = str.size() - sign;
	std::vector<std::vector<unsigned>> powers(0);	// Powers of ten written in base 2^32-1
	powers.push_back(std::vector<unsigned>{1});	// 10^0 = 1
	
	// Computing these powers
	for (int i = 0; i < num_digits - 1; i += 1) {
		// Computing the i+1th power
		std::vector<unsigned> cur_power = powers[i];	// We start with the last power
		cur_power.push_back(0);							// It's impossible for multiplying by 10 to create >= 2 new segments
		std::vector<int> overflows(0);					// Potential overflows during our computation
		for (int j = 0; j < cur_power.size(); j += 1) {
			if (cur_power[j] <= 429496729) {			// 429496729.5 == (2^32-1)/10
				cur_power[j] *= 10;
			}
			else {										// This can be overkill, but let's just add it to itself 10 times
				unsigned temp = cur_power[j];
				for (int k = 0; k < 10; k += 1) {
					#if __has_builtin(__builtin_uadd_overflow)
					if (__builtin_uadd_overflow(cur_power[j], temp, &cur_power[j])) {
						overflows.push_back(j);
					}
					#else
					// TODO
					#endif
				}
			}
		}

		// Dealing with overflows:
		OverflowResolution(cur_power, overflows);
		RemoveZeroes(cur_power);
		powers.push_back(std::move(cur_power));
	}

	// Now we've computed these powers, let's add multiplied versions of them to return
	segments = std::vector<unsigned>(powers[powers.size() - 1].size()+1);
	std::vector<int> overflows(0);
	for (int i = 0; i < powers.size(); i += 1) {
		// Loop over ever segment in powers[i], add it to ret_segment[i], and deal with overflows:
		for (int j = 0; j < powers[i].size(); j += 1) {
			for (int k = 0; k < str[str.size()-i-1] - '0'; k += 1) {
				#if __has_builtin(__builtin_uadd_overflow)
				if (__builtin_uadd_overflow(segments[j], powers[i][j], &segments[j])) {
					overflows.push_back(j);
				}
				#else
				// TODO
				#endif
			}
		}
	}

	OverflowResolution(segments, overflows);
	RemoveZeroes(segments);
}

//// Function implementations
// Simple printing function
void BigInt::simplePrint() {
	for (int i = 0; i < this->segments.size(); i += 1) {
		std::cout << this->segments[i];
		if (i != this->segments.size() - 1) {
			std::cout << ",";
		}
	}
	std::cout << "\n";
	return;
}

// Base 10
void BigInt::base10Print() {
	
}

//// Operator implementations
// Equality
bool BigInt::operator==(const BigInt& b) const {
	return (this->sign == b.sign && this->segments == b.segments);	// Both the sign *and* segments must be equal
}
bool BigInt::operator!=(const BigInt& b) const {
	return !operator==(b);
}

// Comparison
// Is the segment of a bigger than the segment of b?
bool SegmentAbove(const BigInt& a, const BigInt& b) {
	// First, if one is longer that will determine which is larger
	// Check which is larger, this depends on their sign.
	if (a.segments.size() > b.segments.size()) {
		return 1;
	}
	else if (a.segments.size() < b.segments.size()) {
		return 0;
	}

	// If neither segment is longer, we loop through each segment until one is shorter.
	for (int i = a.segments.size()-1; i >= 0; i -= 1) {
		if (a.segments[i] > b.segments[i]) {
			return 1;
		}
		else if (a.segments[i] < b.segments[i]) {
			return 0;
		}
	}

	// Looks like none differed
	return false;
}

bool BigInt::operator>(const BigInt& b) const {
	//// Are the signs different?
	if (this->sign != b.sign) {
		return this->sign < b.sign;
	}

	//// Otherwise, let's compare their segments
	bool seg_res = SegmentAbove(*this, b);
	if (seg_res) {
		return this->sign == 0;
	}
	return this->sign == 1;
}

bool BigInt::operator<(const BigInt& b) const {
	return b > (*this);
}

// Addition / Subtraction implementation functions
// Same sign addition + subtraction
BigInt SameSignAddition(const BigInt& a, const BigInt& b, bool sign) {		// REALLY needs optimization
	// Ensures that a has at least as many segments as b
	if (b.segments.size() > a.segments.size()) {
		return SameSignAddition(b, a, sign);
	}

	// Making a new BigInt object
	BigInt ret{ std::vector<unsigned int>(0), sign };
	ret.segments = a.segments;
	ret.segments.push_back(0);

	// Overflow vector
	std::vector<int> overflows = std::vector<int>(0);

	// Initial addition
	for (int i = 0; i < b.segments.size(); i += 1) {
		#if __has_builtin(__builtin_uadd_overflow)
		if (__builtin_uadd_overflow(ret.segments[i], b.segments[i], &ret.segments[i])) {
			overflows.push_back(i);
		}
		#else
		// TODO
		#endif
	}

	// Fixing the overflows
	OverflowResolution(ret.segments, overflows);

	// Do we need to get rid of the extra int?
	if (ret.segments[ret.segments.size() - 1] == 0) {
		ret.segments.pop_back();
	}

	return ret;
}


// runs the actual addition algorithm, but needs a guarentee that |a| >= |b|
BigInt DifferingSignAddition(const BigInt& a, const BigInt& b, bool sign) {
	// Creating the returned BigInt
	BigInt ret{
		std::vector<unsigned>(0), sign
	};
	ret.segments = a.segments;

	// Adding with different signs in the same as subtraction. So we're going to intialize ret with a, and subtract b.
	// Initial subtraction
	std::vector<int> try_again(0);
	for (int i = 0; i < b.segments.size(); i += 1) {
		if (ret.segments[i] < b.segments[i]) {
			try_again.push_back(i);
		}
		else {
			ret.segments[i] = ret.segments[i] - b.segments[i];
		}
	}

	if (try_again.size() == 0) {
		return ret;
	}

	//// Borrowing and doing the actual subtraction
	// Constructing the borrowing vector
	// First, let's figure out where we can borrowing from
	int borrowed_from = try_again.size();
	while (ret.segments[borrowed_from] != 0) {
		borrowed_from += 1;
	}
	ret.segments[borrowed_from] -= 1;

	// Then creating a borrowing vector for that:
	std::vector<unsigned> borrowing(borrowed_from - 1, 4294967295);

	// Actually doing the borrowing
	for (int i : try_again) {
		borrowing[i] -= b.segments[i];
	}

	// Adding 1 to the borrowing matrix, then adding it to ret
	for (int i = 0; i < try_again[0]; i += 1) {
		borrowing[i] = 0;
	}
	borrowing[try_again[0]] += 1;
	ret = ret + BigInt{ borrowing, ret.sign };

	return ret;
}



// Entrypoint for differing sign addition for adding integers
static BigInt DSA_AddEntry(const BigInt& a, const BigInt& b) {
	// Run f in a different order depending on which is larger in terms of segment size
	BigInt ret;
	if (SegmentAbove(a, b)) {
		ret = DifferingSignAddition(a, b, a.sign);
	}
	else {
		ret = DifferingSignAddition(b, a, b.sign);
	}

	RemoveZeroes(ret.segments);
	return(ret);
}

// Entrypoint for differing sign addition for subtracting integers
static BigInt DSA_SubEntry(const BigInt& a, const BigInt& b) {
	// Run f in a different order depending on which is larger in terms of segment size
	BigInt ret;
	if (SegmentAbove(a, b)) {
		ret = DifferingSignAddition(a, b, a.sign);
	}
	else {
		ret = DifferingSignAddition(b, a, a.sign^1);
	}

	RemoveZeroes(ret.segments);
	return(ret);
}

// Addition
BigInt BigInt::operator+(const BigInt& b) const {
	// Are the signs the same?
	if (this->sign == b.sign) {
		return SameSignAddition(*this, b, b.sign);
	}

	// Otherwise we run the differing signs version
	return DSA_AddEntry(*this, b);
}

// Subtraction
BigInt BigInt::operator-(BigInt& b) const {
	// The algorithm we use depends on the signs
	if (this->sign == 0 && b.sign == 1) {	// + - -
		return SameSignAddition(*this, b, 0);
	}
	else if (this->sign == 1 && b.sign == 0) {	// - - +
		return SameSignAddition(*this, b, 1);
	}

	// Otherwise, let's use the differing signs addition function
	return DSA_AddEntry(*this, b);
}

