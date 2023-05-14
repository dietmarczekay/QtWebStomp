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
    
    
