# QtWebStomp
This is a fork of Gaspa79/QtWebStomp.

Instead of building a library, here the component is adapted to work as a component in QT.
So, callback functions are replaced by signals.

setup a new connection:

      myClient = new QTWebStompClient("wss://some.adress", "someUser", "somePassword", "", true);
      connect(myClient, &QTWebStompClient::connectedMessage, this, &MainWindow::connectedMessage);
      connect(myClient, &QTWebStompClient::subscribedMessage, this, &MainWindow::subscribedMessage);
      connect(myClient, &QTWebStompClient::isClosed, this, &MainWindow::connectionClosed);
      connect(myClient, &QTWebStompClient::submitError, this, &MainWindow::getErrorMessage);
      myClient->openConnection();
      
create a timer for a heartbeat

      heartbeater.setInterval(2500ms);
      connect(&heartbeater, &QTimer::timeout, this, &MainWindow::sendHeartBeat);
      
in the timeout routine just send the QWebSocket::ping

      myClient->heartBeat();
      
The QtStompClient::heartbeat() is doing this for you. 
next step is to subscribe to a topic nad start the heartbeat by

      myClient->Subscribe("/topic/user/trigger",QTWebStompClient::AckMode::Auto);
      heartbeater.start();
    
'subscribedMessage' get the received message as argument. With this you can extract the needed information:

      void MainWindow::subscribedMessage(QString message)
      {
          qDebug() << QDateTime::currentMSecsSinceEpoch() << message;
          StompMessage stompMessage(message.toStdString().c_str());

          QJsonDocument json = QJsonDocument::fromJson(stompMessage.m_message.c_str());
          QJsonObject dataObject = json.object();
          ...
      }
