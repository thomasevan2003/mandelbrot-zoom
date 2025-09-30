#include "fixedpoint1024.hpp"

#include <cstring>
#include <iostream>
#include <vector>

fixedpoint1024::fixedpoint1024() {
	memset((void*)this, 0, total_bits/8);
}

fixedpoint1024::fixedpoint1024(double x) {
	
	memset((void*)this, 0, total_bits/8);
	
	if (x == 0.0) {
		return;
	}
	
	// extract sign bit
	uint64_t sign;
	memcpy((void*)&sign, (const void*)&x, sizeof(double)); 
	header.flags.sign = (sign >> 63) & 1;
	
	// extract exponent
	uint64_t exponent_biased;
	memcpy((void*)&exponent_biased, (const void*)&x, sizeof(double));
	exponent_biased = ((exponent_biased >> 52) & 0x7FFull);
	int exponent = static_cast<int>(exponent_biased) - 1023;
	
	// above overflow value - set to inf
	if (exponent >= point_position) {
		header.flags.inf = true;
		return;
	}
	// below underflow value - set to 0
	if (exponent <= point_position - value_bits) {
		memcpy((void*)this, 0, total_bits/8);
		return;
	}		
	
	// extract fractional component
	uint64_t fraction_left_aligned;
	memcpy((void*)&fraction_left_aligned, (const void*)&x, sizeof(double));
	fraction_left_aligned = fraction_left_aligned << 12;
	uint64_t fraction = fraction_left_aligned >> 12;
		
	// assign implicit 1 in largest position
	int largest_nonzero_bit = point_position - exponent - 1; // index of largest nonzero bit
	int largest_nonzero_block = largest_nonzero_bit / 32;    // index of largest nonzero block
	int largest_nonzero_leftshift_in_block = (largest_nonzero_block+1)*32 - largest_nonzero_bit - 1; // offset of largest nonzero bit from right side of its block
	value[largest_nonzero_block] = (0x1ull <<  largest_nonzero_leftshift_in_block);
	
	// assign fractional component
	int first_fractional_bit = largest_nonzero_bit + 1; // index of first bit in fractional component
	int last_fractional_bit = largest_nonzero_bit + 52; // index of last bit in fractional component
	int first_fractional_block = first_fractional_bit / 32; // index of block of first bit in fractional component
	int second_fractional_block = first_fractional_block + 1; // index of second block spanned by the fraction
	int last_fractional_block = last_fractional_bit / 32; // index of last block spanned by the fraction (could be second or third);
	int first_fractional_bit_rightshift_in_block = first_fractional_bit - first_fractional_block*32; // offset of first bit of fractional component from left side of its block 
	uint32_t fraction_first_block_value = static_cast<uint32_t>(fraction_left_aligned >> 32 >> first_fractional_bit_rightshift_in_block); // first block spanned by fraction
	uint32_t fraction_second_block_value = static_cast<uint32_t>((fraction_left_aligned >> first_fractional_bit_rightshift_in_block) & 0xffffffffull); // second block spanned by fraction
	if (first_fractional_block < num_blocks) {
		value[first_fractional_block] = value[first_fractional_block] | fraction_first_block_value;
	}
	if (second_fractional_block < num_blocks) {
		value[second_fractional_block] = value[second_fractional_block] | fraction_second_block_value;
	}
	if (last_fractional_block > second_fractional_block) {
		uint32_t fraction_last_block_value = static_cast<uint32_t>(((fraction_left_aligned << 32) >> first_fractional_bit_rightshift_in_block) & 0xffffffffull); // last block spanned by fraction
		value[last_fractional_block] = value[last_fractional_block] | fraction_last_block_value;
	}
}

fixedpoint1024::fixedpoint1024(std::string s) {
	
	memset((void*)this, 0, total_bits/8);
	
	// set sign bit if present
	int integer_position = 0;
	if (s[0] == '+') {
		header.flags.sign = false;
		integer_position = 1;
	} else if (s[0] == '-') {
		header.flags.sign = true;
		integer_position = 1;
	}
	
	// get positions of integer and fraction
	int point_position = integer_position;
	bool point_found = false;
	bool integer_present = false;
	bool fraction_present = false;
	for (int i = integer_position; i < s.length(); ++i) {
		if (s[i] == '.') {
			point_position = i;
			point_found = true;
			if (i > integer_position) {
				integer_present = true;
			}
			if (i < s.length()-1) {
				fraction_present = true;
			}
			break;
		}
	}
	
	// assign integer component
	// loop through each digit and scale by powers of 10
	int integer_width = point_position - integer_position;
	uint32_t integer = 0;
	uint32_t digit_magnitude = 1;
	for (int i = 0; i < integer_width; ++i) {
		char digit_char = s[point_position-1-i];
		uint32_t digit_value = static_cast<uint32_t>(digit_char - '0');
		integer += digit_value*digit_magnitude;
		digit_magnitude *= 10;
	}
	value[0] = integer;
	
	// assign fractional component
	// multiply fraction in decimal by 2 repeatedly and extract integer each iteration
	int fraction_width = s.length() - point_position - 1;
	std::vector<uint32_t> fraction_multiples;
	fraction_multiples.resize(fraction_width+1);
	for (int i = 0; i < fraction_width; ++i) {
		char digit_char = s[point_position+i+1];
		uint32_t digit_value = static_cast<uint32_t>(digit_char - '0');
		fraction_multiples[i+1] = digit_value;
	}
	for (int i = 0; i < value_bits - 32; ++i) {
		fraction_multiples[0] = 0;
		for (int j = 1; j < fraction_multiples.size(); ++j) {
			fraction_multiples[j-1] += (fraction_multiples[j]*2)/10;
			fraction_multiples[j] = (fraction_multiples[j]*2)%10;
		}
		if (fraction_multiples[0]) {
			int block = (i/32) + 1;
			int leftshift_in_block = 31 - (i%32);
			value[block] = value[block] | (0x1ul << leftshift_in_block);
		}
	}
	
}

std::string fixedpoint1024::string_binary() {
	if (header.flags.inf) {
		if (header.flags.sign) {
			return "-inf";
		} else {
			return "inf";
		}
	}
	std::string out;
	out.reserve(1100);
	out.push_back(header.flags.sign ? '-' : '+');
	for (int i = 0; i < num_blocks; ++i) {
		for (int j = 0; j < 32; ++j) {
			if (i*32 + j == point_position) {
				out.push_back('.');
			} else if ((i*32 + j - point_position) % 32 == 0) {
				out.push_back(',');
			}
			uint32_t block = value[i];
			bool bit = static_cast<bool>((block >> (31 - j)) & 0x1ull);
			out.push_back(bit ? '1' : '0');
		}
	}
	return out;
}

fixedpoint1024 fixedpoint1024::operator-() const {
	fixedpoint1024 out = *this;
	out.header.flags.sign = !out.header.flags.sign;
	return out;
}

fixedpoint1024 fixedpoint1024::operator+(const fixedpoint1024& other) const {
	fixedpoint1024 out;
	memset((void*)&out, 0, total_bits/8);
	bool first_sign = this->header.flags.sign;
	bool same_sign = first_sign == other.header.flags.sign;
	// if same sign, add from last digit to first, carrying digit as necessary
	if (same_sign) {
		out.header.flags.sign = first_sign;
		uint32_t carry_over = 0x0ul;
		for (int i = num_blocks-1; i >= 0; --i) {
			uint64_t digit_sum = static_cast<uint64_t>(this->value[i]) + static_cast<uint64_t>(other.value[i]) + static_cast<uint64_t>(carry_over);
			carry_over = static_cast<uint32_t>(digit_sum >> 32);
			uint32_t remainder_after_carry = static_cast<uint32_t>(digit_sum);
			out.value[i] = remainder_after_carry;
		}
		if (carry_over) {
			out.header.flags.inf = true;
		}
	} 
	// if opposite signs, subtract from last digit to first, borrowing digit as necessary
	else {
		int64_t carry_over = 0x0l;
		bool other_has_greater_magnitude = false;
		for (int i = 0; i < num_blocks; ++i) {
			if (this->value[i] > other.value[i]) {
				break;
			} else if (this->value[i] < other.value[i]) {
				other_has_greater_magnitude = true;
				break;
			}
		}
		for (int i = num_blocks-1; i >= 0; --i) {
			int64_t digit_difference;
			if (other_has_greater_magnitude) {
				digit_difference = static_cast<int64_t>(other.value[i]) - static_cast<int64_t>(this->value[i]) - carry_over;
			} else {
				digit_difference = static_cast<int64_t>(this->value[i]) - static_cast<int64_t>(other.value[i]) - carry_over;
			}
			if (digit_difference < 0) {
				carry_over = 0x1ll;
				out.value[i] = static_cast<uint32_t>(digit_difference + 0x100000000ll);
			} else {
				carry_over = 0x0ll;
				out.value[i] = static_cast<uint32_t>(digit_difference);
			}
		}
		if (other_has_greater_magnitude) {
			out.header.flags.sign = !first_sign;
		} else {
			out.header.flags.sign = first_sign;
		}
	}
	return out;
}

bool fixedpoint1024::operator>(const fixedpoint1024& other) const {
	// if arguments are opposite signs, positive is greater
	if (this->header.flags.sign != other.header.flags.sign) {
		if (this->header.flags.sign) {
			return false;
		} else {
			return true;
		}
	}
	// arguments are the same sign: compare each digit in order and break out when difference is found
	else {
		bool other_greater_in_magnitude = false;
		for (int i = 0; i < num_blocks; ++i) {
			if (this->value[i] > other.value[i]) {
				break;
			} else if (this->value[i] < other.value[i]) {
				other_greater_in_magnitude = true;
				break;
			}
		}
		if (this->header.flags.sign) {
			return other_greater_in_magnitude;
		} else {
			return !other_greater_in_magnitude;
		}
	}
}

fixedpoint1024 fixedpoint1024::operator*(const fixedpoint1024& other) const {
	
	fixedpoint1024 out;
	memset((void*)&out, 0, total_bits/8);
	
	if (this->header.flags.sign != other.header.flags.sign) {
		out.header.flags.sign = true;
	}
	
	const uint16_t* this_source = reinterpret_cast<const uint16_t*>(&this->value);
	const uint16_t* other_source = reinterpret_cast<const uint16_t*>(other.value);
	uint16_t* out_source = reinterpret_cast<uint16_t*>(out.value);
	
	const int num_half_blocks = num_blocks*2;
	
	uint16_t this_value[num_half_blocks];
	uint16_t other_value[num_half_blocks];
	uint16_t out_value[num_half_blocks];
	
	for (int i = 0; i < num_blocks; ++i) {
		int first = 2*i;
		int second = 2*i+1;
		this_value[first] = this_source[second];
		this_value[second] = this_source[first];
		other_value[first] = other_source[second];
		other_value[second] = other_source[first];
	}
	
	uint64_t overflowed_block = 0x0ull;
	for (int i = num_half_blocks-1; i >= 0; --i) {
		const int minj = std::max(0, i-num_half_blocks+2);
		const int maxj = std::min(num_half_blocks-1, i+1);
		for (int j = minj; j <= maxj; ++j) {
			overflowed_block = overflowed_block + static_cast<uint64_t>(this_value[j])*static_cast<uint64_t>(other_value[i-j+1]);
		}
		out_value[i] = static_cast<uint16_t>(overflowed_block);
		overflowed_block = overflowed_block >> 16;
	}
	
	for (int i = 0; i < num_blocks; ++i){
		int first = 2*i;
		int second = 2*i+1;
		out_source[first] = out_value[second];
		out_source[second] = out_value[first];
	}
	
	if (overflowed_block > 0) {
		out.header.flags.inf = true;
	}
	
	return out;
	
}