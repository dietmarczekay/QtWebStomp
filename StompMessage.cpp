#include "StompMessage.h"

using namespace std;

StompMessage::StompMessage(const char* rawMessage)
{
	std::string strMessage(rawMessage);
	vector<std::string> messageVector = messageToVector(strMessage, "\n");
	m_message = messageVector.back(); // deep copies
	messageVector.pop_back();
	bool first = true;
	for (const auto& header : messageVector)
	{
		size_t pos = header.find_first_of(':', 0);
		std::string key = header.substr(0, pos);
		if (first)
		{
			m_messageType = key;
			first = false;
			continue;
		}
		std::string value = header.substr(++pos, value.length() - pos);
		m_headers[key] = value;
	}
}

StompMessage::StompMessage(string messageType, map<string, string> headers, const char* messageBody)
{
	m_messageType = std::string(messageType);
	m_message = messageBody;
	m_headers = headers;
}

std::string StompMessage::toString() const
{
	std::string result = m_messageType + "\u000A";
	for (const auto &header : m_headers)
	{
		result += header.first + ":" + header.second + "\u000A";
	}

	result += "\u000A" + m_message + "\u0000";

	return result;
}

vector<string> StompMessage::messageToVector(const string& str, const string& delim)
{
	vector<string> messageParts;
	size_t prev = 0, pos = 0;
	bool last = false;
	do
	{
		if (last)
		{
			auto message = str.substr(prev, str.length() - prev);
			messageParts.push_back(message);
			break;
		}

		pos = str.find(delim, prev);
		if (pos == string::npos) pos = str.length();
		string token = str.substr(prev, pos - prev);
		if (!token.empty())
		{
			messageParts.push_back(token);
		}
		else
		{
			// If the token is empty the headers finished =) Just the body left!
			last = true;
		}
		prev = pos + delim.length();
	} while (pos < str.length() && prev <= str.length()); // I know but we have to do it at least once, so...
	return messageParts;
}

