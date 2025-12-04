#ifndef RBINTERPRETER_H
#define RBINTERPRETER_H

#include <godot_cpp/classes/Node.hpp>
#include <string>
#include <vector>
#include <unordered_map>

namespace godot {

class RBInterpreter : public Node {
	GDCLASS(RBInterpreter, Node)
	
	private:
	
	
	protected:
	static void _bind_methods();
	
	void runFile(std::string path);
	
	public:
	RBInterpreter();
	~RBInterpreter();
	
	void _process(double delta);
	void reportError(int line, std::string message);
	
	//see if this is needed in cpp, otherwise remove
	void main(String arg);
	void run(std::string input);
};






}
#endif