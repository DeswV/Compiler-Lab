#include "LexicalAnalyzer.h"

#include "GlobalVariable.h"
#include "Utils.h"


const std::unordered_set<char> StartOfSpecialSymbols = { '.','=',';',',',':','<','>','+','-','*','/','(',')' };
const std::unordered_set<char> StartOfNumbers = { '0','1','2','3','4','5','6','7','8','9' };
const std::unordered_set<char> StartOfIdentifiers = { '_','a','b','c','d','e','f','g','h','i','j','k','l','m',
											   'n','o','p','q','r','s','t','u','v','w','x','y','z',
											   'A','B','C','D','E','F','G','H','I','J','K','L','M',
											   'N','O','P','Q','R','S','T','U','V','W','X','Y','Z' };
const std::unordered_set<std::string> Keywords = { "const","var","procedure","call","begin","end","if","then","while","do","odd" };


char CLexicalAnalyzer::GetChar(size_t position)
{
	char ret;
	if (position >= 0 && position < FileSize) {
		SourceFile.seekg(position, std::ios::beg); // �ƶ���ָ��λ��
		SourceFile.get(ret); // ��ȡ��λ�õ��ֽ�
		return ret;
	}
	else {
		Error("GetChar: position out of range.");
	}
}

CLexicalAnalyzer::CLexicalAnalyzer(const std::string& sourceFilePath) : SourceFile{ sourceFilePath,std::ios::binary }
{
	if (!SourceFile.is_open())
	{
		Error("Cannot open file: " + sourceFilePath);
	}
	// ��ȡ�ļ���С
	SourceFile.seekg(0, std::ios::end);
	FileSize = SourceFile.tellg();
}

/*
�ʷ������������񣺶�ȡ�ļ����ó��ս�����б��浽 TerminatorSequence
*/
void CLexicalAnalyzer::LexicalAnalyze()
{
	size_t currentPosition = 0;
	size_t currentLine = 1;

	while (currentPosition < FileSize) {
		char c = GetChar(currentPosition);

		//���1�� �������
		if (StartOfSpecialSymbols.contains(c)) {
			if (c == ':') {
				if (currentPosition + 1 < FileSize && GetChar(currentPosition + 1) == '=') {
					currentPosition += 2;
					TerminatorSequence.push_back({ currentLine, ":=" });
				}
				else {
					Error("Unknown operator: ':', on line " + std::to_string(currentLine));
				}
			}
			else if (c == '<') {
				if (currentPosition + 1 < FileSize && GetChar(currentPosition + 1) == '>') {
					currentPosition += 2;
					TerminatorSequence.push_back({ currentLine,"<>" });
				}
				else if (currentPosition + 1 < FileSize && GetChar(currentPosition + 1) == '=') {
					currentPosition += 2;
					TerminatorSequence.push_back({ currentLine, "<=" });
				}
				else {
					currentPosition += 1;
					TerminatorSequence.push_back({ currentLine, "<" });
				}
			}
			else if (c == '>') {
				if (currentPosition + 1 < FileSize && GetChar(currentPosition + 1) == '=') {
					currentPosition += 2;
					TerminatorSequence.push_back({ currentLine, ">=" });
				}
				else {
					currentPosition += 1;
					TerminatorSequence.push_back({ currentLine, ">" });
				}
			}
			else if (c == '/') {
				if (currentPosition + 1 < FileSize && GetChar(currentPosition + 1) == '/') {
					//����һ��ע��
					currentPosition += 2;
					while (currentPosition < FileSize && GetChar(currentPosition) != '\n') {
						currentPosition += 1;
					}
				}
				else {
					currentPosition += 1;
					TerminatorSequence.push_back({ currentLine,"/" });
				}
			}
			else {
				currentPosition += 1;
				TerminatorSequence.push_back({ currentLine, std::string(1,c) });
			}
		}
		//���2�� ����
		else if (StartOfNumbers.contains(c)) {
			std::string number;
			while (currentPosition < FileSize && StartOfNumbers.contains(GetChar(currentPosition))) {
				number += GetChar(currentPosition);
				currentPosition += 1;
			}
			TerminatorSequence.push_back({ currentLine, "number",std::stoi(number) });
		}
		//���3�� ��ʶ��
		else if (StartOfIdentifiers.contains(c)) {
			std::string identifier;
			while (currentPosition < FileSize && (StartOfIdentifiers.contains(GetChar(currentPosition)) || StartOfNumbers.contains(GetChar(currentPosition)))) {
				identifier += GetChar(currentPosition);
				currentPosition += 1;
			}
			if (Keywords.contains(identifier)) {
				TerminatorSequence.push_back({ currentLine, identifier });
			}
			else {
				TerminatorSequence.push_back({ currentLine,"ident",0,identifier });
			}
		}
		//���4�� ����
		else if (c == '\n') {
			currentPosition += 1;
			currentLine += 1;
		}
		//���5�� ������ֱ������
		else {
			currentPosition += 1;
		}
	}
}

const std::vector<STerminator>& CLexicalAnalyzer::GetTerminatorSequence()
{
	return TerminatorSequence;
}
