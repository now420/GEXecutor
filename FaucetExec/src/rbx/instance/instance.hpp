#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace rbx {
	class instance_t {
	public:
		uintptr_t address;

		void SetBytecode(std::vector<char> bytes, int bytecode_size);
		void GetBytecode(std::vector<char>& bytecode, size_t& bytecode_size);
		void modulebypassi();

		std::vector<rbx::instance_t> getchildren();
		rbx::instance_t waitfor_child(std::string name, int timeout = 5);
		rbx::instance_t findfirstchild(std::string name);

		rbx::instance_t ObjectValue();

		void SetBoolValue(bool rizz);

		std::string getname();
		std::string getclassname();
		bool IsA(std::string tocheck);
	};
}