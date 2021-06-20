#include "bp.hpp"
#include <vector>
#include <iostream>
#include <sstream>
using namespace std;

bool replace(string& str, const string& from, const string& to, const BranchLabelIndex index);

CodeBuffer::CodeBuffer() : buffer(), globalDefs() {}

CodeBuffer &CodeBuffer::instance() {
	static CodeBuffer inst;//only instance
	return inst;
}

string CodeBuffer::genLabel(){
	std::stringstream label;
	label << "label_";
	label << buffer.size();
	std::string ret(label.str());
	label << ":";
	emit(label.str());
	return ret;
}

int CodeBuffer::emit(const string &s){
    buffer.push_back(s);
	return buffer.size() - 1;
}

void CodeBuffer::bpatch(const vector<pair<int,BranchLabelIndex>>& address_list, const std::string &label){
    for(vector<pair<int,BranchLabelIndex>>::const_iterator i = address_list.begin(); i != address_list.end(); i++){
    	int address = (*i).first;
    	BranchLabelIndex labelIndex = (*i).second;
		replace(buffer[address], "@", "%" + label, labelIndex);
    }
}

void CodeBuffer::printCodeBuffer(){
	for (std::vector<string>::const_iterator it = buffer.begin(); it != buffer.end(); ++it) 
	{
		cout << *it << endl;
    }
}

vector<pair<int,BranchLabelIndex>> CodeBuffer::makelist(pair<int,BranchLabelIndex> item)
{
	vector<pair<int,BranchLabelIndex>> newList;
	newList.push_back(item);
	return newList;
}

vector<pair<int,BranchLabelIndex>> CodeBuffer::merge(const vector<pair<int,BranchLabelIndex>> &l1,const vector<pair<int,BranchLabelIndex>> &l2)
{
	vector<pair<int,BranchLabelIndex>> newList(l1.begin(),l1.end());
	newList.insert(newList.end(),l2.begin(),l2.end());
	return newList;
}

// ******** Methods to handle the global section ********** //
void CodeBuffer::emitGlobal(const std::string& dataLine) 
{
	globalDefs.push_back(dataLine);
}

void CodeBuffer::printGlobalBuffer()
{
	for (vector<string>::const_iterator it = globalDefs.begin(); it != globalDefs.end(); ++it)
	{
		cout << *it << endl;
	}
}

// ******** Helper Methods ********** //
bool replace(string& str, const string& from, const string& to, const BranchLabelIndex index) {
	size_t pos;
	if (index == SECOND) {
		pos = str.find_last_of(from);
	}
	else { //FIRST
		pos = str.find_first_of(from);
	}
    if (pos == string::npos)
        return false;
    str.replace(pos, from.length(), to);
    return true;
}

// ************ Extra methods that we added *************************** //

// emit what should be included in every run
void CodeBuffer::preprocess() {
    emitGlobal("declare i32 @printf(i8*, ...)");
    emitGlobal("declare void @exit(i32)");
    emitGlobal("@.int_specifier = constant [4 x i8] c\"%d\\0A\\00\"");
    emitGlobal("@.str_specifier = constant [4 x i8] c\"%s\\0A\\00\"");
    emitGlobal("@.devidebyzero  = constant [23 x i8] c\"Error division by zero\\00\"");
    emitGlobal("");
    emitGlobal("define void @printi(i32) {");
    emitGlobal("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.int_specifier, i32 0, i32 0");
    emitGlobal("call i32 (i8*, ...) @printf(i8* %spec_ptr, i32 %0)");
    emitGlobal("ret void");
    emitGlobal("}");
    emitGlobal("");
    emitGlobal("define void @print(i8*) {");
    emitGlobal("%spec_ptr = getelementptr [4 x i8], [4 x i8]* @.str_specifier, i32 0, i32 0");
    emitGlobal("call i32 (i8*, ...) @printf(i8* %spec_ptr, i8* %0)");
    emitGlobal("ret void");
    emitGlobal("}");
    emitGlobal("");
    emitGlobal("define void @assertDiv(i32) {");
    emitGlobal("%divcmp = icmp eq i32 %0, 0");
    emitGlobal("br i1 %divcmp, label %assertDivIfEqual, label %assertDivIfUnequal");
    emitGlobal("assertDivIfEqual:");
    emitGlobal("%div_error_str = getelementptr [23 x i8], [23 x i8]* @.devidebyzero, i32 0, i32 0");
    emitGlobal("call void @print(i8* %div_error_str)");
    emitGlobal("call void @exit(i32 -1)");
    emitGlobal("ret void");
    emitGlobal("assertDivIfUnequal:");
    emitGlobal("ret void");
    emitGlobal("}");
    emitGlobal("");
}

int CodeBuffer::nextInstruction() {
    return buffer.size();
}

string CodeBuffer::genLabelNextLine() {
    int jmpInstr = emit("br label @");
    string nextInstr = genLabel();
    vector<pair<int,BranchLabelIndex>> v1 = {{jmpInstr, FIRST}};
    bpatch(v1, nextInstr);
    return nextInstr;
}
