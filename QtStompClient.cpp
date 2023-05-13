
#include "QtStompClient.h"

using namespace std;

QTWebStompClient::QTWebStompClient(const char* url, const char* login, const char* passcode, /*void(*onConnected)(void), */const char* vHost, bool debug, QObject *parent)
{
    m_url = QUrl(url);
	m_debug = debug;
	m_login = login;
	m_passcode = passcode;
    //m_onConnectedCallback = onConnected;
	if (m_debug) {
		qDebug() << "Connecting to WebSocket server:" << url;
	}
	m_vHost = vHost;

	connect(&m_webSocket, &QWebSocket::connected, this, &QTWebStompClient::onConnected);	
	
    connect(&m_webSocket, (&QWebSocket::sslErrors), this, &QTWebStompClient::onSslErrors);

	connect(&m_webSocket, &QWebSocket::disconnected, this, &QTWebStompClient::closed);
    //m_webSocket.open(m_url);
}

void QTWebStompClient::openConnection()
{
    m_webSocket.open(m_url);
}


void QTWebStompClient::onConnected()
{
	if (m_debug) {
        qDebug() << "-----------------------" << Qt::endl << "Connected to Websocket!" << Qt::endl << "-----------------------" << Qt::endl;
	}
	connect(&m_webSocket, &QWebSocket::textMessageReceived, this, &QTWebStompClient::onTextMessageReceived);

	m_connectionState = Connecting;
	QString connectFrame = "CONNECT\u000A{vHost}accept-version:1.2\u000Alogin:{Login}\u000Apasscode:{Passcode}\u000A\u000A\u0000";
	connectFrame.replace("{Login}", m_login);
	connectFrame.replace("{Passcode}", m_passcode);
	QString vHost = "";
	if (m_vHost) {
		vHost = "vHost:" + QString(m_vHost) + QString("\u000A");
	}

	connectFrame.replace("{vHost}", vHost);

	QString connectFrameMessageWithNullFix = QString(connectFrame.data(), connectFrame.size() + 1); // solves the null-terminator issue
	m_webSocket.sendTextMessage(connectFrameMessageWithNullFix);

	if (m_debug) {
		qDebug() << "Sent message" << connectFrameMessageWithNullFix;
	}
}

void QTWebStompClient::onTextMessageReceived(QString message)
{
	StompMessage stompMessage(message.toStdString().c_str());

	switch (m_connectionState) {
		
		case Connecting:
			if (m_debug) {
				qDebug() << "Connection response: " << stompMessage.toString().c_str();
			}
		
			if (stompMessage.m_messageType == "CONNECTED")
			{
				if (m_debug) {
                    qDebug() << "--------------------" << Qt::endl << "Connected to STOMP!" << Qt::endl << "--------------------" << Qt::endl;
				}
				m_connectionState = Connected;
                //if (this->m_onConnectedCallback == NULL)
                //{
                //	qDebug() << "WARNING: No callback selected for connection";
                //	throw runtime_error("No onConnect callback set!");
                //}
                //else
                //{
                    emit connectedMessage(message);
                    //this->m_onConnectedCallback();
                //}
			}
			else
			{
				if (m_debug)
				{
					qDebug() << "Message type CONNECTED expected, got " << stompMessage.m_messageType.c_str();
				}

				throw runtime_error("Message type CONNECTED expected, got" + stompMessage.toString());
			}
			break;

		case Subscribed:
			// TODO: Improve check, maybe different messages are allowed when subscribed
			if (stompMessage.m_messageType == "MESSAGE") {
				if (m_debug) {
                    qDebug() << "Message received from queue!" << Qt::endl << stompMessage.toString().c_str();
				}

                emit subscribedMessage(message);
                //m_onMessageCallback(stompMessage);
			}
			else {
				throw runtime_error("Message type MESSAGE expected, got" + stompMessage.m_messageType + ". Message is : " +stompMessage.toString());
			}
			break;

		default:
			throw runtime_error("Unsupported connection state");
			break;
	}

}

// TODO: Change to have multiple ids (so we can handle more than one subscription)
void QTWebStompClient::Subscribe(const char* queueName, /*void(*onMessageCallback)(const StompMessage &s), */QTWebStompClient::AckMode ackMode)
{
	if (m_connectionState != Connected)
	{
		// For now, if you need to subscribe to 2 queues, you can create two instances of the client. Later an improvement would be to use the id variables of the underlying websocket lib.
		throw runtime_error("Cannot subscribe when connection hasn't finished or when already subscribed. Try using the callback function for onConnect to subscribe");
	}
	map<string, string> headers;
	headers["id"] = "0";
	headers["destination"] = std::string(queueName);
	switch (ackMode) {
		case Client:
			headers["ack"] = "client";
			break;

		case ClientIndividual:
			headers["ack"] = "client-individual";
			break;

		default:
			headers["ack"] = "auto";
			break;
	}

	StompMessage myMessage("SUBSCRIBE", headers, "");

	auto subscribeMessage = QString(myMessage.toString().c_str());
	QString subscribeFrame = QString(subscribeMessage.data(), subscribeMessage.size() + 1);

	m_webSocket.sendTextMessage(subscribeFrame);
	m_connectionState = Subscribed;
    //m_onMessageCallback = onMessageCallback;
}

void QTWebStompClient::onSslErrors(const QList<QSslError> &errors)
{
	throw runtime_error("SSL error! I'd show the error description if there were an easy way to convert from enum to string in c++. You'll have to debug.");
}

void QTWebStompClient::closed()
{
    qDebug() << QDateTime::currentMSecsSinceEpoch() << "Connection closed =(";
    emit isClosed();
    //throw runtime_error("Underlying connection unexpectedly closed =(");
}

void QTWebStompClient::Ack(const StompMessage & s)
{
	auto ack = s.m_headers.at(std::string("ack"));
	Ack(ack.c_str());
}

void QTWebStompClient::Ack(const char* id)
{
	if (m_debug) {
		qDebug() << "Acking message with id: " << id;
	}
	// Yes I know that this is disgusting
	QString ackFrame("ACK\u000Aid:{{TheAckId}}\u000A\u000A\u000A\u0000");
	ackFrame.replace("{{TheAckId}}", id);
	QString ackFrameNullFixed(ackFrame.data(), ackFrame.size() + 1); // solves the null-terminator issue
	m_webSocket.sendTextMessage(ackFrameNullFixed);
}

void QTWebStompClient::Send(const StompMessage & stompMessage)
{
	if (m_debug) {
		qDebug() << "Sending message: " << stompMessage.toString().c_str();
	}

	std::string sendFrame = std::string(stompMessage.m_messageType+"\u000A");
	for (auto &header : stompMessage.m_headers)
	{
		sendFrame += header.first + ":" + header.second + "\u000A";
	}

	sendFrame += "\u000A" + stompMessage.m_message + "\u0000";

	QString sendFrameTemp(sendFrame.c_str());
	QString sendFrameMessage(sendFrameTemp.data(), sendFrameTemp.size() + 1);
	m_webSocket.sendTextMessage(sendFrameMessage);
}

void QTWebStompClient::Send(const char* destination, const char* message, map<string, string> headers)
{
	headers[std::string("destination")] = std::string(destination);
	StompMessage s("SEND", headers, message);
	Send(s);
}
