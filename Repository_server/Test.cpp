#include "Test1.h"
#include <iostream>
#include <iostream>
int Test::One::method3() {
	try {
		int i = 0;
		std::cout << i;
	}
	catch (std::exception& ex)
	{
		std::cout << "\n\n  " << ex.what() << "\n\n";
		return 0;
	}
}

//----< To test lambda and functors >--------------------------------
void File::lambda_funct()
{
	std::function<std::string(size_t, const std::string&)> f;
	f = [](size_t size, const std::string& msg) ->std::string { return testFunction(size, msg); };
	std::cout << f(__LINE__, "via lambda");
}

//----< open for reading or writing >--------------------------------------
bool File::method1(direction dirn, type typ)
{
	dirn_ = dirn;
	typ_ = typ;
	if (dirn == in)
	{
		pIStream = new std::ifstream;
		if (typ == binary)
		{
			pIStream->open(name_.c_str(), std::ios::in | std::ios::binary);
		}
		else{
			pIStream->open(name_.c_str(), std::ios::in);
		}
		if (!(*pIStream))
			good_ = false;
	}
	else
	{
		pOStream = new std::ofstream;
		if (typ == binary) {
			pOStream->open(name_.c_str(), std::ios::out | std::ios::binary);
		}
		else {
			pOStream->open(name_.c_str(), std::ios::out);
		}
		if (!(*pOStream)) {
			good_ = false;
		}
		//throw std::runtime_error("\n  open for output failed in File constructor");
	}
	return good_;
}

//----< reads a block of bytes from binary file >--------------------------
Block File::getBlock(size_t size)
{
	if (pIStream == 0 || !pIStream->good())
		throw std::runtime_error("input stream not open");
	if (typ_ != binary)
		throw std::runtime_error("reading binary from text file");
	if (dirn_ == out)
		throw std::runtime_error("reading output file");
	Block blk;
	if (pIStream)
	{
		for (size_t i = 0; i<size; ++i)
		{
			Byte b;
			pIStream->get(b);
			if (pIStream->good())
				blk.push_back(b);
			else
				break;
		}
	}
	return blk;
}
//----< tests for error free stream state >--------------------------------

bool File::isGood()
{
	if (!good_) {
		return false;
	}
	if (pIStream)
	{
		try {
			return (good_ = pIStream->good());
		}
		catch (std::exception& ex)
		{
			std::cout << "\n\n  " << ex.what() << "\n\n";
			return 0;
		}
	}
	return (good_ = false);
}
//----< flushes output stream to its file >--------------------------------
void File::flush()
{
	if (pOStream)
		pOStream->flush();
	if (pOStream)
		return;
}
