#include "tree.hpp"
#include<iostream>

std::ostream& BinOpNode::print() {
    return std::cout;
}

std::ostream& LiteralNumberNode::print() {
    return std::cout << value << std::endl;
}

std::ostream& VariableNode::print() {
    return std::cout;
}

std::ostream& AssignmentNode::print() {
    std::cout << name << " = ";
    value->print();
    return std::cout;
}

std::ostream& CodeBlockNode::print() {
    return std::cout;
}