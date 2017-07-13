#ifndef TEST1_H
#define TEST1_H

#include <iostream>
#include <vector>
#include <map>

namespace Test {
	struct element {
		std::string name,type;
		int start_line, end_line;
		std::vector<element> children;
	};
	enum Numbers{one=1,two,three,four};
	class One {
	public:
		One() {};
		~One() {};
		void inlineMethod(int a) {
			int i;
			for (i = 0; i < 2; i++) {
				std::cout << a;
				if (i == 1)
				{
					std::cout << i;
				}
			}
			if (i == 1) {
				std::cout << a;
			}
			else
			{
			}
		}
		int method1();
		int method3();
		int method2()
		{
			std::cout << "Hello OOD";
		}
		int method4();
	};
	
int One::method4() { return -1; }

	
	static void globalFunction() {
		std::map<int, int> map = {{1,1},{2,2}};
		while (true) {
			{
				std::cout << "\n Global Function";
			}
			break;
		}
	}
}

static void globalFunction2() {
	std::map<int, int> map = { { 1,1 },{ 2,2 } };
	while (true) {
		{
			std::cout << "\n Global Function";
		}
		break;
	}
}




#endif
