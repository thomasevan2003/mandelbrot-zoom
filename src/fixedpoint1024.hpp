#ifndef FIXEDPOINT1024_HPP
#define FIXEDPOINT1024_HPP

#include <cstdint>
#include <string>

struct fixedpoint1024_flags {
	bool sign; // true if negative
	bool inf;
	bool nan;
};
union fixedpoint1024_header {
	uint32_t block;
	fixedpoint1024_flags flags;
};

// 1024 bit fixed point number
// 8 byte header, 120 byte for data
class fixedpoint1024 {
	public:
		
		fixedpoint1024();
		fixedpoint1024(double x);
		fixedpoint1024(std::string s);
		std::string string_binary();
		double truncated_double() const;
		fixedpoint1024 times_2() const;
		fixedpoint1024 operator-() const;
		fixedpoint1024 operator+(const fixedpoint1024& other) const;
		fixedpoint1024 operator*(const fixedpoint1024& other) const;
		bool operator>(const fixedpoint1024& other) const;
	
	private:

		static constexpr int point_position = 32; // position of decimal point (number of bits to the left of the point)
		static constexpr int total_bits = 160; // total size of the class
		static constexpr int value_bits = total_bits - sizeof(fixedpoint1024_header)*8; // size of the value portion of the class
		static constexpr int num_blocks = value_bits/32; // number of 32 bit blocks

		fixedpoint1024_header header;
		uint32_t value[num_blocks];
	
};

#endif