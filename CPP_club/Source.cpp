#include<iostream>
#include<fstream>
#include<vector>
#include<string>
#include<map>
#include<time.h>

using namespace std;

ifstream inputMask("mask.txt");
ifstream inputParameters("parameters.json");
ofstream outputResult("result.txt");

const string NOT_ENOUGH_PARAMETERS_ERROR = "not enough parameters";
const string TEXT_NOT_READED_ERROR = "text wasn't been readed";
const string RESERVED_PARAMETER_USED_ERROR = "reserved parameter was defined in JSON file";
const string INCORRECT_ARGUMENT_ERROR = "entered argument is incorrect";

string getCurrentDate() {
	time_t currentTime = time(NULL);
	tm realCurrentTime;
	localtime_s(&realCurrentTime, &currentTime);
	string resultTime;
	resultTime += to_string(realCurrentTime.tm_mday / 10) + to_string(realCurrentTime.tm_mday % 10) + '.';
	resultTime += to_string((realCurrentTime.tm_mon + 1) / 10) + to_string(realCurrentTime.tm_mon % 10) + '.';
	resultTime += to_string(realCurrentTime.tm_year + 1900);
	return resultTime;
}

struct ParameterPosition {
	int line;
	int r, l;
};

struct Parameter {
private:
	bool isObligatory;
	ParameterPosition position;
	string value;
public:
	void setValue(string _value) {
		value = _value;
	}

	string getValue() {
		return value;
	}

	ParameterPosition getPosition() {
		return position;
	}

	bool obligatory() {
		return isObligatory;
	}

	Parameter(bool _isObligatory, ParameterPosition _position) {
		value = "";
		isObligatory = _isObligatory;
		position = _position;
	}

	Parameter(string _value, bool _isObligatory, ParameterPosition _position) {
		value = _value;
		isObligatory = _isObligatory;
		position = _position;
	}

	Parameter() {}
};

class Text {
private:
	vector<string> text;
	map<string, Parameter> parameters;
	const string reservedParameter = "current_date";
	int cntObligatory = 0;

	void updateText() {
		if (text.size() == 0) {
			throw TEXT_NOT_READED_ERROR;
		}
		if (cntObligatory > 0) {
			throw NOT_ENOUGH_PARAMETERS_ERROR;
		}
		if (parameters.count(reservedParameter) && parameters[reservedParameter].getValue() != "") {
			throw RESERVED_PARAMETER_USED_ERROR;
		}
		parameters[reservedParameter].setValue(getCurrentDate());
		for (auto params : parameters) {
			ParameterPosition position = params.second.getPosition();
			int line = position.line;
			string resultString;
			resultString += text[line].substr(0, position.l);
			resultString += params.second.getValue();
			resultString += text[line].substr(position.r + 1, text[line].size());
			text[line] = resultString;
		}
	}

	void printTextToFile() {
		updateText();
		for (int i(0); i < text.size(); i++) {
			outputResult << text[i] << '\n';
		}
	}

	void printTextToConsole() {
		updateText();
		for (int i(0); i < text.size(); i++) {
			cout << text[i] << '\n';
		}
	}

public:
	void readText() {
		string str;
		while (getline(inputMask, str)) {
			text.push_back(str);
			for (int i(0); i < text.back().size(); i++) {
				if (text.back()[i] == '{') {
					ParameterPosition pos;
					pos.line = text.size() - 1;
					pos.l = i;
					i++;
					bool isObligatory = false;
					if (text.back()[i] == '*') {
						isObligatory = true;
						i++;
					}
					string key;
					while (text.back()[i] != '}') {
						key += text.back()[i++];
					}
					pos.r = i;
					parameters[key] = Parameter(isObligatory, pos);
					if (isObligatory) {
						cntObligatory++;
					}
				}
			}
		}
	}

	void readParameters() {
		string str;
		while (getline(inputParameters, str)) {
			string key = "";
			string value = "";
			for (int i(0); i < str.size(); i++) {
				if (str[i] == '\"') {
					i++;
					if (key == "") {
						while (str[i] != '\"') {
							key += str[i++];
						}
						i++;
					}
					else {
						while (str[i] != '\"') {
							value += str[i++];
						}
						i++;
					}
				}
			}
			if (key != "") {
				parameters[key].setValue(value);
				if (parameters[key].obligatory()) {
					cntObligatory--;
				}
			}
		}
	}

	void printText(string arg) {
		try {
			if (arg == "-f") {
				printTextToFile();
			}
			else if (arg == "-c") {
				printTextToConsole();
			}
			else {
				throw INCORRECT_ARGUMENT_ERROR;
			}
		}
		catch (string errorMessage) {
			outputResult << "Error: " << errorMessage;
		}
	}
};

int main(int argc, char *argv[]) {
	Text text;
	text.readText();
	text.readParameters();
	text.printText(argv[1]);

	return 0;
}